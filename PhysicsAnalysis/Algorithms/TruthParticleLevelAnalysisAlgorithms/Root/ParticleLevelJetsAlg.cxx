/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelJetsAlg.h"

#include <AsgDataHandles/ReadDecorHandle.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

StatusCode ParticleLevelJetsAlg::initialize() {

  ANA_CHECK(m_jetsKey.initialize());
  ANA_CHECK(m_eventInfoKey.initialize());
  ANA_CHECK(m_truthLabelKey.initialize());
  ANA_CHECK(m_decNumTruthBJetsKey.initialize());
  ANA_CHECK(m_decNumTruthCJetsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelJetsAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey, ctx);
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

  // accessors and decorators
  SG::ReadDecorHandle<xAOD::JetContainer, int> acc_flav(m_truthLabelKey, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_nBJets(
      m_decNumTruthBJetsKey, ctx);
  SG::WriteDecorHandle<xAOD::EventInfo, int> dec_nCJets(
      m_decNumTruthCJetsKey, ctx);

  // decoration availability is a property of the whole container
  const bool hasFlav = acc_flav.isAvailable();

  // the number of b- and c-jets without any event cuts applied
  int num_bjets(0), num_cjets(0);

  for (const auto* jet : *jets) {

    // check the flavour label of the jet
    if (hasFlav) {
      int flavourLabel = acc_flav(*jet);
      if (flavourLabel == 5)
        num_bjets++;
      if (flavourLabel == 4)
        num_cjets++;
    } else {
      if (!m_warnedMissingLabel.exchange(true))
        ANA_MSG_WARNING(
            "Truth jet is missing the decoration: HadronConeExclTruthLabelID.");
      num_bjets = num_cjets = -999;
      break;
    }
  }

  // decorate the EventInfo with the number of b- and c-jets
  dec_nBJets(*eventInfo) = num_bjets;
  dec_nCJets(*eventInfo) = num_cjets;

  return StatusCode::SUCCESS;
}

}  // namespace CP
