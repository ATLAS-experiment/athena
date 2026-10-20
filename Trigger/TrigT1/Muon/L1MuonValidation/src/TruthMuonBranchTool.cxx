/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthMuonBranchTool.h"

#include "TruthMuonBranch.h"

#include <memory>

namespace L1Muon {

TruthMuonBranchTool::TruthMuonBranchTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : AthAlgTool(type, name, parent) {
  declareInterface<IValidationBranchTool>(this);
}

StatusCode TruthMuonBranchTool::initialize() {
  ATH_CHECK(m_referenceTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode TruthMuonBranchTool::addBranches(MuonVal::MuonTesterTree& tree) {
  if (!tree.addBranch(std::make_shared<TruthMuonBranch>(
          tree, m_referenceTool.get(), msgLevel()))) {
    ATH_MSG_ERROR("Failed to add the common TruthMuonBranch");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
