/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthMuonBranch.h"

#include <vector>

namespace L1Muon {

TruthMuonBranch::TruthMuonBranch(MuonVal::MuonTesterTree& tree,
                                 const ITruthMuonReferenceTool* referenceTool,
                                 const MSG::Level msgLevel)
    : MuonVal::MuonTesterBranch(tree, " truthMuon module"),
      m_referenceTool{referenceTool},
      m_truthDependencyKey{referenceTool != nullptr ? referenceTool->inputKey()
                                                    : std::string{}} {
  setLevel(msgLevel);
}

bool TruthMuonBranch::init() {
  if (m_referenceTool == nullptr || m_truthDependencyKey.empty()) {
    ATH_MSG_ERROR("No valid truth-muon reference tool was provided");
    return false;
  }
  return declare_dependency(m_truthDependencyKey);
}

bool TruthMuonBranch::fill(const EventContext& ctx) {
  std::vector<TruthMuonReference> references{};
  if (m_referenceTool->selectedMuons(ctx, references).isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve selected truth-muon references");
    return false;
  }

  for (const TruthMuonReference& reference : references) {
    m_pdgId.push_back(reference.pdgId);
    m_barcode.push_back(reference.barcode);
    m_pt.push_back(static_cast<float>(reference.ptMeV * 1.e-3));
    m_eta.push_back(static_cast<float>(reference.eta));
    m_phi.push_back(static_cast<float>(reference.phi));
    m_charge.push_back(reference.charge);
  }
  return true;
}

}  // namespace L1Muon
