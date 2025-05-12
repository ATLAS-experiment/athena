/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "AsgAnalysisAlgorithms/PDFinfoAlg.h"

namespace CP {

StatusCode PDFinfoAlg::initialize() {

  ANA_CHECK(m_truthEventsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PDFinfoAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_truthEventsKey, ctx);

  const xAOD::EventInfo *evtInfo = 0;
  ANA_CHECK(evtStore()->retrieve(evtInfo, "EventInfo"));

  // accessors
  static const SG::AuxElement::ConstAccessor<int> acc_pdfid1("PDFID1");
  static const SG::AuxElement::ConstAccessor<int> acc_pdfid2("PDFID2");
  static const SG::AuxElement::ConstAccessor<int> acc_pdgid1("PDGID1");
  static const SG::AuxElement::ConstAccessor<int> acc_pdgid2("PDGID2");
  static const SG::AuxElement::ConstAccessor<float> acc_Q("Q");
  static const SG::AuxElement::ConstAccessor<float> acc_X1("X1");
  static const SG::AuxElement::ConstAccessor<float> acc_X2("X2");
  static const SG::AuxElement::ConstAccessor<float> acc_XF1("XF1");
  static const SG::AuxElement::ConstAccessor<float> acc_XF2("XF2");

  // decorators
  static const SG::AuxElement::Decorator<int> dec_pdfid1("PDFID1");
  static const SG::AuxElement::Decorator<int> dec_pdfid2("PDFID2");
  static const SG::AuxElement::Decorator<int> dec_pdgid1("PDGID1");
  static const SG::AuxElement::Decorator<int> dec_pdgid2("PDGID2");
  static const SG::AuxElement::Decorator<float> dec_Q("Q");
  static const SG::AuxElement::Decorator<float> dec_X1("X1");
  static const SG::AuxElement::Decorator<float> dec_X2("X2");
  static const SG::AuxElement::Decorator<float> dec_XF1("XF1");
  static const SG::AuxElement::Decorator<float> dec_XF2("XF2");

  // for now we only look at the 0th element - do we need any others?
  for (const auto* truthEvent : *truthEvents) {

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

    // skip any other element
    break;
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
