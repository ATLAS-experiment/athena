/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/FtagLargeRJetTruthLabelTool.h"
#include "ParticleJetTools/ParticleJetLabelCommon.h"

#include "xAODJet/Jet.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"

#include "AsgTools/CurrentContext.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include <algorithm>
#include <cmath>

FtagLargeRJetTruthLabelTool::FtagLargeRJetTruthLabelTool(const std::string& name)
    : asg::AsgTool(name) {}

StatusCode FtagLargeRJetTruthLabelTool::initialize() {
  if (m_jetsKey.key().empty()) {
    ATH_MSG_ERROR("JetContainer property must be set");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_jetsKey.initialize());
  ATH_CHECK(m_decLabel.initialize());
  ATH_MSG_INFO("Initializing FtagLargeRJetTruthLabelTool: container=" << m_jetsKey.key()
               << "  decoration=" << m_decLabel.key());

  return StatusCode::SUCCESS;
}

StatusCode FtagLargeRJetTruthLabelTool::decorate(const xAOD::JetContainer& jets) const {

  // IJetDecorator::decorate() has no EventContext parameter; retrieve from Gaudi TLS.
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::WriteDecorHandle<xAOD::JetContainer, int> dec_label(m_decLabel, ctx);

  auto ptComp = [](const xAOD::TruthParticle* a, const xAOD::TruthParticle* b) {
    return a->pt() < b->pt();
  };
  //construct 'Ghost' strings before loop
  const std::string gHBosons{"GhostHBosons"};
  const std::string gTQuarks{"GhostTQuarksFinal"};
  const std::string gZBosons{"GhostZBosons"};
  const std::string gWBosons{"GhostWBosons"};
  const std::string gBHadrons{"GhostBHadronsFinal"};
  const std::string gCHadrons{"GhostCHadronsFinal"};
  const std::string gTaus{"GhostTausFinal"};
  const std::string gExtendedTruthLabel{"HadronGhostExtendedTruthLabelID"};
  //
  for (const xAOD::Jet* jet : jets) {
    std::vector<const xAOD::TruthParticle*> ghostH   = jet->getAssociatedObjects<xAOD::TruthParticle>(gHBosons);
    std::vector<const xAOD::TruthParticle*> ghostTop = jet->getAssociatedObjects<xAOD::TruthParticle>(gTQuarks);
    std::vector<const xAOD::TruthParticle*> ghostZ   = jet->getAssociatedObjects<xAOD::TruthParticle>(gZBosons);
    std::vector<const xAOD::TruthParticle*> ghostW   = jet->getAssociatedObjects<xAOD::TruthParticle>(gWBosons);
    std::vector<const xAOD::TruthParticle*> ghostB   = jet->getAssociatedObjects<xAOD::TruthParticle>(gBHadrons);
    std::vector<const xAOD::TruthParticle*> ghostC   = jet->getAssociatedObjects<xAOD::TruthParticle>(gCHadrons);
    std::vector<const xAOD::TruthParticle*> ghostTau = jet->getAssociatedObjects<xAOD::TruthParticle>(gTaus);

    // Origin priority: H > top > Z > W > QCD
    int originPdgId = 0;
    const xAOD::TruthParticle* parent = nullptr;
    if      (!ghostH.empty())   { originPdgId = 25; parent = *std::max_element(ghostH.begin(), ghostH.end(), ptComp);     }
    else if (!ghostTop.empty()) { originPdgId = 6;  parent = *std::max_element(ghostTop.begin(), ghostTop.end(), ptComp); }
    else if (!ghostZ.empty())   { originPdgId = 23; parent = *std::max_element(ghostZ.begin(), ghostZ.end(), ptComp);     }
    else if (!ghostW.empty())   { originPdgId = 24; parent = *std::max_element(ghostW.begin(), ghostW.end(), ptComp);     }

    // C-hadron count after removing B->C cascade descendants
    std::vector<const xAOD::TruthParticle*> deduplicatedC = std::move(ghostC);
    ParticleJetTools::childrenRemoved(ghostB, deduplicatedC);
    int nB   = static_cast<int>(ghostB.size());
    int nC   = static_cast<int>(deduplicatedC.size());
    int nTau = static_cast<int>(ghostTau.size());

    // Extended truth label encodes tau-pair decay mode:
    // 1515 = had-had, 151511 = had + tau->e, 151513 = had + tau->mu
    static const SG::ConstAccessor<int> accExtLabel(gExtendedTruthLabel);
    int extLabel = accExtLabel.isAvailable(*jet) ? accExtLabel(*jet) : 0;

    FtagLargeRLabel::TypeEnum label = FtagLargeRLabel::UNKNOWN;
    if (originPdgId != 0 && parent && parent->hasDecayVtx())
      label = classifyDecay(parent, originPdgId, nB, nC, nTau, extLabel);
    else
      label = classifyQCD(nB, nC);

    dec_label(*jet) = static_cast<int>(label);
  }

  return StatusCode::SUCCESS;
}

