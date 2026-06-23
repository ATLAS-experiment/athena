/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OnDemandMinbiasSvc.h"

#include <GaudiKernel/ConcurrencyFlags.h>

#include <algorithm>
#include <boost/core/demangle.hpp>
#include <chrono>
#include <format>
#include <random>
#include <range/v3/algorithm.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view.hpp>
#include <thread>

#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IProxyProviderSvc.h"
#include "CxxUtils/FastReseededPRNG.h"
#include "SGTools/CurrentEventStore.h"
#include "StoreGate/ActiveStoreSvc.h"


inline std::string CLIDToString(const CLID& clid) {
  return boost::core::demangle(CLIDRegistry::CLIDToTypeinfo(clid)->name());
}

OnDemandMinbiasSvc::OnDemandMinbiasSvc(const std::string& name,
                                       ISvcLocator* svc)
    : base_class(name, svc),
      m_bkg_evt_sel_ctx(nullptr),
      m_proxyProviderSvc("ProxyProviderSvc/BkgPPSvc_"+name, name),
      m_last_loaded_hs()
{}

OnDemandMinbiasSvc::~OnDemandMinbiasSvc() {}

StatusCode OnDemandMinbiasSvc::initialize() {
  ATH_CHECK(m_bkgEventSelector.retrieve());
  ATH_CHECK(m_activeStoreSvc.retrieve());
  ATH_CHECK(m_skipEventIdxSvc.retrieve());
  if (m_useBeamInt) {
    ATH_CHECK(m_beamInt.retrieve());
  }
  if (m_useBeamLumi) {
    ATH_CHECK(m_beamLumi.retrieve());
  }
  // Setup context
  if (!m_bkgEventSelector->createContext(m_bkg_evt_sel_ctx).isSuccess()) {
    ATH_MSG_ERROR("Failed to create background event selector context");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(SmartIF<IService>(m_bkgEventSelector.get())->start());

  // Setup proxy provider
  ATH_CHECK(m_proxyProviderSvc.retrieve());

  // Setup Address Providers
  SmartIF<IAddressProvider> addressProvider{m_bkgEventSelector.get()};
  if (!addressProvider) {
    ATH_MSG_WARNING(
        "Could not cast background event selector to IAddressProvider");
  } else {
    m_proxyProviderSvc->addProvider(addressProvider);
  }
  // AthenaPoolAddressProviderSvc
  SmartIF<IAddressProvider> athPoolAP{
    serviceLocator()->service(std::format("AthenaPoolAddressProviderSvc/BkgAPAPSvc_{}", name()))
  };
  if (!athPoolAP) {
    ATH_MSG_WARNING(
        "Could not cast AthenaPoolAddressProviderSvc to IAddressProvider");
  } else {
    m_proxyProviderSvc->addProvider(athPoolAP);
  }
  // AddressRemappingSvc
  SmartIF<IAddressProvider> addRemapAP{
    serviceLocator()->service(std::format("AddressRemappingSvc/BkgARSvc_{}", name()))
  };
  if (!addRemapAP) {
    ATH_MSG_WARNING("Could not cast AddressRemappingSvc to IAddressProvider");
  } else {
    m_proxyProviderSvc->addProvider(addRemapAP);
  }

  const int n_stores = 50;  // Start with 50 stores per event
  std::size_t i = 0;
  // setup n_concurrent vectors of n_stores StoreGates in m_stores
  for (auto& sgs : m_stores) {
    sgs.reserve(n_stores);
    for (int j = 0; j < n_stores; ++j) {
      // creates / retrieves a different StoreGateSvc for each slot
      auto& sg = sgs.emplace_back(
          std::format("StoreGateSvc/StoreGate_{}_{}_{}", name(), i, j), name());
      ATH_CHECK(sg.retrieve());
      sg->setStoreID(StoreID::PILEUP_STORE);
      sg->setProxyProviderSvc(m_proxyProviderSvc.get());
    }
    ++i;
  }

  // setup spare store for event skipping
  ATH_CHECK(m_spare_store.retrieve());
  m_spare_store->setStoreID(StoreID::PILEUP_STORE);
  m_spare_store->setProxyProviderSvc(m_proxyProviderSvc.get());
  auto skipEvent_callback = [this](
                                ISkipEventIdxSvc::EvtIter begin,
                                ISkipEventIdxSvc::EvtIter end) -> StatusCode {
    using namespace std::chrono_literals;
    auto* const old_store = m_activeStoreSvc->activeStore();
    m_activeStoreSvc->setStore(m_spare_store.get());
    ATH_MSG_INFO("Skipping " << end - begin << " HS events. ");
    for (auto iter = begin; iter < end; ++iter) {
      const auto& evt = *iter;
      EventContext ctx;
      EventIDBase eid;
      eid.set_run_number(evt.runNum);
      eid.set_lumi_block(evt.lbNum);
      eid.set_event_number(evt.evtNum);
      ctx.setEventID(eid);
      const std::size_t n_to_skip = calcMBRequired(evt.evtIdx, ctx);
      ATH_MSG_DEBUG("Skipping HS_ID " << evt.evtIdx << " --> skipping "
                                      << n_to_skip << " pileup events");
      for (std::size_t i = 0; i < n_to_skip; ++i) {
        if (!m_bkgEventSelector->next(*m_bkg_evt_sel_ctx).isSuccess()) {
          ATH_MSG_ERROR("Ran out of background events");
          return StatusCode::FAILURE;
        }
      }
    }
    m_last_loaded_hs.store((end - 1)->evtIdx);
    m_activeStoreSvc->setStore(old_store);
    return StatusCode::SUCCESS;
  };
  ATH_CHECK(m_skipEventIdxSvc->registerCallback(skipEvent_callback));
  ATH_MSG_INFO("Initializing ODMBSvc");
  return StatusCode::SUCCESS;
}

std::size_t OnDemandMinbiasSvc::calcMBRequired(std::int64_t hs_id,
                                               const EventContext& ctx) {
  ATH_MSG_DEBUG("Run " << ctx.eventID().run_number()
                       << ", lumi " << ctx.eventID().lumi_block()
                       << ", event " << ctx.eventID().event_number()
                       << "| hs_id " << hs_id);
  const int n_bunches = m_latestDeltaBC.value() - m_earliestDeltaBC.value() + 1;

  std::vector<std::uint64_t>& num_mb_by_bunch = *m_num_mb_by_bunch.get(ctx);
  num_mb_by_bunch.clear();
  num_mb_by_bunch.resize(n_bunches);
  FastReseededPRNG prng{m_seed.value(), hs_id};

  // First apply the beam luminosity SF
  bool sf_updated_throwaway;
  const float beam_lumi_sf =
      m_useBeamLumi ? m_beamLumi->scaleFactor(ctx.eventID().run_number(),
                                              ctx.eventID().lumi_block(),
                                              sf_updated_throwaway)
                    : 1.F;
  const float beam_lumi = beam_lumi_sf * m_nPerBunch.value();
  std::vector<float> avg_num_mb_by_bunch(n_bunches, beam_lumi);
  // Now update using beam intensities
  if (m_useBeamInt) {
    // Supposed to be once per event, but ends up running once per minbias type
    // per event now
    m_beamInt->selectT0(ctx);
    for (int bunch = m_earliestDeltaBC.value();
         bunch <= m_latestDeltaBC.value(); ++bunch) {
      std::size_t idx = bunch - m_earliestDeltaBC.value();
      avg_num_mb_by_bunch[idx] *= m_beamInt->normFactor(bunch);
    }
  }

  if (m_usePoisson) {
    std::transform(avg_num_mb_by_bunch.begin(), avg_num_mb_by_bunch.end(),
                   num_mb_by_bunch.begin(), [&prng](float avg) {
                     return std::poisson_distribution<std::uint64_t>(avg)(prng);
                   });
  } else {
    std::transform(avg_num_mb_by_bunch.begin(), avg_num_mb_by_bunch.end(),
                   num_mb_by_bunch.begin(), [](float f) {
                     return static_cast<std::uint64_t>(std::round(f));
                   });
  }

  std::uint64_t num_mb = ranges::accumulate(num_mb_by_bunch, 0UL);
  if (!ctx.valid()) {
    return num_mb;
  }
  // Won't go past here for an invalid slot (during initialize()

  std::vector<std::uint64_t>& index_array = *m_idx_lists.get(ctx);
  index_array.clear();
  index_array.resize(num_mb);
  std::iota(index_array.begin(), index_array.end(), 0);
  // Don't need to shuffle, since these events aren't reused
  // std::shuffle(index_array.begin(), index_array.end(), prng);

  // Commented out until we can use C++ 23 range formatting
  // ATH_MSG_DEBUG("HS ID " << hs_id << " uses " << num_mb << " events\n"
  //                        << fmt::format("\t\tBy bunch: [{}]\n",
  //                                       fmt::join(num_mb_by_bunch, ", ")));
  return num_mb;
}

StatusCode OnDemandMinbiasSvc::beginHardScatter(const EventContext& ctx) {
  using namespace std::chrono_literals;
  std::chrono::steady_clock::time_point order_wait_start{};

  const std::int64_t hs_id = get_hs_id(ctx);
  const std::size_t slot = ctx.slot();
  const std::size_t num_to_load = calcMBRequired(hs_id, ctx);
  auto& stores = *m_stores.get(ctx);
  // If we don't have enough stores, make more
  if (stores.size() < num_to_load) {
    ATH_MSG_INFO("Adding " << num_to_load - stores.size() << " stores");
    stores.reserve(num_to_load);
    for (std::size_t i = stores.size(); i < num_to_load; ++i) {
      auto& sg = stores.emplace_back(
          std::format("StoreGateSvc/StoreGate_{}_{}_{}", name(), slot, i),
          name());
      ATH_CHECK(sg.retrieve());
      sg->setStoreID(StoreID::PILEUP_STORE);
      sg->setProxyProviderSvc(m_proxyProviderSvc.get());
    }
  }
  // Ensure loading is done in order
  if (m_last_loaded_hs < hs_id - 1) {
    ATH_MSG_INFO("Waiting to prevent out-of-order loading. Last loaded is "
                 << m_last_loaded_hs << " and we are " << hs_id);
    order_wait_start = std::chrono::steady_clock::now();
    while (m_last_loaded_hs < hs_id - 1) {
      std::this_thread::sleep_for(50ms);
    }
    auto wait_time = std::chrono::steady_clock::now() - order_wait_start;
    ATH_MSG_INFO(std::format("Waited {:%M:%S} to prevent out-of-order loading",
                             wait_time));
  }
  // Lock reading mutex
  std::unique_lock lck(m_reading_batch_mtx);
  auto start = std::chrono::steady_clock::now();
  // Remember old store to reset later
  auto* old_store = m_activeStoreSvc->activeStore();
  for (std::size_t i = 0; i < num_to_load; ++i) {
    auto& sg = stores[i];
    // Change active store
    m_activeStoreSvc->setStore(sg.get());
    SG::CurrentEventStore::Push reader_sg_ces(sg.get());
    // Read next event
    ATH_CHECK(sg->clearStore(true));
    if (!(m_bkgEventSelector->next(*m_bkg_evt_sel_ctx)).isSuccess()) {
      ATH_MSG_FATAL("Ran out of minbias events");
      return StatusCode::FAILURE;
    }
    IOpaqueAddress* addr = nullptr;
    if (!m_bkgEventSelector->createAddress(*m_bkg_evt_sel_ctx, addr)
             .isSuccess()) {
      ATH_MSG_WARNING("Failed to create address. No more events?");
      return StatusCode::FAILURE;
    }
    if (addr == nullptr) {
      ATH_MSG_WARNING("createAddress returned nullptr. No more events?");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(sg->recordAddress(addr));
    ATH_CHECK(sg->loadEventProxies());
    // Read data now if desired
    if (!m_onDemandMB) {
      for (const auto* proxy_ptr : sg->proxies()) {
        // Sort of a const_cast, then ->accessData()
        sg->proxy_exact(proxy_ptr->sgkey())->accessData();
      }
    }
  }
  // Reset active store
  m_activeStoreSvc->setStore(old_store);
  ATH_MSG_INFO(std::format("Took {:%M:%S} to load events",
                           std::chrono::steady_clock::now() - start));
  // Update last loaded
  m_last_loaded_hs.store(hs_id);
  return StatusCode::SUCCESS;
}

StoreGateSvc* OnDemandMinbiasSvc::getMinbias(const EventContext& ctx,
                                             std::uint64_t mb_id) {
  const std::size_t index = m_idx_lists.get(ctx)->at(mb_id);
  return m_stores.get(ctx)->at(index).get();
}

std::size_t OnDemandMinbiasSvc::getNumForBunch(const EventContext& ctx,
                                               int bunch) const {
  if (bunch < m_earliestDeltaBC.value() || bunch > m_latestDeltaBC.value()) {
    throw std::logic_error(std::format(
        "Tried to request bunch {} which is outside the range [{}, {}]", bunch,
        m_earliestDeltaBC.value(), m_latestDeltaBC.value()));
  }
  return m_num_mb_by_bunch.get(ctx)->at(bunch - m_earliestDeltaBC.value());
}

StatusCode OnDemandMinbiasSvc::endHardScatter(const EventContext& ctx) {
  // clear all stores
  for (SGHandle& sg : *m_stores.get(ctx)) {
    ATH_CHECK(sg->clearStore());
  }
  return StatusCode::SUCCESS;
}
