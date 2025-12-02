/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncEventAction.h"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

namespace G4UA
{

void SyncEventAction::BeginOfEventAction(const G4Event* event)
{
  if(!event->GetUserInformation()) {
    G4RunManager::GetRunManager()->AbortRun();
  }
}

void SyncEventAction::EndOfEventAction(const G4Event* event)
{
  // Simulation is done, signal back to Athena
  if(auto* atlasG4EvtUserInfo = dynamic_cast< AtlasG4SyncEventUserInfo* >( event->GetUserInformation() )) { 
    atlasG4EvtUserInfo->SyncInterface()->EventAborted(event->IsAborted());
    atlasG4EvtUserInfo->SyncInterface()->SetStatusDone();
  }
}

} // namespace G4UA
