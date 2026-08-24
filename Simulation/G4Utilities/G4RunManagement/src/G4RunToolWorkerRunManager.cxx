/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunToolWorkerRunManager.h"

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include "G4Event.hh"

#include <memory>

void G4RunToolWorkerRunManager::TerminateOneEvent()
{
  std::shared_ptr<G4EventSynchronizationInterface> syncInterface;
  bool eventAborted = false;

  if (currentEvent) {
    eventAborted = currentEvent->IsAborted();
    if (auto* eventInfo = dynamic_cast<AtlasG4SyncEventUserInfo*>(
          currentEvent->GetUserInformation())) {
      syncInterface = eventInfo->SyncInterface();
    }
  }

  // This stacks or deletes the G4Event and updates Geant4's event count.
  // Notify Athena only after all Geant4 event processing is complete.
  G4RunManager::TerminateOneEvent();

  if (syncInterface) {
    syncInterface->Complete(eventAborted);
  }
}

G4WorkerRunManager*
G4RunToolWorkerThreadInitialization::CreateWorkerRunManager() const
{
  return new G4RunToolWorkerRunManager;
}