// ===== Decay + containment classification =====

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyDecay(
    const xAOD::TruthParticle* parent, int originPdgId, int nB, int nC, int nTau, int extLabel) const {

  std::vector<const xAOD::TruthParticle*> children = getDecayProducts(parent);
  if (children.empty()) return FtagLargeRLabel::UNKNOWN;

  std::set<int> ids;
  for (const auto* c : children) ids.insert(std::abs(c->pdgId()));

  switch (originPdgId) {
    case 25: return classifyHiggsDecay(ids, children, nB, nC, nTau, extLabel);
    case 6:  return classifyTopDecay(children, nB, nC);
    case 24: return classifyWDecay(children, nC);
    case 23: return classifyZDecay(ids, nB, nC, nTau, extLabel);
    default: return FtagLargeRLabel::UNKNOWN;
  }
}

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyHiggsDecay(
    const std::set<int>& ids, const std::vector<const xAOD::TruthParticle*>& children,
    int nB, int nC, int nTau, int extLabel) const {
  if (ids.count(5)) {
    if (nB >= 2) return FtagLargeRLabel::Hbb;
    if (nB == 1) return FtagLargeRLabel::Hb;
    return FtagLargeRLabel::Hother;
  }
  if (ids.count(4)) {
    if (nC >= 2) return FtagLargeRLabel::Hcc;
    if (nC == 1) return FtagLargeRLabel::Hc;
    return FtagLargeRLabel::Hother;
  }
  if (ids.count(15)) {
    if (nTau == 0) return FtagLargeRLabel::Hother;
    if (nTau == 1) return FtagLargeRLabel::Htau;
    switch (extLabel) {
      case 1515:   return FtagLargeRLabel::Htautau;
      case 151511: return FtagLargeRLabel::HtautauEl;
      case 151513: return FtagLargeRLabel::HtautauMu;
      default:     return FtagLargeRLabel::Hother;
    }
  }
  // H -> VV: navigate each boson's decay to count hadronic vs leptonic legs.
  // Bosons with no decay vertex are skipped, which demotes the event to Hother.
  if (ids.count(24)) {
    int nHadW = 0, nLepW = 0;
    for (const auto* child : children) {
      if (std::abs(child->pdgId()) != 24 || !child->hasDecayVtx()) continue;
      bool lep = false;
      for (const auto* wc : getDecayProducts(child)) {
        int id = std::abs(wc->pdgId());
        if (id == 11 || id == 13 || id == 15) { lep = true; break; }
      }
      if (lep) ++nLepW; else ++nHadW;
    }
    if (nHadW == 2)              return FtagLargeRLabel::HWWhad;
    if (nHadW == 1 && nLepW == 1) return FtagLargeRLabel::HWWlep;
    return FtagLargeRLabel::Hother;
  }
  if (ids.count(23)) {
    int nHadZ = 0;
    for (const auto* child : children) {
      if (std::abs(child->pdgId()) != 23 || !child->hasDecayVtx()) continue;
      for (const auto* zc : getDecayProducts(child)) {
        int id = std::abs(zc->pdgId());
        if (id >= 1 && id <= 6) { ++nHadZ; break; }
      }
    }
    if (nHadZ == 2) return FtagLargeRLabel::HZZhad;
    return FtagLargeRLabel::Hother;
  }
  return FtagLargeRLabel::Hother;
}

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyTopDecay(
    const std::vector<const xAOD::TruthParticle*>& children, int nB, int nC) const {
  for (const auto* child : children) {
    if (std::abs(child->pdgId()) != 24) continue;
    if (!child->hasDecayVtx()) return FtagLargeRLabel::Wother;

    std::vector<const xAOD::TruthParticle*> wProducts = getDecayProducts(child);
    if (wProducts.empty()) return FtagLargeRLabel::Wother;

    bool hasCharm = false, hasQuark = false;
    for (const auto* wc : wProducts) {
      int id = std::abs(wc->pdgId());
      if (id >= 1 && id <= 5) hasQuark = true;
      if (id == 4)            hasCharm = true;
    }

    if (hasCharm) {
      if (nB >= 1 && nC >= 1) return FtagLargeRLabel::TopBcs;
      if (nB >= 1)            return FtagLargeRLabel::TopBx;
      if (nC >= 1)            return FtagLargeRLabel::Wcs;
      return FtagLargeRLabel::Wother;
    }
    if (hasQuark) {
      if (nB >= 1) return FtagLargeRLabel::TopBqq;
      return FtagLargeRLabel::Wqq;
    }
    if (nB >= 1) return FtagLargeRLabel::TopBlv;
    return FtagLargeRLabel::Wother;
  }
  return FtagLargeRLabel::Wother;
}

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyWDecay(
    const std::vector<const xAOD::TruthParticle*>& children, int nC) const {
  bool hasCharm = false, hasQuark = false;
  for (const auto* child : children) {
    int id = std::abs(child->pdgId());
    if (id >= 1 && id <= 5) hasQuark = true;
    if (id == 4)            hasCharm = true;
  }
  if (hasCharm && nC >= 1) return FtagLargeRLabel::Wcs;
  if (hasQuark) return FtagLargeRLabel::Wqq;
  return FtagLargeRLabel::Wother;
}

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyZDecay(
    const std::set<int>& ids, int nB, int nC, int nTau, int extLabel) const {
  if (ids.count(5)) {
    if (nB >= 2) return FtagLargeRLabel::Zbb;
    if (nB == 1) return FtagLargeRLabel::Zb;
    return FtagLargeRLabel::Zother;
  }
  if (ids.count(4)) {
    if (nC >= 2) return FtagLargeRLabel::Zcc;
    if (nC == 1) return FtagLargeRLabel::Zc;
    return FtagLargeRLabel::Zother;
  }
  if (ids.count(3))  return FtagLargeRLabel::Zss;
  if (ids.count(15)) {
    if (nTau == 0) return FtagLargeRLabel::Zother;
    if (nTau == 1) return FtagLargeRLabel::Ztau;
    switch (extLabel) {
      case 1515:   return FtagLargeRLabel::Ztautau;
      case 151511: return FtagLargeRLabel::ZtautauEl;
      case 151513: return FtagLargeRLabel::ZtautauMu;
      default:     return FtagLargeRLabel::Zother;
    }
  }
  if (ids.count(1) || ids.count(2)) return FtagLargeRLabel::Zqq;
  return FtagLargeRLabel::Zother;
}

