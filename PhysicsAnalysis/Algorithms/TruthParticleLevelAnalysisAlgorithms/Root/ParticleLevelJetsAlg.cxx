/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "TruthParticleLevelAnalysisAlgorithms/ParticleLevelJetsAlg.h"

namespace CP {

// accessors and decorators
// these are done at file-level to register types with output algorithms
static const SG::ConstAccessor<int> acc_flav(
    "HadronConeExclTruthLabelID");
static const SG::Decorator<int> dec_nBJets(
    "num_truth_bjets_nocuts");
static const SG::Decorator<int> dec_nCJets(
    "num_truth_cjets_nocuts");

StatusCode ParticleLevelJetsAlg::initialize() {

  ANA_CHECK(m_jetsKey.initialize());
  ANA_CHECK(m_eventInfoKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ParticleLevelJetsAlg::execute(const EventContext &ctx) const {

  SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey, ctx);
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

  // the number of b- and c-jets without any event cuts applied
  int num_bjets(0), num_cjets(0);

  for (const auto* jet : *jets) {

    // check the flavour label of the jet
    if (acc_flav.isAvailable(*jet)) {
      int flavourLabel = acc_flav(*jet);
      if (flavourLabel == 5)
        num_bjets++;
      if (flavourLabel == 4)
        num_cjets++;
    } else {
      ANA_MSG_WARNING(
          "Truth jet is missing the decoration: HadronConeExclTruthLabelID.");
      num_bjets = num_cjets = -999;
    }
  }

  // decorate the EventInfo with the number of b- and c-jets
  dec_nBJets(*eventInfo) = num_bjets;
  dec_nCJets(*eventInfo) = num_cjets;

  return StatusCode::SUCCESS;
}

}  // namespace CP
