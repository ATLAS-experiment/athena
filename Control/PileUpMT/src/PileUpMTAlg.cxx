/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PileUpMT includes
#include "PileUpMTAlg.h"

#include <CxxUtils/XXH.h>
#include <fmt/format.h>
#include <unistd.h>

#include <boost/core/demangle.hpp>
#include <range/v3/all.hpp>

#include <chrono>
#include <format>

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "EventInfo/EventInfo.h"
#include <tuple>

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandPoisson.h"
#include "CLHEP/Random/RandomEngine.h"
#include "PileUpTools/PileUpHashHelper.h"
#include "PileUpTools/PileUpMisc.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventInfoAuxContainer.h"
#include "xAODEventInfo/EventInfoContainer.h"

using SubEvent = xAOD::EventInfo::SubEvent;
namespace rv = ranges::views;
// namespace ra = ranges::actions;

inline std::string CLIDToString(const CLID& clid) {
  return boost::core::demangle(CLIDRegistry::CLIDToTypeinfo(clid)->name());
}

PileUpMTAlg::PileUpMTAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

PileUpMTAlg::~PileUpMTAlg() {}

StatusCode PileUpMTAlg::get_ei(StoreGateSvc& sg,
                               std::unique_ptr<const xAOD::EventInfo>& ei_,
                               std::unique_ptr<xAOD::EventAuxInfo>& ei_aux_,
                               bool pileup) const {
  std::string key = pileup ? "EventInfo" : "HSEventInfo";
  auto newEi = std::make_unique<xAOD::EventInfo>();
  auto eiAux = std::make_unique<xAOD::EventAuxInfo>();
  newEi->setStore(eiAux.get());
  SG::ReadHandle<xAOD::EventInfo> ei_h(key, sg.name());
  const xAOD::EventInfo* ei = ei_h.get();
  if (ei != nullptr) {
    *newEi = *ei;
  } else {
    ATH_MSG_ERROR("Couldn't find xAOD::EventInfo. " << sg.name());
    ATH_MSG_ERROR(sg.dump());
    return StatusCode::FAILURE;
  }
  // Use attribute list if EventInfo doesn't have event numbers set
  if (newEi->eventNumber() == 0) {
    const auto* attr_list_p = sg.tryConstRetrieve<AthenaAttributeList>("Input");
    if (attr_list_p != nullptr) {
      if (attr_list_p->size() <= 6) {
        ATH_MSG_WARNING("Read Input attribute list but size <= 6");
      } else {
        const AthenaAttributeList& attr_list = *attr_list_p;
        const auto runNum = attr_list["RunNumber"].data<unsigned>();
        const auto evtNum = attr_list["EventNumber"].data<unsigned long long>();
        const auto lbNum = attr_list["LumiBlockN"].data<unsigned>();
        newEi->setRunNumber(runNum);
        newEi->setLumiBlock(lbNum);
        newEi->setEventNumber(evtNum);
      }
    } else {
      ATH_MSG_WARNING(
          "Could not read Input attribute list and EventInfo has no event "
          "number");
    }
  }
  newEi->setEvtStore(&sg);
  ei_ = std::move(newEi);
  ei_aux_ = std::move(eiAux);
  return StatusCode::SUCCESS;
}

