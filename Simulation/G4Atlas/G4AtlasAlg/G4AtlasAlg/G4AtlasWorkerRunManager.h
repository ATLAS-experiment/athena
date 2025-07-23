/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4ATLASWORKERRUNMANAGER_H
#define G4ATLASALG_G4ATLASWORKERRUNMANAGER_H

// Hide multi-threading classes from builds without G4MT
#include "G4Types.hh"
#ifdef G4MULTITHREADED

#include "G4WorkerRunManager.hh"
#include "AthenaBaseComps/AthMessaging.h"


/// @brief ATLAS worker run manager for master-slave multi-threading model
class G4AtlasWorkerRunManager : public G4WorkerRunManager, public AthMessaging {

public:

  /// Get the (pure) singleton instance
  static G4AtlasWorkerRunManager* GetG4AtlasWorkerRunManager();

  /// We cram all of the necessary worker run manager initialization here.
  /// In G4 some of it is called instead under BeamOn
  void Initialize() override final;

  /// Does the work of simulating an ATLAS event
  bool ProcessEvent(G4Event* event);

  /// G4 function called at end of run
  void RunTermination() override final;

  /// Configure the QuietMode option
  void SetQuietMode(bool quietMode) {
    m_quietMode = quietMode;
  }
  /// @}

protected:

  /// Initialize the geometry on the worker
  void InitializeGeometry() override final;

  /// Initialize the physics on the worker
  void InitializePhysics() override final;

private:

  /// Pure singleton private constructor
  G4AtlasWorkerRunManager();

  /// This command prints a message about a G4Command depending on its returnCode
  void CommandLog(int returnCode, const std::string& commandString) const;

private:

  /// Quiet Mode for production
  bool m_quietMode{true};

};

#endif // G4MULTITHREADED

#endif
