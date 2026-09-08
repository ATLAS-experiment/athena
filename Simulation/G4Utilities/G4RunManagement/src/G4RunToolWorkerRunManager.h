/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_G4RUNTOOLWORKERRUNMANAGER_H
#define G4RUNMANAGEMENT_G4RUNTOOLWORKERRUNMANAGER_H

#include "G4UserWorkerThreadInitialization.hh"
#include "G4WorkerRunManager.hh"

/// Worker run manager which notifies Athena after Geant4 has fully terminated
/// an event.
class G4RunToolWorkerRunManager final : public G4WorkerRunManager
{
 public:
  G4RunToolWorkerRunManager() = default;

 protected:
  void TerminateOneEvent() override;
};

/// Factory used by G4MTRunManager to create G4RunToolWorkerRunManager objects.
class G4RunToolWorkerThreadInitialization final
  : public G4UserWorkerThreadInitialization
{
 public:
  G4WorkerRunManager* CreateWorkerRunManager() const override;
};

#endif
