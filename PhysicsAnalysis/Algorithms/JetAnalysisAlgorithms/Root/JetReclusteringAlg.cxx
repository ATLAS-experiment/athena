/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tomas Dado

#include "JetAnalysisAlgorithms/JetReclusteringAlg.h"
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"

namespace CP {
  using ROOT::Math::PtEtaPhiEVector;
  using ROOT::Math::VectorUtil::DeltaR;

  StatusCode JetReclusteringAlg::initialize() {

    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle, SG::AllowEmpty));

    ANA_CHECK(m_smallRjetIndicesDecor.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));
    ANA_CHECK(m_rcEnergyDecor.initialize(m_systematicsList, m_outHandle, SG::AllowEmpty));

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

  StatusCode JetReclusteringAlg::execute() {

    for (const auto &sys : m_systematicsList.systematicsVector()) {

      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys));

      auto rcJetAuxContainer = std::make_unique<xAOD::JetAuxContainer>();
      auto rcJetContainer = std::make_unique<xAOD::JetContainer>();
      rcJetContainer->setStore(rcJetAuxContainer.get());

      // only take selected jets
      std::vector<fastjet::PseudoJet> clusters;
      std::vector<const xAOD::Jet*> selectedJets;
      for (const xAOD::Jet *jet : *jets) {
        if (m_jetSelection.getBool(*jet, sys)) {
          fastjet::PseudoJet pseudoJet(jet->px(), jet->py(), jet->pz(), jet->e());
          clusters.emplace_back(std::move(pseudoJet));
          selectedJets.push_back(jet);
        }
      }

      // no jets (clusters) - nothing to do
      if (clusters.empty()) {
        ANA_CHECK(m_outHandle.record(std::move(rcJetContainer), std::move(rcJetAuxContainer), sys));
        continue;
      }

      fastjet::ClusterSequence cluster_sequence(clusters, *m_fastjetClustering);
      std::vector<fastjet::PseudoJet> rcJets = cluster_sequence.inclusive_jets(0.0);

      for (const auto& irc : rcJets) {
        xAOD::Jet *rcJet = new xAOD::Jet();

        const xAOD::JetFourMom_t p4(irc.pt(), irc.eta(), irc.phi_std(), irc.m());
        rcJetContainer->push_back(rcJet);

        rcJetContainer->back()->setJetP4(p4);

        const auto constituents = irc.constituents();

        const std::vector<int> indices = this->matchRCjets(selectedJets, constituents);

        if (indices.size() != constituents.size()) {
          ANA_MSG_ERROR("Size of the contituents does not match the size of the indices");
          ANA_CHECK(m_outHandle.record(std::move(rcJetContainer), std::move(rcJetAuxContainer), sys));
          return StatusCode::FAILURE;
        }

        m_smallRjetIndicesDecor.set(*(rcJetContainer->back()), indices, sys);

        // Write energy to the RC jet, since default 4-vector form is (pt, eta, phi, m)
        m_rcEnergyDecor.set(*(rcJetContainer->back()), irc.e(), sys);
      }

      ANA_CHECK(m_outHandle.record(std::move(rcJetContainer), std::move(rcJetAuxContainer), sys));

    }
    return StatusCode::SUCCESS;
  }

  std::vector<int> JetReclusteringAlg::matchRCjets(const std::vector<const xAOD::Jet*>& smallJets, const std::vector<fastjet::PseudoJet>& constituents) const {
    std::vector<int> indices;
    for (const auto& iconst : constituents) {
      PtEtaPhiEVector iconstP4(iconst.pt(), iconst.eta(), iconst.phi_std(), iconst.e());
      for (std::size_t i = 0; i < smallJets.size(); ++i) {
        const float dR = DeltaR(smallJets.at(i)->p4(), iconstP4);
        if (dR > 0.1) continue; // no match

        indices.emplace_back(i);
      }
    }
    return indices;
  }

} // namespace
