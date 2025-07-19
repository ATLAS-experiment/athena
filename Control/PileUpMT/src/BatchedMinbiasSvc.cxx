/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BatchedMinbiasSvc.h"

#include <GaudiKernel/ConcurrencyFlags.h>

#include <algorithm>
#include <boost/core/demangle.hpp>
#include <chrono>
#include <cmath>
#include <format>
#include <random>
#include <range/v3/algorithm/stable_sort.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view.hpp>
#include <thread>

#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IProxyProviderSvc.h"
#include "CxxUtils/FastReseededPRNG.h"
#include "SGTools/CurrentEventStore.h"

namespace rv = ranges::views;

inline std::string CLIDToString(const CLID& clid) {
  return boost::core::demangle(CLIDRegistry::CLIDToTypeinfo(clid)->name());
}

BatchedMinbiasSvc::BatchedMinbiasSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc),
      m_bkg_evt_sel_ctx(nullptr),
      m_last_loaded_batch() {}

BatchedMinbiasSvc::~BatchedMinbiasSvc() {}

int BatchedMinbiasSvc::event_to_batch(std::int64_t hs_id) {
  return int(hs_id / m_HSBatchSize.value());
}

StatusCode BatchedMinbiasSvc::initialize() {
  ATH_CHECK(m_skipEventIdxSvc.retrieve());
  ATH_CHECK(m_beamInt.retrieve());
  ATH_CHECK(m_beamLumi.retrieve());
  std::size_t n_concurrent =
      Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
  m_idx_lists.clear();
  m_idx_lists.resize(n_concurrent);

  m_num_mb_by_bunch.clear();
  m_num_mb_by_bunch.resize(n_concurrent);

  m_cache.clear();
  m_empty_caches.clear();
  m_batch_use_count.clear();
  m_batch_use_count.reserve(m_actualNHSEventsPerBatch.value().size());
  for (std::size_t i = 0; i < m_actualNHSEventsPerBatch.value().size(); ++i) {
    m_batch_use_count.emplace_back(std::make_unique<std::atomic_int>(0));
  }
  ATH_CHECK(m_bkgEventSelector.retrieve());
  ATH_CHECK(m_activeStoreSvc.retrieve());
  // Setup context
  if (!m_bkgEventSelector->createContext(m_bkg_evt_sel_ctx).isSuccess()) {
    ATH_MSG_ERROR("Failed to create background event selector context");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(SmartIF<IService>(m_bkgEventSelector.get())->start());

  // Setup proxy provider
  SmartIF<IProxyProviderSvc> proxyProviderSvc{
    serviceLocator()->service(std::format("ProxyProviderSvc/BkgPPSvc_{}", name()))
  };
  ATH_CHECK(proxyProviderSvc.isValid());

  // Setup Address Providers
  SmartIF<IAddressProvider> addressProvider{m_bkgEventSelector.get()};
  if (!addressProvider) {
    ATH_MSG_WARNING(
        "Could not cast background event selector to IAddressProvider");
  } else {
    proxyProviderSvc->addProvider(addressProvider);
  }
  // AthenaPoolAddressProviderSvc
  SmartIF<IAddressProvider> athPoolAP{
    serviceLocator()->service(std::format("AthenaPoolAddressProviderSvc/BkgAPAPSvc_{}", name()))
  };
  if (!athPoolAP) {
    ATH_MSG_WARNING(
        "Could not cast AthenaPoolAddressProviderSvc to IAddressProvider");
  } else {
    proxyProviderSvc->addProvider(athPoolAP);
  }
  // AddressRemappingSvc
  SmartIF<IAddressProvider> addRemapAP{
    serviceLocator()->service(std::format("AddressRemappingSvc/BkgARSvc_{}", name()))
  };
  if (!addRemapAP) {
    ATH_MSG_WARNING("Could not cast AddressRemappingSvc to IAddressProvider");
  } else {
    proxyProviderSvc->addProvider(addRemapAP);
  }

  int mbBatchSize = m_MBBatchSize.value();
  // setup NSimultaneousBatches vectors of MBBatchSize StoreGates in
  // m_empty_caches
  for (int i = 0; i < m_NSimultaneousBatches.value(); ++i) {
    auto& sgs = m_empty_caches.emplace_back(std::make_unique<SGHandleArray>());
    sgs->reserve(mbBatchSize);
    for (int j = 0; j < mbBatchSize; ++j) {
      // creates / retrieves a different StoreGateSvc for each slot
      auto& sg = sgs->emplace_back(
          std::format("StoreGateSvc/StoreGate_{}_{}_{}", name(), i, j), name());
      ATH_CHECK(sg.retrieve());
      sg->setStoreID(StoreID::PILEUP_STORE);
      sg->setProxyProviderSvc(proxyProviderSvc);
    }
  }

  // Setup the spare store for event skipping
  ATH_CHECK(m_spare_store.retrieve());
  m_spare_store->setStoreID(StoreID::PILEUP_STORE);
  m_spare_store->setProxyProviderSvc(proxyProviderSvc);

  // Setup the callback for event skipping
  auto skipEvent_callback = [this, mbBatchSize](
                                ISkipEventIdxSvc::EvtIter begin,
                                ISkipEventIdxSvc::EvtIter end) -> StatusCode {
    using namespace std::chrono_literals;
    auto evts = ranges::make_subrange(begin, end);
    ATH_MSG_INFO("Skipping " << end - begin << " HS events.");
    auto batches_all =
        evts | rv::transform([this](const ISkipEventIdxSvc::EvtId& evt) {
          return event_to_batch(evt.evtIdx);
        });
    std::vector<std::tuple<int, int>> batches_with_counts{};
    // Produce a list of batches, and how many times they appear
    for (int batch : batches_all) {
      // First entry
      if (batches_with_counts.empty()) {
        batches_with_counts.emplace_back(batch, 1);
        continue;
      }
      // Subsequent entries
      auto& last_entry = batches_with_counts.back();
      if (batch == std::get<0>(last_entry)) {
        std::get<1>(last_entry) += 1;
        continue;
      }
      batches_with_counts.emplace_back(batch, 1);
    }

    // Discard batches
    const int hs_batch_size = m_HSBatchSize.value();
    auto* const old_store = m_activeStoreSvc->activeStore();
    m_activeStoreSvc->setStore(m_spare_store.get());
    ATH_CHECK(m_spare_store->clearStore());
    for (const auto& [batch, count] : batches_with_counts) {
      if (m_cache.count(batch) != 0) {
        // batch is currently loaded, just update the use count
        m_batch_use_count[batch]->fetch_add(count);
        continue;
      }
      // force ordering in background stream
      while (m_last_loaded_batch < batch - 1) {
        std::this_thread::sleep_for(50ms);
      }
      // if we aren't skipping all the hardscatters in the batch, do nothing
      if ((m_batch_use_count[batch]->fetch_add(count) + count) <
          hs_batch_size) {
        continue;
      }
      // otherwise discard the batch
      ATH_MSG_INFO("Discarding batch " << batch);
      std::unique_lock lck{m_reading_batch_mtx};
      if (!m_bkgEventSelector->next(*m_bkg_evt_sel_ctx, mbBatchSize)
               .isSuccess()) {
        ATH_MSG_INFO("Ran out of background events");
        return StatusCode::FAILURE;
      }
      // increment counters
      m_last_loaded_batch.fetch_add(1);
    }
    ATH_CHECK(m_spare_store->clearStore());
    m_activeStoreSvc->setStore(old_store);
    return StatusCode::SUCCESS;
  };

  // register callback
  ATH_CHECK(m_skipEventIdxSvc->registerCallback(skipEvent_callback));
  return StatusCode::SUCCESS;
}

std::size_t BatchedMinbiasSvc::calcMBRequired(std::int64_t hs_id,
                                              std::size_t slot,
                                              unsigned int run,
                                              unsigned int lumi,
                                              std::uint64_t event) {
  const int n_bunches = m_latestDeltaBC.value() - m_earliestDeltaBC.value() + 1;
  FastReseededPRNG prng{m_seed.value(), hs_id};

  // First apply the beam luminosity SF
  bool sf_updated_throwaway;
  const float beam_lumi_sf =
      m_useBeamLumi ? m_beamLumi->scaleFactor(run, lumi, sf_updated_throwaway)
                    : 1.f;
  std::vector<float> avg_num_mb_by_bunch(n_bunches,
                                         beam_lumi_sf * m_nPerBunch.value());
  // Now update using beam intensities
  if (m_useBeamInt) {
    // Supposed to be once per event, but ends up running once per minbias type
    // per event now
    m_beamInt->selectT0(run, event);
    for (int bunch = m_earliestDeltaBC.value();
         bunch <= m_latestDeltaBC.value(); ++bunch) {
      std::size_t idx = bunch - m_earliestDeltaBC.value();
      avg_num_mb_by_bunch[idx] *= m_beamInt->normFactor(bunch);
    }
  }

  std::vector<std::uint64_t>& num_mb_by_bunch = m_num_mb_by_bunch[slot];
  num_mb_by_bunch.clear();
  num_mb_by_bunch.resize(n_bunches);

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
  std::vector<std::uint64_t>& index_array = m_idx_lists[slot];
  const std::uint64_t mbBatchSize = m_MBBatchSize.value();
  // Prevent running out of events
  if (num_mb > mbBatchSize) {
    const int center_bunch = -m_earliestDeltaBC.value();
    auto indices =
        rv::iota(0ULL, num_mb_by_bunch.size()) |
        rv::filter([center_bunch, &num_mb_by_bunch](int idx) {
          bool good = idx != center_bunch;  // filter out the central bunch
          good =
              good && num_mb_by_bunch[idx] > 0;  // filter out unfilled bunches
          return good;
        }) |
        ranges::to<std::vector>;
    // sort by distance from central bunch
    ranges::stable_sort(indices, std::greater{},
                        [center_bunch](std::size_t idx) {
                          return std::size_t(std::abs(int(idx) - center_bunch));
                        });
    // subtract from bunches until we aren't using too many events
    for (auto idx : indices) {
      const std::uint64_t max_to_subtract = num_mb - mbBatchSize;
      const std::uint64_t num_subtracted =
          std::min(max_to_subtract, num_mb_by_bunch[idx]);
      num_mb_by_bunch[idx] -= num_subtracted;
      num_mb -= num_subtracted;
      if (num_mb <= mbBatchSize) {
        break;
      }
    }
    // Print an error anyway so we can fix the job
    ATH_MSG_ERROR("We need " << num_mb << " events but the batch size is "
                             << mbBatchSize << ". Restricting to "
                             << mbBatchSize << " events!");
  }
  index_array = rv::ints(0, int(mbBatchSize)) | rv::sample(num_mb, prng) |
                ranges::to<std::vector<std::uint64_t>>;
  ranges::shuffle(index_array, prng);
  ATH_MSG_DEBUG("HS ID " << hs_id << " uses " << num_mb << " events");
  // Disabled until C++ 23 range formatting can be used
  // if (m_HSBatchSize <= 1) {
  //   ATH_MSG_DEBUG(fmt::format("\t\tBy bunch: [{}]\n", fmt::join(num_mb_by_bunch, ", "))
  //                 << fmt::format("\t\tOrder: [{}]", fmt::join(index_array, ", ")));
  // }
  return num_mb;
}

StatusCode BatchedMinbiasSvc::beginHardScatter(const EventContext& ctx) {
  using namespace std::chrono_literals;
  bool first_wait = true;
  std::chrono::steady_clock::time_point cache_wait_start{};
  std::chrono::steady_clock::time_point order_wait_start{};
  const std::int64_t hs_id = get_hs_id(ctx);
  const int batch = event_to_batch(hs_id);
  calcMBRequired(hs_id, ctx.slot(),
                 ctx.eventID().run_number(),  // don't need the total, only
                 ctx.eventID().lumi_block(),  // need to populate the arrays
                 ctx.eventID().event_number());
  while (true) {
    if (m_cache.count(batch) != 0) {
      // batch already loaded
      // mutex prevents returning when batch is partially loaded
      m_cache_mtxs[batch].lock();
      m_cache_mtxs[batch].unlock();
      return StatusCode::SUCCESS;
    }
    // prevent batches loading out-of-order
    if (m_last_loaded_batch < (batch - 1)) {
      ATH_MSG_INFO("Waiting to prevent out-of-order loading of batches");
      order_wait_start = std::chrono::steady_clock::now();
      while (m_last_loaded_batch < (batch - 1)) {
        std::this_thread::sleep_for(50ms);
      }
      auto wait_time = std::chrono::steady_clock::now() - order_wait_start;
      ATH_MSG_INFO(std::format(
          "Waited {:%M:%S} to prevent out-of-order loading", wait_time));
    }
    // See if there are any free caches
    // Using try_lock here to avoid reading same batch twice
    if (m_empty_caches_mtx.try_lock()) {
      if (m_empty_caches.empty()) {
        // Unlock mutex if we got the lock but there were no free caches
        m_empty_caches_mtx.unlock();
        if (first_wait) {
          ATH_MSG_INFO("Waiting for a free cache");
          first_wait = false;
          cache_wait_start = std::chrono::steady_clock::now();
        }
        // Wait 100ms then try again
        std::this_thread::sleep_for(100ms);
        continue;
      }
      if (!first_wait) {
        auto wait_time = std::chrono::steady_clock::now() - cache_wait_start;
        ATH_MSG_INFO(
            std::format("Waited {:%M:%S} for a free cache", wait_time));
      }
      std::scoped_lock reading{m_cache_mtxs[batch], m_reading_batch_mtx};
      if (m_HSBatchSize != 0) {
        ATH_MSG_INFO("Reading next batch in event " << ctx.evt() << ", slot "
                                                    << ctx.slot() << " (hs_id "
                                                    << hs_id << ")");
      }
      auto start_time = std::chrono::system_clock::now();
      m_cache[batch] = std::move(m_empty_caches.front());
      m_empty_caches.pop_front();
      // Remember old store to reset later
      auto* old_store = m_activeStoreSvc->activeStore();
      for (auto&& sg : *m_cache[batch]) {
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
            if (!proxy_ptr->isValid()) {
              continue;
            }

            // Sort of a const_cast, then ->accessData()
            sg->proxy_exact(proxy_ptr->sgkey())->accessData();
          }
        }
      }
      // Reset active store
      m_activeStoreSvc->setStore(old_store);
      if (m_HSBatchSize != 0) {
        ATH_MSG_INFO(std::format(
            "Reading {} events took {:%OMm %OSs}", m_cache[batch]->size(),
            std::chrono::system_clock::now() - start_time));
      }
      m_empty_caches_mtx.unlock();
      m_last_loaded_batch.exchange(batch);
      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::SUCCESS;
}

StoreGateSvc* BatchedMinbiasSvc::getMinbias(const EventContext& ctx,
                                            std::uint64_t mb_id) {
  const std::int64_t hs_id = get_hs_id(ctx);
  const std::size_t slot = ctx.slot();
  const std::size_t index = m_idx_lists.at(slot).at(mb_id);
  const int batch = event_to_batch(hs_id);
  return m_cache[batch]->at(index).get();
}

std::size_t BatchedMinbiasSvc::getNumForBunch(const EventContext& ctx,
                                              int bunch) const {
  if (bunch < m_earliestDeltaBC.value() || bunch > m_latestDeltaBC.value()) {
    throw std::logic_error(std::format(
        "Tried to request bunch {} which is outside the range [{}, {}]", bunch,
        m_earliestDeltaBC.value(), m_latestDeltaBC.value()));
  }
  return m_num_mb_by_bunch.at(ctx.slot()).at(bunch - m_earliestDeltaBC.value());
}

StatusCode BatchedMinbiasSvc::endHardScatter(const EventContext& ctx) {
  using namespace std::chrono_literals;
  const std::int64_t hs_id = get_hs_id(ctx);
  const int batch = event_to_batch(hs_id);
  const int uses = m_batch_use_count[batch]->fetch_add(1) + 1;

  // If we're done with every event in the batch, clear the stores and return
  // them
  if (uses == m_HSBatchSize.value()) {
    std::unique_ptr temp = std::move(m_cache[batch]);
    m_cache.erase(batch);
    for (auto&& sg : *temp) {
      ATH_CHECK(sg->clearStore());
    }
    std::lock_guard lg{m_empty_caches_mtx};
    m_empty_caches.emplace_back(std::move(temp));
  } else {
    ATH_MSG_DEBUG("BATCH " << batch << ": " << uses << " uses out of "
                           << m_HSBatchSize << "  "
                           << m_actualNHSEventsPerBatch);
  }
  return StatusCode::SUCCESS;
}
