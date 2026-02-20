/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncPrimaryGeneratorActionTool.h"

#include <memory>

namespace G4UA {

std::unique_ptr<SyncPrimaryGeneratorAction>
SyncPrimaryGeneratorActionTool::makeAndFillAction( G4AtlasUserActions& userActions) {
  auto action = std::make_unique<SyncPrimaryGeneratorAction>(m_g4RunTool);
  userActions.primaryGeneratorActions.push_back(action.get());
  return action;
}

}  // namespace G4UA
