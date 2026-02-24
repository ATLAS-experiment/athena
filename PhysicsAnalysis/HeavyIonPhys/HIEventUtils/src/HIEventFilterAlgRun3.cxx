/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/HIEventFilterAlgRun3.h"

#include <GaudiKernel/StatusCode.h>

#include "EventBookkeeperTools/FilterReporter.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/WriteDecorHandle.h"

HI::HIEventFilterAlgRun3::HIEventFilterAlgRun3(const std::string& name,
                                               ISvcLocator* pSvcLocator)
    : ::AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode HI::HIEventFilterAlgRun3::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_tracksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_verticesKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_hiEventShapeKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_zdcKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_decisionBitsKey.initialize());
  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
  ATH_CHECK(m_filterParams.initialize());

  if (m_useIonDataTypeDefaultMask) {
    if (m_selectionMask.value() !=
        static_cast<mask_t>(HI::SelectionMask::NoEventError)) {
      ATH_MSG_ERROR(
          "The selection mask is set while the flag UseIonDataTypeDefaultMask");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO(
        "Will use selection cuts that are default for the data that is "
        "processed");
  }
  return StatusCode::SUCCESS;
}

StatusCode HI::HIEventFilterAlgRun3::execute(const EventContext& ctx) const {
  FilterReporter filter(m_filterParams, true, ctx);
  mask_t mask = 0;  // this mask will be filled below
  auto eventInfoHandle = SG::makeHandle(m_eventInfoKey, ctx);
  if (m_tool->noDetectorError(eventInfoHandle.cptr())) {
    store(HI::SelectionMask::NoEventError, mask);
  }
  auto mon_fcalEt = Monitored::Scalar<float>("fcalEt");
  auto mon_zdcE = Monitored::Scalar<float>("zdcE");
  auto mon_nTrk = Monitored::Scalar<float>("nTrk");

  // go over required masks and ask tool if cut is passed
  const HI::IonDataType period = m_tool->toDataType(eventInfoHandle.cptr());

  ATH_MSG_DEBUG("Decoded IonDataType to be " << HI::toString(period));

  // dive the mask by period or by configuration
  const mask_t maskToUse = m_useIonDataTypeDefaultMask
                               ? m_tool->defaultMaskForPeriod(period)
                               : m_selectionMask.value();
  ATH_MSG_DEBUG("Mask requested " << maskToString(maskToUse));

  if (isRequested(maskToUse, HI::SelectionMask::PUFCalVsZDCAny)) {
    auto esHandle = SG::makeHandle(m_hiEventShapeKey, ctx);
    auto zdcHandle = SG::makeHandle(m_zdcKey, ctx);
    if (!m_monTool.empty()) {
      mon_fcalEt = m_tool->fcalEt(period, esHandle.cptr());
      mon_zdcE = m_tool->zdcE(period, zdcHandle.cptr());
    }

    if (m_tool->puZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                            HI::PileupVariation::Tight)) {
      store(HI::SelectionMask::PUFCalVsZDCTight, mask);
    }
    if (m_tool->puZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                            HI::PileupVariation::Nominal)) {
      store(HI::SelectionMask::PUFCalVsZDCNominal, mask);
    }
    if (m_tool->puZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                            HI::PileupVariation::Loose)) {
      store(HI::SelectionMask::PUFCalVsZDCLoose, mask);
    }
  }

  if (isRequested(maskToUse, HI::SelectionMask::PUZDCPresampler)) {
    auto zdcHandle = SG::makeHandle(m_zdcKey, ctx);
    if (m_tool->puZDCPresampler(period, zdcHandle.cptr(),
                                HI::PileupVariation::Nominal)) {
      store(HI::SelectionMask::PUZDCPresampler, mask);
    }
  }

  if (isRequested(maskToUse, HI::SelectionMask::PUOOSingleVertexNominal)) {
    auto vertexHandle = SG::makeHandle(m_verticesKey, ctx);
    if (m_tool->puOOVertexCuts(period, vertexHandle.cptr())) {
      store(HI::SelectionMask::PUOOSingleVertexNominal, mask);
    }
  }

  if (isRequested(maskToUse, HI::SelectionMask::PUFCalVsNTrackAny)) {
    auto esHandle = SG::makeHandle(m_hiEventShapeKey, ctx);
    auto vertexHandle = SG::makeHandle(m_verticesKey, ctx);
    auto tracksHandle = SG::makeHandle(m_tracksKey, ctx);
    if (!m_monTool.empty()) {
      mon_fcalEt = m_tool->fcalEt(period, esHandle.cptr());
      mon_nTrk = m_tool->nTrk(period, tracksHandle.cptr(), vertexHandle.cptr());
    }

    if (m_tool->puFCalVsNtracks(period, esHandle.cptr(), tracksHandle.cptr(),
                                vertexHandle.cptr(),
                                HI::PileupVariation::Loose)) {
      store(HI::SelectionMask::PUFCalVsNTrackLoose, mask);
    }
    if (m_tool->puFCalVsNtracks(period, esHandle.cptr(), tracksHandle.cptr(),
                                vertexHandle.cptr(),
                                HI::PileupVariation::Nominal)) {
      store(HI::SelectionMask::PUFCalVsNTrackNominal, mask);
    }
  }

  const bool filterDecision = (maskToUse & mask) == maskToUse;
  ATH_MSG_DEBUG("Mask produced " << maskToString(mask) << " filter decision "
                                 << filterDecision);

  auto mon_passed = Monitored::Scalar<bool>("passed", filterDecision);
  auto mon_failed = Monitored::Scalar<bool>("failed", not filterDecision);
  if (m_doFilter)
    filter.setPassed(filterDecision);
  fillCounters(mask);

  Monitored::Group(m_monTool, mon_fcalEt, mon_zdcE, mon_nTrk, mon_passed, mon_failed);
  // record mask for client as decoration of EventInfo object
  SG::WriteDecorHandle<xAOD::EventInfo, mask_t> handle =
      SG::makeHandle<mask_t>(m_decisionBitsKey, ctx);
  handle(*handle) = mask;

  return StatusCode::SUCCESS;
}

StatusCode HI::HIEventFilterAlgRun3::finalize() {
  ATH_MSG_INFO(m_filterParams.summary());
  return StatusCode::SUCCESS;
}

void HI::HIEventFilterAlgRun3::fillCounters(
    const HI::HIEventFilterAlgRun3::mask_t m) const {
  std::bitset<8 * sizeof(m)> bits(m);
  for (size_t b = 0; b < bits.size(); ++b) {
    if (bits[b]) {
      // m_counters[b]++;
    }
  }
}

std::string HI::HIEventFilterAlgRun3::maskToString(
    const HI::HIEventFilterAlgRun3::mask_t m) const {
  std::bitset<8 * sizeof(m)> bits(m);
  std::string ret;
  for (size_t b = 0; b < bits.size(); ++b) {
    if (bits[b]) {
      ret += std::to_string(b) + ":" +
             toString(static_cast<HI::SelectionMask>(1 << b)) + " ";
    }
  }
  return ret;
}
