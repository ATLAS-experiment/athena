/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "AsgAnalysisAlgorithms/PDFinfoAlg.h"

#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

StatusCode PDFinfoAlg::initialize() {

  ANA_CHECK(m_truthEventsKey.initialize());
  ANA_CHECK(m_eventInfoKey.initialize());

  ANA_CHECK(m_pdfid1Key.initialize());
  ANA_CHECK(m_pdfid2Key.initialize());
  ANA_CHECK(m_pdgid1Key.initialize());
  ANA_CHECK(m_pdgid2Key.initialize());
  ANA_CHECK(m_QKey.initialize());
  ANA_CHECK(m_X1Key.initialize());
  ANA_CHECK(m_X2Key.initialize());
  ANA_CHECK(m_XF1Key.initialize());
  ANA_CHECK(m_XF2Key.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PDFinfoAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_truthEventsKey, ctx);
  ANA_CHECK(truthEvents.isValid());

  SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfoKey, ctx);
  ANA_CHECK(evtInfo.isValid());

  // for now we only look at the 0th element - do we need any others?
  if (truthEvents->empty()) {
    ANA_MSG_WARNING("TruthEvents container is empty, PDF info decorations will not be written for this event");
    return StatusCode::SUCCESS;
  }
  const xAOD::TruthEvent *truthEvent = truthEvents->front();

  // accessors
  static const SG::ConstAccessor<int> acc_pdfid1("PDFID1");
  static const SG::ConstAccessor<int> acc_pdfid2("PDFID2");
  static const SG::ConstAccessor<int> acc_pdgid1("PDGID1");
  static const SG::ConstAccessor<int> acc_pdgid2("PDGID2");
  static const SG::ConstAccessor<float> acc_Q("Q");
  static const SG::ConstAccessor<float> acc_X1("X1");
  static const SG::ConstAccessor<float> acc_X2("X2");
  static const SG::ConstAccessor<float> acc_XF1("XF1");
  static const SG::ConstAccessor<float> acc_XF2("XF2");

  // decorators
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_pdfid1(m_pdfid1Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_pdfid2(m_pdfid2Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_pdgid1(m_pdgid1Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_pdgid2(m_pdgid2Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, float> dec_Q(m_QKey, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, float> dec_X1(m_X1Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, float> dec_X2(m_X2Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, float> dec_XF1(m_XF1Key, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, float> dec_XF2(m_XF2Key, ctx);

  // decorate onto EventInfo
  dec_pdfid1(*evtInfo) = acc_pdfid1(*truthEvent);
  dec_pdfid2(*evtInfo) = acc_pdfid2(*truthEvent);
  dec_pdgid1(*evtInfo) = acc_pdgid1(*truthEvent);
  dec_pdgid2(*evtInfo) = acc_pdgid2(*truthEvent);
  dec_Q(*evtInfo) = acc_Q(*truthEvent);
  dec_X1(*evtInfo) = acc_X1(*truthEvent);
  dec_X2(*evtInfo) = acc_X2(*truthEvent);
  dec_XF1(*evtInfo) = acc_XF1(*truthEvent);
  dec_XF2(*evtInfo) = acc_XF2(*truthEvent);

  return StatusCode::SUCCESS;
}

}  // namespace CP
