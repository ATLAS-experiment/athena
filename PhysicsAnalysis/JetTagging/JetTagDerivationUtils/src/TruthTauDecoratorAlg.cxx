/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthTauDecoratorAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstAccessor.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "TruthUtils/TruthClasses.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace {
  const SG::ConstAccessor<double> accPtVis("pt_vis");
  const SG::ConstAccessor<double> accEtaVis("eta_vis");
  const SG::ConstAccessor<double> accPhiVis("phi_vis");
  const SG::ConstAccessor<double> accMVis("m_vis");
  const SG::ConstAccessor<unsigned int> accType("classifierParticleType");
  const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> accOrig(
    "originalTruthParticle");

  constexpr float kNaN = std::numeric_limits<float>::quiet_NaN();

  // A ghost tau together with the TruthTaus entry holding its visible decay.
  struct Candidate {
    const xAOD::TruthParticle* tau{};
    const xAOD::TruthParticle* vis{};
  };
}

namespace ftag {

  // Per-event decoration writer for one tau slot.
  struct TruthTauDecoratorAlg::SlotDecor {
    using FHandle = SG::WriteDecorHandle<JC, float>;

    SG::WriteDecorHandle<JC, char> matched;
    FHandle deltaR, deltaPt, dEta, dPhi, pt, m, charge;

    SlotDecor(const Slot& s, const EventContext& ctx)
      : matched(s.matched, ctx), deltaR(s.deltaR, ctx), deltaPt(s.deltaPt, ctx),
        dEta(s.dEta, ctx), dPhi(s.dPhi, ctx), pt(s.pt, ctx), m(s.m, ctx),
        charge(s.charge, ctx) {}

    void set(const xAOD::Jet& jet, const Candidate* cand) {
      if (!cand) {
        matched(jet) = 0;
        deltaR(jet) = kNaN; deltaPt(jet) = kNaN;
        dEta(jet) = kNaN; dPhi(jet) = kNaN;
        pt(jet) = kNaN; m(jet) = kNaN; charge(jet) = kNaN;
        return;
      }
      TLorentzVector vis;
      vis.SetPtEtaPhiM(accPtVis(*cand->vis), accEtaVis(*cand->vis),
                       accPhiVis(*cand->vis), accMVis(*cand->vis));
      matched(jet) = 1;
      deltaR(jet) = jet.p4().DeltaR(vis);
      deltaPt(jet) = jet.pt() - vis.Pt();
      dEta(jet) = vis.Eta() - jet.eta();
      dPhi(jet) = xAOD::P4Helpers::deltaPhi(vis.Phi(), jet.phi());
      pt(jet) = cand->tau->pt();
      m(jet) = cand->tau->m();
      charge(jet) = cand->tau->charge();
    }
  };

  TruthTauDecoratorAlg::TruthTauDecoratorAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {
    auto declareSlot = [this](Slot& slot, const std::string& prefix) {
      declareProperty(prefix + "FloatsToCopy", slot.floats.toCopy);
      declareProperty(prefix + "DoublesToCopy", slot.doubles.toCopy);
      declareProperty(prefix + "IntsToCopy", slot.ints.toCopy);
      declareProperty(prefix + "UintsToCopy", slot.uints.toCopy);
      declareProperty(prefix + "UlongsToCopy", slot.ulongs.toCopy);
      declareProperty(prefix + "CharsToCopy", slot.chars.toCopy);
    };
    declareSlot(m_lead, "lead");
    declareSlot(m_sublead, "sublead");
  }

