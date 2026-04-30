/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Ported from TDD's JetLeptonDecayLabelDecorator + TruthTools.
// Computes LeptonDecayLabel and TauDecayLabel from ghost-associated
// b/c hadron decay chains, without needing TruthParticles in the DAOD.

#include "FlavorTagDiscriminants/JetLeptonDecayLabelAlg.h"

#include "StoreGate/WriteDecorHandle.h"

#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <limits>


namespace {

  // --- helper structs / functions ported from TDD ---

  struct LeptonCounter {
    int el_fromHad = 0;
    int mu_fromHad = 0;
    int tau_fromHad = 0;
    int el_fromTau = 0;
    int mu_fromTau = 0;
  };

  // DFS walk of decay products, skipping simulation particles and
  // stopping at c/b hadron boundaries.  (TruthTools::getAllChildren)
  // Returns false if a cycle is detected.
  bool getAllChildren(const xAOD::TruthParticle* particle,
                      std::vector<const xAOD::TruthParticle*>& out,
                      std::unordered_set<const xAOD::TruthParticle*>& visited) {
    if (!particle->hasDecayVtx()) return true;
    if (!visited.insert(particle).second) return false;
    const xAOD::TruthVertex* vtx = particle->decayVtx();
    for (unsigned i = 0; i < vtx->nOutgoingParticles(); ++i) {
      const xAOD::TruthParticle* child = vtx->outgoingParticle(i);
      if (!child) continue;
      if (child->isSimulationParticle()) continue;
      if ((child->status() == 1 || child->status() == 2)
          && !child->isCharmHadron() && !child->isBottomHadron()) {
        out.push_back(child);
      }
      if (!getAllChildren(child, out, visited)) return false;
    }
    return true;
  }

  // Sort hadron indices by deltaR to jet.  (TruthTools::getDRSortedIndices)
  std::vector<int> getDRSortedIndices(
      const std::vector<const xAOD::TruthParticle*>& hadrons,
      const xAOD::Jet& jet) {
    std::vector<float> dRs;
    dRs.reserve(hadrons.size());
    for (const auto* h : hadrons) {
      dRs.push_back(jet.p4().DeltaR(h->p4()));
    }
    std::vector<int> idx(dRs.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(),
              [&](int a, int b) { return dRs[a] < dRs[b]; });
    return idx;
  }

  // Count electrons, muons, taus (and tau sub-products) among hadron
  // decay children.  (JetLeptonDecayLabelDecorator::countLeptons)
  LeptonCounter countLeptons(
      const std::vector<const xAOD::TruthParticle*>& hadrons,
      const xAOD::Jet& jet) {
    LeptonCounter lc;
    if (hadrons.empty()) return lc;
    std::vector<int> indices = getDRSortedIndices(hadrons, jet);
    for (unsigned iHad = 0; iHad < hadrons.size(); ++iHad) {
      const xAOD::TruthParticle* had = hadrons.at(indices[iHad]);
      std::vector<const xAOD::TruthParticle*> children;
      std::unordered_set<const xAOD::TruthParticle*> visited;
      if (!getAllChildren(had, children, visited)) {
        // Cycle detected in truth record — skip this hadron to avoid
        // infinite recursion.  Should not happen in well-formed MC.
        continue;
      }
      for (unsigned i = 0; i < children.size(); ++i) {
        int absId = std::abs(children[i]->pdgId());
        if (absId == 11) lc.el_fromHad += 1;
        if (absId == 13) lc.mu_fromHad += 1;
        if (absId == 15) {
          lc.tau_fromHad += 1;
          if (children.size() > i + 3) {
            for (unsigned j = 1; j <= 3; ++j) {
              int tauChildId = std::abs(children[i + j]->pdgId());
              if (tauChildId == 11) lc.el_fromTau += 1;
              if (tauChildId == 13) lc.mu_fromTau += 1;
            }
          }
        }
      }
    }
    return lc;
  }

  // Encode lepton species into a decimal label.
  // (JetLeptonDecayLabelDecorator::getDecayLabel)
  int getDecayLabel(const std::vector<int>& leps) {
    //max int value is 2147483647
    //log10(2147483647) ~ 9.3
    //To avoid overflow of 'factor' we limit the leps vector size
    if (leps.size()>9) throw std::overflow_error("getDecayLabel: too many leptons");
    int factor = 1;
    int label = 0;
    for (int pdg : leps) {
      if (pdg == 15) label += factor * 3;
      if (pdg == 13) label += factor * 2;
      if (pdg == 11) label += factor * 1;
      factor *= 10; //possible wrap around to negative value if factor is too big
    }
    return label;
  }

