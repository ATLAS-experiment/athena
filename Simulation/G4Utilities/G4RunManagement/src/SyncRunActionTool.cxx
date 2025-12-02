/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncRunActionTool.h"

#include <memory>

namespace G4UA {

std::unique_ptr<SyncRunAction>
SyncRunActionTool::makeAndFillAction( G4AtlasUserActions& userActions) {
  auto action = std::make_unique<SyncRunAction>(m_g4RunTool);
  userActions.runActionsMaster.push_back(action.get());
  return action;
}

}  // namespace G4UA