StatusCode PileUpMTAlg::add_subevt(
    const std::vector<std::uint32_t>& bcid,
    SG::WriteHandle<xAOD::EventInfo>& overlaidEvt,
    SG::WriteHandle<xAOD::EventInfoContainer>& puCont,
    ServiceHandle<IMinbiasSvc>& mbSvc, xAOD::EventInfo::PileUpType puType,
    int bc, const EventContext& ctx, unsigned long subevt_id,
    std::vector<std::uint64_t>& trace) {
  // Keep the code to add and process a subevent in one place
  const unsigned int bc_idx = bc - m_earliestDeltaBC;
  StoreGateSvc* sg = mbSvc->getMinbias(ctx, subevt_id);
  std::unique_ptr<const xAOD::EventInfo> ei;
  std::unique_ptr<xAOD::EventAuxInfo> eiAux;
  ATH_CHECK(get_ei(*sg, ei, eiAux, true));
  xAOD::EventInfo mb_to_modify(*ei);
  if (m_writeTrace) {
    trace.push_back(mb_to_modify.eventNumber());
  }
  mb_to_modify.setBCID(bcid[bc_idx]);
  try {
    addSubEvent(overlaidEvt.ptr(), &mb_to_modify, bc * m_BCSpacing, puType,
                puCont.ptr(), m_evtInfoContKey.key(), sg);
  } catch (const std::exception& e) {
    ATH_MSG_ERROR("Caught exception adding subevent: " << e.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode PileUpMTAlg::initialize() {
  using namespace std::chrono;
  ATH_MSG_DEBUG("Initializing " << name() << "...");
  if (m_writeTrace) {
    std::string filename = std::format("pileup_trace_skipping-{}_{:%Y-%m-%dT%H%M}.txt",
                                       m_skippedHSEvents.value(),
                                       system_clock::now());
    if (!m_pileupTrace.init(filename)) {
      ATH_MSG_ERROR("Cannot append to file " << filename);
      return StatusCode::FAILURE;
    }
  }
  ATH_CHECK(m_skipEventIdxSvc.retrieve());
  ATH_CHECK(m_rngSvc.retrieve());
  if (m_fracLowPt != 0) {
    ATH_CHECK(m_lowptMBSvc.retrieve());
  }
  if (m_fracHighPt != 0) {
    ATH_CHECK(m_highptMBSvc.retrieve());
  }
  if (m_numCavern != 0) {
    ATH_CHECK(m_cavernMBSvc.retrieve());
  }
  if (m_numBeamGas != 0) {
    ATH_CHECK(m_beamgasMBSvc.retrieve());
  }
  if (m_numBeamHalo != 0) {
    ATH_CHECK(m_beamhaloMBSvc.retrieve());
  }
  ATH_CHECK(m_beamInt.retrieve());
  ATH_CHECK(m_beamLumi.retrieve());
  ATH_CHECK(m_puTools.retrieve());

  m_evtInfoContKey = "PileUpEventInfo";
  ATH_CHECK(m_evtInfoKey.initialize());
  ATH_CHECK(m_evtInfoContKey.initialize());
  ATH_CHECK(m_beamSpotKey.initialize());

  // Trace skipped events
  if (m_writeTrace) {
    auto handler = [](ISkipEventIdxSvc::EvtIter it,
                      ISkipEventIdxSvc::EvtIter end) -> StatusCode {
      std::string trace_buf{};
      auto trace = std::back_inserter(trace_buf);
      for (; it != end; ++it) {
        std::format_to(trace, "SKIPPING Run: {} LB: {} EVT: {} HS ID: {}\n",
                       it->runNum, it->lbNum, it->evtNum, it->evtIdx);
      }
      m_pileupTrace.print(trace_buf);
      return StatusCode::SUCCESS;
    };
    if (!m_skiptrace_written) {
      ATH_CHECK(m_skipEventIdxSvc->registerCallback(handler));
      m_skiptrace_written = true;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode PileUpMTAlg::finalize() {
  ATH_MSG_DEBUG("Finalizing " << name() << "...");
  //
  // Things that happen once at the end of the event loop go here
  //

  return StatusCode::SUCCESS;
}

StatusCode PileUpMTAlg::execute() {
  using PUType = xAOD::EventInfo::PileUpType;
  std::string trace_buf{};  // Hold trace of events.
  auto trace = std::back_inserter(trace_buf);
  ATH_MSG_DEBUG("Executing " << name() << "...");
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const auto& evtID = ctx.eventID();

  ATH_CHECK(evtStore().retrieve());
  setFilterPassed(false);  // optional: start with algorithm not passed

  // Code based on PileUpEventLoopMgr and PileUpToolsAlg (trying to extract the
  // core merging code) Read hard scatter
  std::unique_ptr<const xAOD::EventInfo> hsEvt = nullptr;
  std::unique_ptr<xAOD::EventAuxInfo> hsEvtAux = nullptr;
  ATH_CHECK(get_ei(*evtStore(), hsEvt, hsEvtAux));

  // Setup overlaid event
  SG::WriteHandle<xAOD::EventInfo> overlaidEvt(m_evtInfoKey, ctx);
  ATH_CHECK(overlaidEvt.record(std::make_unique<xAOD::EventInfo>(),
                               std::make_unique<xAOD::EventAuxInfo>()));
  *overlaidEvt = *hsEvt;  // copy in hard scatter
  overlaidEvt->setEvtStore(evtStore().get());
  overlaidEvt->clearSubEvents();

  // This was the problem. Need to fix overlaidEvt using context run and lb
  // number
  overlaidEvt->setRunNumber(evtID.run_number());
  overlaidEvt->setLumiBlock(evtID.lumi_block());
  overlaidEvt->setEventNumber(evtID.event_number());
  overlaidEvt->setBCID(evtID.bunch_crossing_id());
  overlaidEvt->setTimeStamp(evtID.time_stamp());
  overlaidEvt->setTimeStampNSOffset(evtID.time_stamp_ns_offset());
  // Set beam spot info
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};
  if (!beamSpotHandle.isValid()) {
    ATH_MSG_ERROR("Beam spot information not valid");
    return StatusCode::FAILURE;
  }

  overlaidEvt->setBeamPos(beamSpotHandle->beamPos()[Amg::x],
                          beamSpotHandle->beamPos()[Amg::y],
                          beamSpotHandle->beamPos()[Amg::z]);
  overlaidEvt->setBeamPosSigma(beamSpotHandle->beamSigma(0),
                               beamSpotHandle->beamSigma(1),
                               beamSpotHandle->beamSigma(2));
  overlaidEvt->setBeamPosSigmaXY(beamSpotHandle->beamSigmaXY());
  overlaidEvt->setBeamTiltXZ(beamSpotHandle->beamTilt(0));
  overlaidEvt->setBeamTiltYZ(beamSpotHandle->beamTilt(1));
  overlaidEvt->setBeamStatus(beamSpotHandle->beamStatus());

  // Pileup container
  SG::WriteHandle<xAOD::EventInfoContainer> puCont(m_evtInfoContKey, ctx);
  ATH_CHECK(puCont.record(std::make_unique<xAOD::EventInfoContainer>(),
                          std::make_unique<xAOD::EventInfoAuxContainer>()));

  // Get crossing number
  m_beamInt->selectT0(evtID.run_number(), evtID.event_number());
  overlaidEvt->setBCID(m_beamInt->getCurrentT0BunchCrossing());

  // Set simulation bit
  overlaidEvt->setEventTypeBitmask(hsEvt->eventTypeBitmask() |
                                   xAOD::EventInfo::IS_SIMULATION);

  // Set properties
  bool sf_updated = false;
  float lumi_sf = m_beamLumi->scaleFactor(evtID.run_number(),
                                          evtID.lumi_block(), sf_updated);
  float cur_avg_mu = lumi_sf * m_avgMu;
  overlaidEvt->setAverageInteractionsPerCrossing(cur_avg_mu);
  overlaidEvt->setActualInteractionsPerCrossing(m_beamInt->normFactor(0) *
                                                cur_avg_mu);

  // Trace
  if (m_writeTrace) {
    std::format_to(trace,
                   "Idx: {} Run: {} LB: {} EVT: {} "
                   "HS ID: {}\n",
                   ctx.evt(), evtID.run_number(), evtID.lumi_block(),
                   evtID.event_number(), m_lowptMBSvc->get_hs_id(ctx));
    auto bunch_pattern =
        rv::closed_iota(m_earliestDeltaBC.value(), m_latestDeltaBC.value()) |
        rv::transform(
            [this](int bc) { return int(m_beamInt->normFactor(bc)); }) |
#if RANGE_V3_VERSION >= 1200
        rv::chunk_by(std::equal_to{}) |
#else
        rv::group_by(std::equal_to{}) |
#endif
        rv::transform([](const auto& rng) {
          return std::format("{}{}", rng.size(), rng[0] == 0 ? 'E' : 'F');
        }) |
        ranges::to<std::vector<std::string>>;
    // Must use fmt::format here because std::format has no range formatting support
    fmt::format_to(trace, "mu = {}, central BCID = {}, bunch pattern = [{}]\n",
                   cur_avg_mu, m_beamInt->getCurrentT0BunchCrossing(),
                   fmt::join(bunch_pattern, " "));
  }
  // Copy subevents
  if (!hsEvt->subEvents().empty()) {
    for (const SubEvent& se : hsEvt->subEvents()) {
      addSubEvent(overlaidEvt.ptr(), se, puCont.ptr(), m_evtInfoContKey.key(),
                  evtStore().get());
    }
  } else {
    // if no subevents, add original event
    addSubEvent(overlaidEvt.ptr(), hsEvt.get(), 0, xAOD::EventInfo::Signal,
                puCont.ptr(), m_evtInfoContKey.key(), evtStore().get());
  }

  // Initialize MinbiasSvcs
  if (m_fracLowPt != 0) {
    ATH_CHECK(m_lowptMBSvc->beginHardScatter(ctx));
  }
  if (m_fracHighPt != 0) {
    ATH_CHECK(m_highptMBSvc->beginHardScatter(ctx));
  }
  if (m_numCavern != 0) {
    ATH_CHECK(m_cavernMBSvc->beginHardScatter(ctx));
  }
  if (m_numBeamHalo != 0) {
    ATH_CHECK(m_beamhaloMBSvc->beginHardScatter(ctx));
  }
  if (m_numBeamGas != 0) {
    ATH_CHECK(m_beamgasMBSvc->beginHardScatter(ctx));
  }

  std::uint32_t central_bcid = overlaidEvt->bcid();
  std::vector<std::uint32_t> bcid{};
  bcid.reserve(m_latestDeltaBC - m_earliestDeltaBC + 1);

  for (int bc = m_earliestDeltaBC; bc <= m_latestDeltaBC; ++bc) {
    bcid.push_back(get_BCID(bc, central_bcid));
  }

  // Setup tools
  for (auto&& tool : m_puTools) {
    // Reset filter -- Don't know if this is necessary
    tool->resetFilter();
  }

  // Now add the events
  std::uint64_t low_pt_count = 0;
  std::uint64_t high_pt_count = 0;
  std::uint64_t cavern_count = 0;
  std::uint64_t beam_halo_count = 0;
  std::uint64_t beam_gas_count = 0;
  auto now = std::chrono::high_resolution_clock::now();
  for (int bc = m_earliestDeltaBC; bc <= m_latestDeltaBC; ++bc) {
    if (m_beamInt->normFactor(bc) == 0.) {
      // skip empty bunch crossings
      continue;
    }
    std::vector<std::uint64_t> subevts_vec{};
    if (m_fracLowPt != 0) {
      if (m_writeTrace) {
        std::format_to(trace, "\tBC {:03} : LOW PT {} ", bc,
                       m_lowptMBSvc->getNumForBunch(ctx, bc));
      }
      for (std::size_t i = 0; i < m_lowptMBSvc->getNumForBunch(ctx, bc); ++i) {
        ATH_CHECK(add_subevt(bcid, overlaidEvt, puCont, m_lowptMBSvc,
                             PUType::MinimumBias, bc, ctx, low_pt_count,
                             subevts_vec));
        ++low_pt_count;
      }
    }
    if (m_fracHighPt != 0) {
      if (m_writeTrace) {
        std::format_to(trace, "HIGH PT {} | ",
                       m_highptMBSvc->getNumForBunch(ctx, bc));
      }
      for (std::size_t i = 0; i < m_highptMBSvc->getNumForBunch(ctx, bc); ++i) {
        ATH_CHECK(add_subevt(bcid, overlaidEvt, puCont, m_highptMBSvc,
                             PUType::HighPtMinimumBias, bc, ctx, high_pt_count,
                             subevts_vec));
        ++high_pt_count;
      }
    }
    if (m_numCavern != 0) {
      if (m_writeTrace) {
        std::format_to(trace, "CAVERN {} | ",
                       m_cavernMBSvc->getNumForBunch(ctx, bc));
      }
      for (std::size_t i = 0; i < m_cavernMBSvc->getNumForBunch(ctx, bc); ++i) {
        ATH_CHECK(add_subevt(bcid, overlaidEvt, puCont, m_cavernMBSvc,
                             PUType::Cavern, bc, ctx, cavern_count,
                             subevts_vec));
        ++cavern_count;
      }
    }
    if (m_numBeamHalo != 0) {
      if (m_writeTrace) {
        std::format_to(trace, "BEAM HALO {} | ",
                       m_beamhaloMBSvc->getNumForBunch(ctx, bc));
      }
      for (std::size_t i = 0; i < m_beamhaloMBSvc->getNumForBunch(ctx, bc);
           ++i) {
        ATH_CHECK(add_subevt(bcid, overlaidEvt, puCont, m_beamhaloMBSvc,
                             PUType::HaloGas, bc, ctx, beam_halo_count,
                             subevts_vec));
        ++beam_halo_count;
      }
    }
    if (m_numBeamGas != 0) {
      if (m_writeTrace) {
        std::format_to(trace, "BEAM GAS {} | ",
                       m_beamgasMBSvc->getNumForBunch(ctx, bc));
      }
      for (std::size_t i = 0; i < m_beamgasMBSvc->getNumForBunch(ctx, bc);
           ++i) {
        ATH_CHECK(add_subevt(bcid, overlaidEvt, puCont, m_beamgasMBSvc,
                             PUType::HaloGas, bc, ctx, beam_gas_count,
                             subevts_vec));
        ++beam_gas_count;
      }
    }
    if (m_writeTrace) {
      std::format_to(trace, "TOTAL {} | HASH {:08X}\n", subevts_vec.size(),
                     xxh3::hash64(subevts_vec));
    }
  }
  if (m_writeTrace) {
    std::format_to(trace, "\n");
    m_pileupTrace.print(trace_buf);
  }

  for (auto&& tool : m_puTools) {
    try {
      ATH_CHECK(tool->processAllSubEvents(ctx));
    } catch (const std::exception& e) {
      ATH_MSG_ERROR("Caught exception running " << tool.name() << ": "
                                                << e.what());
      return StatusCode::FAILURE;
    }
    // Propagate filter result
    if (!tool->filterPassed()) {
      setFilterPassed(false);
    }
  }
  ATH_MSG_DEBUG(std::format("***** Took {:%OMm %OSs} to process all subevents",
                            std::chrono::high_resolution_clock::now() - now));
  //
  // Save hash (direct copy from PileUpEventLoopMgr)
  PileUpHashHelper pileUpHashHelper;
  pileUpHashHelper.addToHashSource(overlaidEvt.cptr());
  ATH_MSG_VERBOSE("Pile-up hash source:" << pileUpHashHelper.hashSource());

  // Calculate and set hash
  uuid_t pileUpHash;
  pileUpHashHelper.calculateHash(pileUpHash);
  overlaidEvt->setPileUpMixtureID(
      PileUpHashHelper::uuidToPileUpMixtureId(pileUpHash));
  ATH_MSG_DEBUG("PileUpMixtureID = " << overlaidEvt->pileUpMixtureID());

  setFilterPassed(true);  // if got here, assume that means algorithm passed
  if (m_fracLowPt != 0) {
    ATH_CHECK(m_lowptMBSvc->endHardScatter(ctx));
  }
  if (m_fracHighPt != 0) {
    ATH_CHECK(m_highptMBSvc->endHardScatter(ctx));
  }
  if (m_numCavern != 0) {
    ATH_CHECK(m_cavernMBSvc->endHardScatter(ctx));
  }
  if (m_numBeamHalo != 0) {
    ATH_CHECK(m_beamhaloMBSvc->endHardScatter(ctx));
  }
  if (m_numBeamGas != 0) {
    ATH_CHECK(m_beamgasMBSvc->endHardScatter(ctx));
  }
  return StatusCode::SUCCESS;
}
