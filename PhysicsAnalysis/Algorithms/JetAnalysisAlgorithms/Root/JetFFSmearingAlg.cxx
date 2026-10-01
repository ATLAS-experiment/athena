/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

//
// includes
//

#include <JetAnalysisAlgorithms/JetFFSmearingAlg.h>

//
// method implementations
//

namespace CP {

StatusCode JetFFSmearingAlg ::initialize() {
  ANA_CHECK(m_FFSmearingTool.retrieve());
  ANA_CHECK(m_jetHandle.initialize(m_systematicsList));
  ANA_CHECK(m_preselection.initialize(m_systematicsList, m_jetHandle,
                                      SG::AllowEmpty));

  ANA_CHECK(m_systematicsList.addSystematics(*m_FFSmearingTool));
  ANA_CHECK(m_systematicsList.initialize());
  ANA_CHECK(m_outOfValidity.initialize());

  // FIXME: while the FFJetSmearingTool is unable to accept external systematics,
  // we make our own set of systematics names
  const CP::SystematicSet& recommendedSysts = m_FFSmearingTool->recommendedSystematics();
  for (const auto &sys : recommendedSysts) m_systematicsNames.insert(CP::SystematicSet({sys}).name());

  return StatusCode::SUCCESS;
}

StatusCode JetFFSmearingAlg ::execute(const EventContext& ctx) {
  for (const auto &sys : m_systematicsList.systematicsVector()) {
    xAOD::JetContainer *jets = nullptr;
    ANA_CHECK(m_jetHandle.getCopy(jets, sys, ctx));

    // FIXME: while the FFJetSmearingTool is unable to accept external systematics,
    // only run over the internal ones
    if ( !m_systematicsNames.contains(sys.name()) ) continue;

    ANA_CHECK(m_FFSmearingTool->applySystematicVariation(sys));

    for (xAOD::Jet *jet : *jets) {
      if (m_preselection.getBool(*jet, sys)) {
        ANA_CHECK_CORRECTION(m_outOfValidity, *jet,
                             m_FFSmearingTool->applyCorrection(*jet));
      }
    }
  }
  return StatusCode::SUCCESS;
}
}  // namespace CP
