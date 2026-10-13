/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tomas Dado

#include "JetAnalysisAlgorithms/JetReclusteringAlg.h"

namespace CP {

  StatusCode JetReclusteringAlg::initialize() {

    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle, SG::AllowEmpty));

    ANA_CHECK(m_smallRjetIndicesDecor.initialize(m_systematicsList, m_outHandle));
    ANA_CHECK(m_rcEnergyDecor.initialize(m_systematicsList, m_outHandle));

    ANA_CHECK(m_outHandle.initialize(m_systematicsList));

    ANA_CHECK(m_systematicsList.initialize());

    // reclustering properties
    if (m_reclusteredJetsRadius < 0) {
      ANA_MSG_ERROR("reclusteredJetsRadius is negative, please fix");
      return StatusCode::FAILURE;
    }

    fastjet::JetAlgorithm algorithm;
    static const std::unordered_map<std::string, fastjet::JetAlgorithm> algorithmMap = {
      {"AntiKt", fastjet::antikt_algorithm},
      {"Kt", fastjet::kt_algorithm},
      {"CamKt", fastjet::cambridge_algorithm}
    };
    auto it = algorithmMap.find(m_clusteringAlgorithm.value());
    if (it != algorithmMap.end()) {
      algorithm = it->second;
    } else {
      ATH_MSG_ERROR("Jet reclustering algorithm " << m_clusteringAlgorithm.value()
                    << " is not supported by CP::JetReclusteringAlg! Choose amongst: AntiKt, Kt, CamKt.");
      return StatusCode::FAILURE;
    }

    m_fastjetClustering = std::make_unique<fastjet::JetDefinition>(algorithm, m_reclusteredJetsRadius, fastjet::E_scheme, fastjet::Best);

    return StatusCode::SUCCESS;
  }

  StatusCode JetReclusteringAlg::execute(const EventContext& ctx) {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys, ctx));

      auto rcJetAuxContainer = std::make_unique<xAOD::JetAuxContainer>();
      auto rcJetContainer = std::make_unique<xAOD::JetContainer>();
      rcJetContainer->setStore(rcJetAuxContainer.get());

      // only take selected jets, remembering their index in the input
      // container via the fastjet user index
      std::vector<fastjet::PseudoJet> clusters;
      clusters.reserve(jets->size());
      for (std::size_t i = 0; i < jets->size(); ++i) {
        const xAOD::Jet *jet = (*jets)[i];
        if (m_jetSelection.getBool(*jet, sys)) {
          fastjet::PseudoJet pseudoJet(jet->px(), jet->py(), jet->pz(), jet->e());
          pseudoJet.set_user_index(static_cast<int>(i));
          clusters.emplace_back(std::move(pseudoJet));
        }
      }

      // no jets (clusters) - nothing to do
      if (clusters.empty()) {
        ANA_CHECK(m_outHandle.record(std::move(rcJetContainer), std::move(rcJetAuxContainer), sys, ctx));
        continue;
      }

      fastjet::ClusterSequence cluster_sequence(clusters, *m_fastjetClustering);
      std::vector<fastjet::PseudoJet> rcJets = cluster_sequence.inclusive_jets(0.0);

      for (const auto& irc : rcJets) {
        xAOD::Jet *rcJet = rcJetContainer->push_back(std::make_unique<xAOD::Jet>());

        const xAOD::JetFourMom_t p4(irc.pt(), irc.eta(), irc.phi_std(), irc.m());
        rcJet->setJetP4(p4);

        const auto constituents = irc.constituents();

        std::vector<int> indices;
        indices.reserve(constituents.size());
        for (const auto& iconst : constituents) {
          indices.push_back(iconst.user_index());
        }

        m_smallRjetIndicesDecor.set(*rcJet, indices, sys);

        // Write energy to the RC jet, since default 4-vector form is (pt, eta, phi, m)
        m_rcEnergyDecor.set(*rcJet, irc.e(), sys);
      }

      ANA_CHECK(m_outHandle.record(std::move(rcJetContainer), std::move(rcJetAuxContainer), sys, ctx));

    }
    return StatusCode::SUCCESS;
  }

} // namespace