FtagLargeRLabel::TypeEnum FtagLargeRJetTruthLabelTool::classifyQCD(
    int nB, int nC) const {
  if (nB >= 2)             return FtagLargeRLabel::QCDbb;
  if (nB == 1 && nC >= 1)  return FtagLargeRLabel::QCDbc;
  if (nB == 0 && nC >= 2)  return FtagLargeRLabel::QCDcc;
  if (nB == 1)             return FtagLargeRLabel::QCDbq;
  if (nC == 1)             return FtagLargeRLabel::QCDcq;
  return FtagLargeRLabel::QCDqq;
}


// ===== Helpers =====

std::vector<const xAOD::TruthParticle*>
FtagLargeRJetTruthLabelTool::getDecayProducts(
    const xAOD::TruthParticle* parent) const {
  std::set<const xAOD::TruthParticle*> visited;
  return getDecayProducts(parent, visited);
}

std::vector<const xAOD::TruthParticle*>
FtagLargeRJetTruthLabelTool::getDecayProducts(
    const xAOD::TruthParticle* parent,
    std::set<const xAOD::TruthParticle*>& visited) const {
  std::vector<const xAOD::TruthParticle*> products;
  if (!parent || !parent->hasDecayVtx()) return products;
  if (!visited.insert(parent).second) return products;  // cycle detected
  const xAOD::TruthVertex* vtx = parent->decayVtx();
  if (!vtx) return products;

  for (size_t i = 0; i < vtx->nOutgoingParticles(); ++i) {
    const xAOD::TruthParticle* child = vtx->outgoingParticle(i);
    if (!child || child == parent) continue;
    int childAbsId = std::abs(child->pdgId());

    // Follow through self-copies (same pdgId -> generator intermediate)
    if (child->pdgId() == parent->pdgId() && child->hasDecayVtx()) {
      std::vector<const xAOD::TruthParticle*> gc = getDecayProducts(child, visited);
      products.insert(products.end(), gc.begin(), gc.end());
      continue;
    }
    // Skip FSR photons and gluons
    if (childAbsId == 22 || childAbsId == 21) continue;

    products.push_back(child);
  }
  return products;
}
