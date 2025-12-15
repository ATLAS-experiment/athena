/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncEventActionTool.h"

#include <memory>

namespace G4UA {

std::unique_ptr<SyncEventAction>
SyncEventActionTool::makeAndFillAction( G4AtlasUserActions& userActions) {
  auto action = std::make_unique<SyncEventAction>();
  userActions.eventActions.push_back(action.get());
  return action;
}

}  // namespace G4UA
