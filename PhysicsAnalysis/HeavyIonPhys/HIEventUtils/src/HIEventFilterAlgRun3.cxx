/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/HIEventFilterAlgRun3.h"

#include <GaudiKernel/StatusCode.h>

#include "AthenaMonitoringKernel/Monitored.h"
#include "EventBookkeeperTools/FilterReporter.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
using namespace HI;

HIEventFilterAlgRun3::HIEventFilterAlgRun3(const std::string& name,
                                           ISvcLocator* pSvcLocator)
    : ::AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode HIEventFilterAlgRun3::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_tracksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_verticesKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_hiEventShapeKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_zdcKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_clustersKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_decisionBitsKey.initialize());
  ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));
  ATH_CHECK(m_filterParams.initialize());

  if (m_useIonDataTypeDefaultMask) {
    if (m_selectionMask.value() !=
        static_cast<mask_t>(SelectionMask::NoEventError)) {
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

StatusCode HIEventFilterAlgRun3::execute(const EventContext& ctx) const {
  FilterReporter filter(m_filterParams, true, ctx);
  mask_t mask = 0;  // this mask will be filled below
  auto eventInfoHandle = SG::makeHandle(m_eventInfoKey, ctx);
  if (m_tool->noDetectorError(eventInfoHandle.cptr())) {
    store(SelectionMask::NoEventError, mask);
  }
  auto mon_fcalEt = Monitored::Scalar<float>("fcalEt");
  auto mon_zdcE = Monitored::Scalar<float>("zdcE");
  auto mon_nTrk = Monitored::Scalar<float>("nTrk");
  auto mon_zdcPreSampleA = Monitored::Scalar<float>("zdcPreSampleA");
  auto mon_zdcPreSampleC = Monitored::Scalar<float>("zdcPreSampleC");

  // go over required masks and ask tool if cut is passed
  const IonDataType period = m_tool->toDataType(eventInfoHandle.cptr());

  ATH_MSG_DEBUG("Decoded IonDataType to be " << toString(period));

  // dive the mask by period or by configuration
  const mask_t maskToUse = m_useIonDataTypeDefaultMask
                               ? m_tool->defaultMaskForPeriod(period)
                               : m_selectionMask.value();
  ATH_MSG_DEBUG("Mask requested " << maskToString(maskToUse));
  if (isRequested(maskToUse, SelectionMask::NoPUFCalVsZDCAny)) {
    auto esHandle = SG::makeHandle(m_hiEventShapeKey, ctx);
    auto zdcHandle = SG::makeHandle(m_zdcKey, ctx);
    if (!m_monTool.empty()) {
      mon_fcalEt = m_tool->fcalEt(period, esHandle.cptr());
      mon_zdcE = m_tool->zdcE(period, zdcHandle.cptr());
    }

    if (m_tool->noPUZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                              PileupVariation::Tight)) {
      store(SelectionMask::NoPUFCalVsZDCTight, mask);
    }
    if (m_tool->noPUZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                              PileupVariation::Nominal)) {
      store(SelectionMask::NoPUFCalVsZDCNominal, mask);
    }
    if (m_tool->noPUZDCvsFCal(period, esHandle.cptr(), zdcHandle.cptr(),
                              PileupVariation::Loose)) {
      store(SelectionMask::NoPUFCalVsZDCLoose, mask);
    }
  }

  if (isRequested(maskToUse, SelectionMask::NoPUZDCPresampler)) {
    auto zdcHandle = SG::makeHandle(m_zdcKey, ctx);
    if (!m_monTool.empty()) {
      std::tie(mon_zdcPreSampleA, mon_zdcPreSampleC) =
          m_tool->ZDCPresamplerAmps(zdcHandle.cptr());
    }
    if (m_tool->noPUZDCPresampler(period, zdcHandle.cptr(),
                                  PileupVariation::Nominal)) {
      store(SelectionMask::NoPUZDCPresampler, mask);
    }
  }

  if (isRequested(maskToUse, SelectionMask::NoPUOOSingleVertexNominal)) {
    auto vertexHandle = SG::makeHandle(m_verticesKey, ctx);
    if (m_tool->noPUOOVertexCuts(period, vertexHandle.cptr())) {
      store(SelectionMask::NoPUOOSingleVertexNominal, mask);
    }
  }

  if (isRequested(maskToUse, SelectionMask::NoPUFCalVsNTrackAny)) {
    auto esHandle = SG::makeHandle(m_hiEventShapeKey, ctx);
    auto vertexHandle = SG::makeHandle(m_verticesKey, ctx);
    auto tracksHandle = SG::makeHandle(m_tracksKey, ctx);
    if (!m_monTool.empty()) {
      mon_fcalEt = m_tool->fcalEt(period, esHandle.cptr());
      mon_nTrk = m_tool->nTrk(period, tracksHandle.cptr(), vertexHandle.cptr());
    }

    if (m_tool->noPUFCalVsNtracks(period, esHandle.cptr(), tracksHandle.cptr(),
                                  vertexHandle.cptr(),
                                  PileupVariation::Loose)) {
      store(SelectionMask::NoPUFCalVsNTrackLoose, mask);
    }
    if (m_tool->noPUFCalVsNtracks(period, esHandle.cptr(), tracksHandle.cptr(),
                                  vertexHandle.cptr(),
                                  PileupVariation::Nominal)) {
      store(SelectionMask::NoPUFCalVsNTrackNominal, mask);
    }
  }
  if (isRequested(maskToUse, SelectionMask::TopoClusterInFCal)) {
    auto clustersHandle = SG::makeHandle(m_clustersKey, ctx);
    if (m_tool->tcInFCalPresent(period, clustersHandle.cptr())) {
      store(SelectionMask::TopoClusterInFCal, mask);
    }
  }

  const bool filterDecision = (maskToUse & mask) == maskToUse;
  ATH_MSG_DEBUG("Mask produced " << maskToString(mask) << " filter decision "
                                 << filterDecision);

  auto mon_passed = Monitored::Scalar<bool>("passed", filterDecision);
  auto mon_failed = Monitored::Scalar<bool>("failed", not filterDecision);
  auto mon_PUFCalVsZDCAny_passed = Monitored::Scalar<bool>(
      "PUFCalVsZDCAny_passed", isSet(mask, SelectionMask::NoPUFCalVsZDCAny));
  auto mon_PUFCalVsZDCAny_failed = Monitored::Scalar<bool>(
      "PUFCalVsZDCAny_failed", isSet(mask, SelectionMask::NoPUFCalVsZDCAny));
  auto mon_PUFCalVsNTrackAny_passed =
      Monitored::Scalar<bool>("PUFCalVsNTrackAny_passed",
                              isSet(mask, SelectionMask::NoPUFCalVsNTrackAny));
  auto mon_PUFCalVsNTrackAny_failed =
      Monitored::Scalar<bool>("PUFCalVsNTrackAny_failed",
                              isSet(mask, SelectionMask::NoPUFCalVsNTrackAny));

  auto mon_OO_1 = Monitored::Scalar<bool>(
      "OO_1_passed", isSet(mask, SelectionMask::NoPUFCalVsNTrackNominal));
  auto mon_OO_2 = Monitored::Scalar<bool>(
      "OO_2_passed",
      mon_OO_1 && isSet(mask, SelectionMask::NoPUOOSingleVertexNominal));
  auto mon_OO_3 = Monitored::Scalar<bool>(
      "OO_3_passed",
      mon_OO_2 && isSet(mask, SelectionMask::NoPUFCalVsZDCNominal));
  auto mon_OO_4 = Monitored::Scalar<bool>(
      "OO_4_passed", mon_OO_3 && isSet(mask, SelectionMask::TopoClusterInFCal));
  ATH_MSG_VERBOSE("OO mon decisions " << mon_OO_1 << " " << mon_OO_2 << " "
                                      << mon_OO_3 << " " << mon_OO_4);
  auto mon_NoPUZDCPresampler_passed =
      Monitored::Scalar<bool>("NoPUZDCPresampler_passed",
                              isSet(mask, SelectionMask::NoPUZDCPresampler));
  auto mon_NoPUZDCPresampler_failed =
      Monitored::Scalar<bool>("NoPUZDCPresampler_failed",
                              isSet(mask, SelectionMask::NoPUZDCPresampler));

  if (m_doFilter)
    filter.setPassed(filterDecision);
  fillCounters(mask);

  Monitored::Group(
      m_monTool, mon_fcalEt, mon_zdcE, mon_nTrk, mon_passed, mon_failed,
      mon_PUFCalVsZDCAny_passed, mon_PUFCalVsZDCAny_failed,
      mon_PUFCalVsNTrackAny_passed, mon_PUFCalVsNTrackAny_failed,
      mon_zdcPreSampleA, mon_zdcPreSampleC, mon_NoPUZDCPresampler_failed,
      mon_NoPUZDCPresampler_passed, mon_OO_1, mon_OO_2, mon_OO_3, mon_OO_4);
  // record mask for client as decoration of EventInfo object
  SG::WriteDecorHandle<xAOD::EventInfo, mask_t> handle =
      SG::makeHandle<mask_t>(m_decisionBitsKey, ctx);
  handle(*handle) = mask;

  return StatusCode::SUCCESS;
}

StatusCode HIEventFilterAlgRun3::finalize() {
  ATH_MSG_INFO(m_filterParams.summary());
  return StatusCode::SUCCESS;
}

void HIEventFilterAlgRun3::fillCounters(
    const HIEventFilterAlgRun3::mask_t m) const {
  std::bitset<8 * sizeof(m)> bits(m);
  for (size_t b = 0; b < bits.size(); ++b) {
    if (bits[b]) {
      // m_counters[b]++;
    }
  }
}

std::string HIEventFilterAlgRun3::maskToString(
    const HIEventFilterAlgRun3::mask_t m) const {
  std::bitset<8 * sizeof(m)> bits(m);
  std::string ret;
  for (size_t b = 0; b < bits.size(); ++b) {
    if (bits[b]) {
      ret += std::to_string(b) + ":" +
             toString(static_cast<SelectionMask>(1 << b)) + " ";
    }
  }
  return ret;
}