  // Encode tau sub-decay leptons.
  // (JetLeptonDecayLabelDecorator::getTauLabel)
  int getTauLabel(const std::vector<int>& tauLeps,
                  const std::vector<int>& leps) {
    if (std::find(leps.begin(), leps.end(), 15) == leps.end()) {
      return -999;
    }
    int maxFactor = std::numeric_limits<int>::max() / 10;
    int factor = 1;
    int label = 0;
    for (int pdg : tauLeps) {
      if (pdg == 11) { 
        label += factor;
        if (factor>maxFactor) throw std::overflow_error("getTauLabel:(pdg = 11) factor overflow");
        factor *= 10; 
      }
      if (pdg == 13) { 
        label += 2 * factor;
        if (factor>maxFactor) throw std::overflow_error("getTauLabel:(pdg = 13) factor overflow"); 
        factor *= 10; 
      }
    }
    return label;
  }

}  // anonymous namespace


namespace FlavorTagDiscriminants {

  JetLeptonDecayLabelAlg::JetLeptonDecayLabelAlg(
      const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode JetLeptonDecayLabelAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << "...");
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_dec_leptonDecayLabel.initialize());
    ATH_CHECK(m_dec_tauDecayLabel.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode JetLeptonDecayLabelAlg::execute(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetContainerKey, ctx);
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Could not retrieve jet container "
                    << m_jetContainerKey.key());
      return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::JetContainer, int> decDecay(
        m_dec_leptonDecayLabel, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, int> decTau(
        m_dec_tauDecayLabel, ctx);

    for (const xAOD::Jet* jet : *jets) {

      std::vector<const xAOD::TruthParticle*> bHadrons;
      jet->getAssociatedObjects<xAOD::TruthParticle>(
          "ConeExclBHadronsFinal", bHadrons);

      std::vector<const xAOD::TruthParticle*> cHadrons;
      jet->getAssociatedObjects<xAOD::TruthParticle>(
          "ConeExclCHadronsFinal", cHadrons);

      decDecay(*jet) = -999;
      decTau(*jet) = -999;

      if (bHadrons.empty() && cHadrons.empty()) continue;

      LeptonCounter allLep = countLeptons(bHadrons, *jet);
      LeptonCounter cLep   = countLeptons(cHadrons, *jet);

      int el_fromB  = allLep.el_fromHad  - cLep.el_fromHad;
      int mu_fromB  = allLep.mu_fromHad  - cLep.mu_fromHad;
      int tau_fromB = allLep.tau_fromHad - cLep.tau_fromHad;
      int tauel_fromB = allLep.el_fromTau - cLep.el_fromTau;
      int taumu_fromB = allLep.mu_fromTau - cLep.mu_fromTau;

      int el_fromC  = cLep.el_fromHad;
      int mu_fromC  = cLep.mu_fromHad;
      int tau_fromC = cLep.tau_fromHad;
      int tauel_fromC = cLep.el_fromTau;
      int taumu_fromC = cLep.mu_fromTau;

      std::vector<int> leps_in_decays;
      if (tau_fromB > 0) leps_in_decays.push_back(15);
      if (tau_fromC > 0) leps_in_decays.push_back(15);
      if (mu_fromB > 0)  leps_in_decays.push_back(13);
      if (mu_fromC > 0)  leps_in_decays.push_back(13);
      if (el_fromB > 0)  leps_in_decays.push_back(11);
      if (el_fromC > 0)  leps_in_decays.push_back(11);

      std::vector<int> tauleps_in_decays;
      tauleps_in_decays.push_back(tauel_fromB > 0 ? 11 : 0);
      tauleps_in_decays.push_back(tauel_fromC > 0 ? 11 : 0);
      tauleps_in_decays.push_back(taumu_fromB > 0 ? 13 : 0);
      tauleps_in_decays.push_back(taumu_fromC > 0 ? 13 : 0);

      decDecay(*jet) = getDecayLabel(leps_in_decays);
      decTau(*jet) = getTauLabel(tauleps_in_decays, leps_in_decays);
    }

    return StatusCode::SUCCESS;
  }

}  // namespace FlavorTagDiscriminants
