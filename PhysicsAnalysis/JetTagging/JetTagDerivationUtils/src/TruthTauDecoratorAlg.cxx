/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthTauDecoratorAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstAccessor.h"
#include "FourMomUtils/xAODP4Helpers.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace ftag {

  // Per-event decoration writer for one tau slot.
  struct TruthTauDecoratorAlg::TauDecor {
    using FHandle = SG::WriteDecorHandle<xAOD::JetContainer, float>;
    using IHandle = SG::WriteDecorHandle<xAOD::JetContainer, int>;

    // Handle-side KinKeys, with the fill written once so a mistake shows up in
    // all four variables rather than one.
    struct KinDecor {
      FHandle pt, deta, dphi, m;

      KinDecor(const KinKeys& k, const EventContext& ctx)
        : pt(k.pt, ctx), deta(k.deta, ctx), dphi(k.dphi, ctx), m(k.m, ctx) {}

      // deta/dphi are the tau relative to the jet axis.
      void fill(const xAOD::Jet& jet, float srcPt, float srcEta, float srcPhi,
                float srcM, float jetEta, float jetPhi) {
        pt(jet)   = srcPt;
        m(jet)    = srcM;
        deta(jet) = srcEta - jetEta;
        dphi(jet) = xAOD::P4Helpers::deltaPhi(srcPhi, jetPhi);
      }
      void clear(const xAOD::Jet& jet) {
        const float kNaN = std::numeric_limits<float>::quiet_NaN();
        pt(jet) = 0.0f; m(jet) = 0.0f; deta(jet) = kNaN; dphi(jet) = kNaN;
      }
    };

    KinDecor total, vis;
    IHandle numCharged, charge, isHadronic;

    TauDecor(const TauKeys& k, const EventContext& ctx)
      : total(k.total, ctx), vis(k.vis, ctx),
        numCharged(k.numCharged, ctx), charge(k.charge, ctx),
        isHadronic(k.isHadronic, ctx) {}

    void set(const xAOD::Jet& jet, const xAOD::TruthParticle* tau,
             const xAOD::TruthParticle* truthTau, float jetEta, float jetPhi) {
      if (tau) {
        total.fill(jet, tau->pt(), tau->eta(), tau->phi(), tau->m(),
                   jetEta, jetPhi);
        charge(jet) = static_cast<int>(std::lround(tau->charge()));
      } else {
        total.clear(jet);
        charge(jet) = 0;
      }
      if (truthTau) {
        static const SG::ConstAccessor<double> accPtVis("pt_vis");
        static const SG::ConstAccessor<double> accEtaVis("eta_vis");
        static const SG::ConstAccessor<double> accPhiVis("phi_vis");
        static const SG::ConstAccessor<double> accMVis("m_vis");
        static const SG::ConstAccessor<std::size_t> accNumCharged("numCharged");
        static const SG::ConstAccessor<char> accIsHadronic("IsHadronicTau");
        vis.fill(jet, accPtVis(*truthTau), accEtaVis(*truthTau),
                 accPhiVis(*truthTau), accMVis(*truthTau), jetEta, jetPhi);
        numCharged(jet) = static_cast<int>(accNumCharged(*truthTau));
        isHadronic(jet) = static_cast<int>(accIsHadronic(*truthTau) != 0);
      } else {
        vis.clear(jet);
        numCharged(jet) = -1;
        isHadronic(jet) = -1;
      }
    }
  };

  StatusCode TruthTauDecoratorAlg::initialize() {
    ATH_CHECK(m_jetKey.initialize());
    ATH_CHECK(m_truthTausKey.initialize());
    ATH_CHECK(m_ghostTauKey.initialize());
    for (auto* k : m_lead.all())    ATH_CHECK(k->initialize());
    for (auto* k : m_sublead.all()) ATH_CHECK(k->initialize());
    ATH_CHECK(m_nGhostTausKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::execute(const EventContext& ctx) const {

    // Required input.
    SG::ReadHandle<xAOD::TruthParticleContainer> truthTaus(m_truthTausKey, ctx);
    if (!truthTaus.isValid()) {
      ATH_MSG_ERROR("Required TruthTaus container '" << m_truthTausKey.key()
                    << "' not found; cannot fill visible-tau decorations.");
      return StatusCode::FAILURE;
    }

    // Map each TruthTaus entry to the truth particle it was built from, to
    // match a ghost tau to its visible momentum.
    static const SG::ConstAccessor<
      ElementLink<xAOD::TruthParticleContainer>> accOrig("originalTruthParticle");
    std::unordered_map<const xAOD::TruthParticle*,
                       const xAOD::TruthParticle*> visByOrig;
    visByOrig.reserve(truthTaus->size());
    for (const xAOD::TruthParticle* truthTau : *truthTaus) {
      const auto& link = accOrig(*truthTau);
      if (link.isValid()) visByOrig[*link] = truthTau;
    }

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container: " << m_jetKey.key());
      return StatusCode::FAILURE;
    }

    SG::ReadDecorHandle<xAOD::JetContainer, GhostLinks> ghostTaus(
      m_ghostTauKey, ctx);

    TauDecor lead(m_lead, ctx);
    TauDecor sublead(m_sublead, ctx);
    SG::WriteDecorHandle<xAOD::JetContainer, int> nGhostTausH(m_nGhostTausKey, ctx);

    for (const xAOD::Jet* jet : *jets) {

      const GhostLinks& links = ghostTaus(*jet);
      std::vector<const xAOD::TruthParticle*> taus;
      taus.reserve(links.size());
      for (const ElementLink<xAOD::IParticleContainer>& link : links) {
        if (!link.isValid()) {
          ATH_MSG_ERROR("Invalid ghost-tau link in '" << m_ghostTauKey.key()
                        << "'; the truth particle was thinned away.");
          return StatusCode::FAILURE;
        }
        const auto* truthPart = dynamic_cast<const xAOD::TruthParticle*>(*link);
        if (!truthPart) {
          ATH_MSG_ERROR("Ghost-tau link in '" << m_ghostTauKey.key()
                        << "' does not point to an xAOD::TruthParticle.");
          return StatusCode::FAILURE;
        }
        taus.push_back(truthPart);
      }
      std::sort(taus.begin(), taus.end(),
        [](const xAOD::TruthParticle* a, const xAOD::TruthParticle* b) {
          return a->pt() > b->pt();
        });

      // Match a ghost tau to its TruthTaus entry.
      auto visMatch = [&](const xAOD::TruthParticle* tau)
          -> const xAOD::TruthParticle* {
        if (!tau) return nullptr;
        auto it = visByOrig.find(tau);
        if (it != visByOrig.end()) return it->second;
        m_nTausNoVisMatch++;
        return nullptr;
      };

      const xAOD::TruthParticle* leadTau = !taus.empty() ? taus[0] : nullptr;
      const xAOD::TruthParticle* subTau  = taus.size() >= 2 ? taus[1] : nullptr;
      const float jetEta = jet->eta();
      const float jetPhi = jet->phi();

      lead.set(*jet, leadTau, visMatch(leadTau), jetEta, jetPhi);
      sublead.set(*jet, subTau, visMatch(subTau), jetEta, jetPhi);
      nGhostTausH(*jet) = static_cast<int>(taus.size());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TruthTauDecoratorAlg::finalize() {
    if (m_nTausNoVisMatch > 0) {
      ATH_MSG_WARNING(
        m_nTausNoVisMatch << " lead/sublead ghost taus had no matching "
        << "TruthTaus entry; their visible variables (truthtau_*_*_vis, "
        << "numCharged) are empty. Check that '" << m_truthTausKey.key()
        << "' is complete.");
    }
    return StatusCode::SUCCESS;
  }

} // end namespace ftag
