/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncPrimaryGeneratorAction.h"
#include "GaudiKernel/StatusCode.h"

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

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
    // convert Genevent to G4Event
    if(inputEventInfo->EventFactory()(*anEvent, std::move(inputEventInfo)).isFailure()) {
      G4cout << "Failed to prepare G4Event from Athena event" << G4endl;
    }
  }
  // if get_Event returns nullptr, this means that no more events are available and the run should be aborted.
  // Because Geant4 will still go through and finish the current event after calling AbortRun, 
  // and it reset the G4Event::Aborted flag after GeneratePrimaries,  abortRun must be called in the BeginOfEventAction
}

} // namespace G4UA
