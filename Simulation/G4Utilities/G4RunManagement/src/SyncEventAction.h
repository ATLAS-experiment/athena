/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_SyncEventAction_H
#define G4RUNMANAGEMENT_SyncEventAction_H

#include "G4UserEventAction.hh"

namespace G4UA
{

class SyncEventAction : public G4UserEventAction
{
public:
  virtual void BeginOfEventAction(const G4Event*) override;
  virtual void EndOfEventAction(const G4Event*) override;
};

} // namespace G4UA

#endif

    
