/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TauSeedJetAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODJet/JetAuxContainer.h"

#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"

#include "TLorentzVector.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <set>
#include <utility>

namespace {
  // same scale as the jet reconstruction ghosts, see PseudoJetAlgorithm
  constexpr double ghostScale = 1e-40;
  const std::string truthLabelPrefix = "TruthLabel";
}

namespace ParticleJetTools {

TauSeedJetAlg::TauSeedJetAlg(const std::string& name, ISvcLocator* loc)
  : AthReentrantAlgorithm(name, loc) {}

StatusCode TauSeedJetAlg::initialize() {
  ATH_CHECK(m_tauKey.initialize());
  ATH_CHECK(m_constituentKey.initialize());
  ATH_CHECK(m_ghostKeys.initialize());
  ATH_CHECK(m_jetKey.initialize());
  if (!m_linkKey.empty()) m_linkKey = m_tauKey.key() + "." + m_linkKey.key();
  ATH_CHECK(m_linkKey.initialize(SG::AllowEmpty));
  if (!m_statusKey.empty()) m_statusKey = m_tauKey.key() + "." + m_statusKey.key();
  ATH_CHECK(m_statusKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_modifiers.retrieve());

  for (const auto& key : m_ghostKeys) {
    std::string label = key.key();
    if (label.starts_with(truthLabelPrefix)) label.erase(0, truthLabelPrefix.size());
    m_ghostLabels.push_back("Ghost" + label);
  }
  return StatusCode::SUCCESS;
}

StatusCode TauSeedJetAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<xAOD::TauJetContainer> taus(m_tauKey, ctx);
  SG::ReadHandle<xAOD::IParticleContainer> constituents(m_constituentKey, ctx);
  ATH_CHECK(taus.isValid());
  ATH_CHECK(constituents.isValid());

  std::vector<const xAOD::TruthParticleContainer*> ghostContainers;
  for (const auto& key : m_ghostKeys) {
    SG::ReadHandle<xAOD::TruthParticleContainer> ghosts(key, ctx);
    ATH_CHECK(ghosts.isValid());
    ghostContainers.push_back(ghosts.cptr());
  }

  auto jets = std::make_unique<xAOD::JetContainer>();
  auto jetsAux = std::make_unique<xAOD::JetAuxContainer>();
  jets->setStore(jetsAux.get());
  std::vector<int> statuses;
  const fastjet::JetDefinition jetDefinition(fastjet::antikt_algorithm, m_jetRadius);

