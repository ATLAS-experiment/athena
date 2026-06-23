/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
    Implemented by Zackary Alegria following instructions from the JETM
    twiki recommendation on Dijet sample normalization
    https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEtMissMCSamples#Dijet_normalization_procedure_HS
    Updated by Baptiste Ravina <baptiste.ravina@cern.ch>
*/

#include "AsgAnalysisAlgorithms/HSTPFilterAlg.h"

#include <EventBookkeeperTools/FilterReporter.h>

namespace CP {

StatusCode HSTPFilterAlg::initialize() {

  ANA_CHECK(m_truthHSCollection.initialize());
  ANA_CHECK(m_truthPUCollection.initialize());

  ANA_CHECK(m_filterParams.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HSTPFilterAlg::execute(const EventContext &ctx) const {

  FilterReporter filter(m_filterParams, false, ctx);

  SG::ReadHandle<xAOD::JetContainer> truthHS_jets(m_truthHSCollection, ctx);
  SG::ReadHandle<xAOD::JetContainer> truthPU_jets(m_truthPUCollection, ctx);

  double pT_j1_truthHS = 5000.; // In the rare case of no truth HS jets, set to 5 GeV
  double pT_j1_truthPU = 0.;

  if (truthHS_jets->size()) {
    auto max_it = std::max_element(truthHS_jets->begin(), truthHS_jets->end(),
                                   [](const xAOD::Jet* a, const xAOD::Jet* b) {
                                     return a->pt() < b->pt();
                                   });
    pT_j1_truthHS = (**max_it).pt();
  }
  if (truthPU_jets->size()) {
    auto max_it = std::max_element(truthPU_jets->begin(), truthPU_jets->end(),
                                   [](const xAOD::Jet* a, const xAOD::Jet* b) {
                                     return a->pt() < b->pt();
                                   });
    pT_j1_truthPU = (**max_it).pt();
  }

  bool pass_HSTP_filter = pT_j1_truthHS > pT_j1_truthPU;
  filter.setPassed(pass_HSTP_filter);

  return StatusCode::SUCCESS;

}

StatusCode HSTPFilterAlg::finalize() {

  ANA_MSG_INFO(m_filterParams.summary());

  return StatusCode::SUCCESS;
}

}  // namespace CP
