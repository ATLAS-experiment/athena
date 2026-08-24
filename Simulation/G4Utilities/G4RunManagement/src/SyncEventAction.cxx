/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncEventAction.h"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"

namespace G4UA
{

void SyncEventAction::BeginOfEventAction(const G4Event* event)
{
  if(!event->GetUserInformation()) {
    G4RunManager::GetRunManager()->AbortRun();
  }
}

} // namespace G4UA
