/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncPrimaryGeneratorAction.h"
#include "GaudiKernel/StatusCode.h"

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <exception>

namespace G4UA
{

SyncPrimaryGeneratorAction::SyncPrimaryGeneratorAction(IG4RunTool* g4RunTool)
 : G4VUserPrimaryGeneratorAction(), m_g4RunTool(g4RunTool)
{
}

void SyncPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // get an event from the shared queue
  if(auto inputEventInfo = m_g4RunTool->GetEvent()) {
    // setup rng engine seeded by Athena
    G4Random::setTheEngine(inputEventInfo->HepRandomEngine());

    // Transfer ownership before calling the event factory so that Athena can
    // still be notified if event preparation fails.
    auto* eventInfo = inputEventInfo.get();
    anEvent->SetEventID(eventInfo->AthenaEventID());
    anEvent->SetUserInformation(inputEventInfo.release());

    bool eventPreparationFailed = false;
    try {
      // convert Genevent to G4Event
      eventPreparationFailed =
        eventInfo->EventFactory()(*anEvent).isFailure();
      if(eventPreparationFailed) {
        G4cerr << "Failed to prepare G4Event from Athena event" << G4endl;
      }
    }
    catch(const std::exception& error) {
      eventPreparationFailed = true;
      G4cerr << "Exception while preparing G4Event from Athena event: "
             << error.what() << G4endl;
    }
    catch(...) {
      eventPreparationFailed = true;
      G4cerr << "Unknown exception while preparing G4Event from Athena event"
             << G4endl;
    }

    if(eventPreparationFailed) {
      eventInfo->SetEventPreparationFailed();
      // AbortCurrentEvent cannot be called until Geant4 enters EventProc.
      // SyncEventAction will abort this event at BeginOfEventAction.
    }
  }
  // If GetEvent returns nullptr, the event queue has been closed and the run
  // should be aborted.
  // Because GeneratePrimaries runs before Geant4 enters EventProc, AbortRun
  // must be called in BeginOfEventAction.
}

} // namespace G4UA
