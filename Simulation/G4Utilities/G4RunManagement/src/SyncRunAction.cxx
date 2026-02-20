/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SyncRunAction.h"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

namespace G4UA
{

SyncRunAction::SyncRunAction(IG4RunTool* g4RunTool)
 : G4UserRunAction(), m_g4RunTool(g4RunTool)
{
}

void SyncRunAction::BeginOfRunAction(const G4Run* /*run*/)
{
  if(isMaster) {
      G4cout << "Notify Athena that Geant4 run has started"<< G4endl;
      m_g4RunTool->NotifyBeginRun();
  }
}

void SyncRunAction::EndOfRunAction(const G4Run* /*run*/)
{
}

} // namespace G4UA
