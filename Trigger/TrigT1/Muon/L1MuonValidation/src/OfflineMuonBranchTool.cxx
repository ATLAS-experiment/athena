/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OfflineMuonBranchTool.h"

#include "OfflineMuonBranch.h"

#include <memory>

namespace L1Muon {

OfflineMuonBranchTool::OfflineMuonBranchTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
    : AthAlgTool(type, name, parent) {
  declareInterface<IValidationBranchTool>(this);
}

StatusCode OfflineMuonBranchTool::initialize() {
  ATH_CHECK(m_referenceTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode OfflineMuonBranchTool::addBranches(
    MuonVal::MuonTesterTree& tree) {
  if (!tree.addBranch(std::make_shared<OfflineMuonBranch>(
          tree, m_referenceTool.get(), msgLevel()))) {
    ATH_MSG_ERROR("Failed to add the common OfflineMuonBranch");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