  for (const xAOD::TauJet* tau : *taus) {
    TLorentzVector seedAxis;
    seedAxis.SetPtEtaPhiM(tau->ptJetSeed(), tau->etaJetSeed(), tau->phiJetSeed(), tau->mJetSeed());
    const fastjet::PseudoJet seedPseudoJet(seedAxis);

    xAOD::Jet* seedJet = jets->push_back(std::make_unique<xAOD::Jet>());
    seedJet->setAlgorithmType(xAOD::JetAlgorithmType::antikt_algorithm);
    seedJet->setSizeParameter(m_jetRadius);
    seedJet->setInputType(xAOD::JetInput::LCTopoOrigin);
    seedJet->setConstituentsSignalState(xAOD::CalibratedJetConstituent);

    std::set<std::size_t> tauClusters;
    TLorentzVector clusterSum;
    for (const auto& link : tau->clusterLinks()) {
      if (link.index() >= constituents->size()) {
        ATH_MSG_WARNING("Tau cluster index " << link.index() << " outside of " << m_constituentKey.key());
        continue;
      }
      if (!tauClusters.insert(link.index()).second) continue;
      const xAOD::IParticle* cluster = constituents->at(link.index());
      seedJet->addConstituent(cluster);
      clusterSum += cluster->p4();
    }
    const xAOD::JetFourMom_t seedP4 = tauClusters.empty()
      ? xAOD::JetFourMom_t(0, 0, 0, 0)
      : xAOD::JetFourMom_t(clusterSum.Pt(), clusterSum.Eta(), clusterSum.Phi(), clusterSum.M());
    seedJet->setJetP4(seedP4);
    seedJet->setJetP4(xAOD::JetConstitScaleMomentum, seedP4);

    std::vector<fastjet::PseudoJet> inputs;
    for (std::size_t index = 0; index < constituents->size(); ++index) {
      const xAOD::IParticle* constituent = constituents->at(index);
      if (constituent->e() <= 0) continue;
      if (!tauClusters.contains(index) && seedAxis.DeltaR(constituent->p4()) > m_neighbourDR) continue;
      fastjet::PseudoJet pseudoJet(constituent->p4());
      pseudoJet.set_user_index(index);
      inputs.push_back(pseudoJet);
    }

    // ghosts get negative user indices pointing into ghostRefs
    std::vector<std::pair<std::size_t, const xAOD::TruthParticle*>> ghostRefs;
    for (std::size_t iContainer = 0; iContainer < ghostContainers.size(); ++iContainer) {
      for (const xAOD::TruthParticle* particle : *ghostContainers.at(iContainer)) {
        if (particle->e() <= 0) continue;
        fastjet::PseudoJet pseudoJet(particle->p4());
        // in rapidity as in the clustering, partons can be far off shell
        if (seedPseudoJet.delta_R(pseudoJet) > m_ghostDR) continue;
        pseudoJet *= ghostScale;
        pseudoJet.set_user_index(-1 - static_cast<int>(ghostRefs.size()));
        ghostRefs.emplace_back(iContainer, particle);
        inputs.push_back(pseudoJet);
      }
    }

    fastjet::ClusterSequence clusterSequence(inputs, jetDefinition);
    const std::vector<fastjet::PseudoJet> clustered = clusterSequence.inclusive_jets();

    const fastjet::PseudoJet* tauJet = nullptr;
    double maxTauPt = 0;
    std::size_t nTauInJet = 0;
    std::size_t nOtherInJet = 0;
    for (const fastjet::PseudoJet& jet : clustered) {
      double tauPt = 0;
      std::size_t nTau = 0;
      std::size_t nOther = 0;
      for (const fastjet::PseudoJet& constituent : jet.constituents()) {
        if (constituent.user_index() < 0) continue;
        if (tauClusters.contains(constituent.user_index())) {
          tauPt += constituent.pt();
          ++nTau;
        } else {
          ++nOther;
        }
      }
      if (nTau > 0 && tauPt > maxTauPt) {
        maxTauPt = tauPt;
        tauJet = &jet;
        nTauInJet = nTau;
        nOtherInJet = nOther;
      }
    }

    std::vector<std::vector<const xAOD::TruthParticle*>> associated(ghostContainers.size());
    int status = -1;
    if (tauJet) {
      const std::size_t nTauInputs = std::count_if(
        inputs.begin(), inputs.end(), [&tauClusters](const fastjet::PseudoJet& pj) {
          return pj.user_index() >= 0 && tauClusters.contains(pj.user_index());
        });
      if (nTauInJet < nTauInputs) status = 2;
      else status = nOtherInJet > 0 ? 1 : 0;
      for (const fastjet::PseudoJet& constituent : tauJet->constituents()) {
        if (constituent.user_index() >= 0) continue;
        const auto& [iContainer, particle] = ghostRefs.at(-1 - constituent.user_index());
        associated.at(iContainer).push_back(particle);
      }
    }
    statuses.push_back(status);

    // same ghost content as in IParticleExtractor
    for (std::size_t iContainer = 0; iContainer < associated.size(); ++iContainer) {
      const std::string& label = m_ghostLabels.at(iContainer);
      float ptSum = 0;
      for (const xAOD::TruthParticle* particle : associated.at(iContainer)) ptSum += particle->pt();
      seedJet->setAssociatedObjects(label, associated.at(iContainer));
      seedJet->setAttribute<float>(label + "Pt", ptSum);
      seedJet->setAttribute<int>(label + "Count", associated.at(iContainer).size());
    }
  }

  SG::WriteHandle<xAOD::JetContainer> seedJets(m_jetKey, ctx);
  ATH_CHECK(seedJets.record(std::move(jets), std::move(jetsAux)));
  for (const ToolHandle<IJetModifier>& modifier : m_modifiers) {
    ATH_CHECK(modifier->modify(*seedJets));
  }

  std::optional<SG::WriteDecorHandle<xAOD::TauJetContainer, ElementLink<xAOD::JetContainer>>> linkHandle;
  if (!m_linkKey.empty()) linkHandle.emplace(m_linkKey, ctx);
  std::optional<SG::WriteDecorHandle<xAOD::TauJetContainer, int>> statusHandle;
  if (!m_statusKey.empty()) statusHandle.emplace(m_statusKey, ctx);
  for (std::size_t index = 0; index < taus->size(); ++index) {
    const xAOD::TauJet& tau = *taus->at(index);
    if (linkHandle) (*linkHandle)(tau) = ElementLink<xAOD::JetContainer>(*seedJets, index, ctx);
    if (statusHandle) (*statusHandle)(tau) = statuses.at(index);
  }

  return StatusCode::SUCCESS;
}

}
