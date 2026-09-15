/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncEventAction.h"

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include "G4RunManager.hh"
#include "G4Event.hh"

namespace G4UA
{

void SyncEventAction::BeginOfEventAction(const G4Event* event)
{
  const auto* eventInfo = dynamic_cast<const AtlasG4SyncEventUserInfo*>(
    event->GetUserInformation());
  if(!eventInfo) {
    // A closed event queue supplies no sync event user info.
    G4RunManager::GetRunManager()->AbortRun();
  }
  else if(eventInfo->EventPreparationFailed()) {
    // Event preparation failed. Abort only this event so that the worker can
    // continue processing subsequent Athena events.
    G4RunManager::GetRunManager()->AbortEvent();
  }
}

} // namespace G4UA
