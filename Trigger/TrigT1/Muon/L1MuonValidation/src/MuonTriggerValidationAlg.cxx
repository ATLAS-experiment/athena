/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTriggerValidationAlg.h"

#include "MuonTesterTree/EventInfoBranch.h"

#include <memory>

namespace L1Muon {

StatusCode MuonTriggerValidationAlg::initialize() {
  m_tree.setPath("MuonTriggerValidation");

  if (m_writeEventInfo.value()) {
    unsigned int eventInfoOptions = 0U;
    if (m_isMC.value()) {
      eventInfoOptions |= MuonVal::EventInfoBranch::isMC;
    }
    if (!m_tree.addBranch(std::make_shared<MuonVal::EventInfoBranch>(
            m_tree, eventInfoOptions, "EventInfo"))) {
      ATH_MSG_ERROR("Failed to add the common EventInfo branch");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(m_branchTools.retrieve());
  for (ToolHandle<IValidationBranchTool>& tool : m_branchTools) {
    ATH_MSG_DEBUG("Register validation branches from " << tool.name());
    ATH_CHECK(tool->addBranches(m_tree));
  }

  ATH_CHECK(m_tree.init(this));
  return StatusCode::SUCCESS;
}

StatusCode MuonTriggerValidationAlg::execute(const EventContext& ctx) {
  return m_tree.fill(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode MuonTriggerValidationAlg::finalize() {
  ATH_CHECK(m_tree.write());
  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
