/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/HIEventFilterAlgRun3.h"

#include <GaudiKernel/StatusCode.h>

#include "StoreGate/WriteDecorHandle.h"

HI::HIEventFilterAlgRun3::HIEventFilterAlgRun3(const std::string& name,
                                               ISvcLocator* pSvcLocator)
    : ::AthFilterAlgorithm(name, pSvcLocator) {}

StatusCode HI::HIEventFilterAlgRun3::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_tracksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_verticesKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_hiEventShapeKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_zdcKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_decisionBitsKey.initialize());

  // consistency check
  if (isRequested(HI::SelectionMask::PUOOVertexAny) and
      m_verticesKey.key().empty()) {
    ATH_MSG_ERROR(
        "Vertices cut for OO required but vertex container name is not set");
    return StatusCode::FAILURE;
  }

  if (isRequested(HI::SelectionMask::PUFCalVsNTrackAny) and
      m_tracksKey.key().empty()) {
    ATH_MSG_ERROR(
        "PU cut using tracks required but tracks container name is not set");
    return StatusCode::FAILURE;
  }

  if ((isRequested(HI::SelectionMask::PUFCalVsNTrackAny) or
       isRequested(HI::SelectionMask::PUFCalVsZDCAny)) and
      m_hiEventShapeKey.key().empty()) {
    ATH_MSG_ERROR(
        "FCAL cut required for PU of FCAL vs N tracks but ES container name is "
        "not set");
    return StatusCode::FAILURE;
  }

  if (isRequested(HI::SelectionMask::PUFCalVsZDCAny) and m_zdcKey.key().empty()) {
    ATH_MSG_ERROR(
        "ZDC cut required for PU FCAL vs ZDC but zdc container name is not "
        "set");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode HI::HIEventFilterAlgRun3::execute() {
  mask_t mask = 0;  // this mask will be filled below
  auto eventInfoHandle = SG::makeHandle(m_eventInfoKey);
  if (m_tool->noDetectorError(eventInfoHandle.cptr())) {
    store(HI::SelectionMask::NoEventError, mask);
  }

  // go over required masks and ask tool if cut is passed

  if (isRequested(HI::SelectionMask::PUFCalVsNTrackAny)) {
    ATH_MSG_WARNING("Not implemented yet");
    // obtain tracks & FCal

    // get the FCAL ET

    // ask tool for decisions

    // set mask

  }

  const bool filterDecision = (m_selectionMask & mask) == m_selectionMask;
  ATH_MSG_DEBUG("Mask produced " << std::bitset<8 * sizeof(mask_t)>(mask)
                                 << " filter decision " << filterDecision);
  if (m_doFilter)
    setFilterPassed(filterDecision);

  // record mask for client as decoration of EventInfo object
  SG::WriteDecorHandle<xAOD::EventInfo, mask_t> handle =
      SG::makeHandle<mask_t>(m_decisionBitsKey);
  handle(*handle) = mask;

  return StatusCode::SUCCESS;
}