  StatusCode TruthTauDecoratorAlg::initializeSlot(Slot& slot,
                                                  const std::string& prefix) {
    for (auto* key : slot.computed()) ATH_CHECK(key->initialize());

    const std::vector<std::string> froms{m_truthTausKey.key()};
    const std::string& to = m_jetKey.key();
    ATH_CHECK(slot.floats.initialize(this, froms, to, prefix));
    ATH_CHECK(slot.doubles.initialize(this, froms, to, prefix));
    ATH_CHECK(slot.ints.initialize(this, froms, to, prefix));
    ATH_CHECK(slot.uints.initialize(this, froms, to, prefix));
    ATH_CHECK(slot.ulongs.initialize(this, froms, to, prefix));
    ATH_CHECK(slot.chars.initialize(this, froms, to, prefix));
    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::initialize() {
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_truthTausKey.initialize());
    ATH_CHECK(m_ghostTauKey.initialize());
    ATH_CHECK(initializeSlot(m_lead, "lead"));
    ATH_CHECK(initializeSlot(m_sublead, "sublead"));
    ATH_CHECK(m_nGhostTausKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<TPC> truthTaus(m_truthTausKey, ctx);
    if (!truthTaus.isValid()) {
      ATH_MSG_ERROR("Required TruthTaus container '" << m_truthTausKey.key()
                    << "' not found; cannot fill visible-tau decorations.");
      return StatusCode::FAILURE;
    }

    // Map each TruthTaus entry to the truth particle it was built from, to
    // match a ghost tau to its visible decay.
    std::unordered_map<const xAOD::TruthParticle*,
                       const xAOD::TruthParticle*> visByOrig;
    visByOrig.reserve(truthTaus->size());
    for (const xAOD::TruthParticle* truthTau : *truthTaus) {
      const auto& link = accOrig(*truthTau);
      if (link.isValid()) visByOrig[*link] = truthTau;
    }

    SG::ReadHandle<JC> jets(m_jetKey, ctx);
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container: " << m_jetKey.key());
      return StatusCode::FAILURE;
    }

    SG::ReadDecorHandle<JC, GhostLinks> ghostTaus(m_ghostTauKey, ctx);

    SlotDecor lead(m_lead, ctx);
    SlotDecor sublead(m_sublead, ctx);
    SG::WriteDecorHandle<JC, int> nGhostTausH(m_nGhostTausKey, ctx);

    std::vector<MatchedPair<MC>> leadPairs, subleadPairs;
    leadPairs.reserve(jets->size());
    subleadPairs.reserve(jets->size());

    const float minPt = m_minTruthTauPt.value();
    const bool isoOnly = m_requireIsolatedTau.value();

    for (const xAOD::Jet* jet : *jets) {

      std::vector<Candidate> cands;
      for (const ElementLink<MC>& link : ghostTaus(*jet)) {
        if (!link.isValid()) {
          ATH_MSG_ERROR("Invalid ghost-tau link in '" << m_ghostTauKey.key()
                        << "'; the truth particle was thinned away.");
          return StatusCode::FAILURE;
        }
        const auto* tau = dynamic_cast<const xAOD::TruthParticle*>(*link);
        if (!tau) {
          ATH_MSG_ERROR("Ghost-tau link in '" << m_ghostTauKey.key()
                        << "' does not point to an xAOD::TruthParticle.");
          return StatusCode::FAILURE;
        }
        auto it = visByOrig.find(tau);
        if (it == visByOrig.end()) {
          m_nTausNoVisMatch++;
          continue;
        }
        const xAOD::TruthParticle* vis = it->second;
        if (isoOnly && accType(*vis) != MCTruthPartClassifier::IsoTau) continue;
        if (accPtVis(*vis) < minPt) continue;
        cands.push_back({tau, vis});
      }
      const auto nRanked = std::min<std::size_t>(2, cands.size());
      std::ranges::partial_sort(cands, cands.begin() + nRanked,
        [](const Candidate& a, const Candidate& b) {
          return a.tau->pt() > b.tau->pt();
        });

      const Candidate* c0 = !cands.empty() ? &cands[0] : nullptr;
      const Candidate* c1 = cands.size() >= 2 ? &cands[1] : nullptr;

      lead.set(*jet, c0);
      sublead.set(*jet, c1);
      nGhostTausH(*jet) = static_cast<int>(cands.size());

      leadPairs.push_back({c0 ? c0->vis : nullptr, jet});
      subleadPairs.push_back({c1 ? c1->vis : nullptr, jet});
    }

    auto copySlot = [&ctx](const Slot& slot,
                           const std::vector<MatchedPair<MC>>& pairs) {
      slot.floats.copy(pairs, ctx);
      slot.doubles.copy(pairs, ctx);
      slot.ints.copy(pairs, ctx);
      slot.uints.copy(pairs, ctx);
      slot.ulongs.copy(pairs, ctx);
      slot.chars.copy(pairs, ctx);
    };
    copySlot(m_lead, leadPairs);
    copySlot(m_sublead, subleadPairs);

    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::finalize() {
    if (m_nTausNoVisMatch > 0) {
      ATH_MSG_WARNING(
        m_nTausNoVisMatch << " ghost taus had no matching TruthTaus entry and "
        << "were skipped. Check that '" << m_truthTausKey.key()
        << "' is complete.");
    }
    return StatusCode::SUCCESS;
  }

} // end namespace ftag
