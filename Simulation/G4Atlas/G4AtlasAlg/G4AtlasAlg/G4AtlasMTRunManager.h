/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4ATLASMTRUNMANAGER_H
#define G4ATLASALG_G4ATLASMTRUNMANAGER_H

// Hide multi-threading classes from builds without G4MT
#include "G4Types.hh"
#ifdef G4MULTITHREADED

// Geant4 includes
#include "G4MTRunManager.hh"

// Framework includes
#include "GaudiKernel/ServiceHandle.h"
#include <GaudiKernel/ToolHandle.h>
#include "AthenaBaseComps/AthMessaging.h"
#include "CxxUtils/checker_macros.h"

// G4Atlas includes
#include "G4AtlasInterfaces/IDetectorConstructionTool.h"
#include "G4AtlasInterfaces/IPhysicsListSvc.h"


/// @class G4AtlasMTRunManager
/// @brief ATLAS master run manager for master-slave multi-threading model
///
/// This thread-local singleton (eww) is used on the G4MT master thread
/// to do some setup for the G4 run.
///
/// The corresponding worker thread run manager is G4AtlasWorkerRunManager.
///
/// @author Steve Farrell <Steven.Farrell@cern.ch>
///
class G4AtlasMTRunManager: public G4MTRunManager, public AthMessaging {

public:

  /// Get the (pure) singleton instance
  static G4AtlasMTRunManager* GetG4AtlasMTRunManager ATLAS_NOT_THREAD_SAFE ();

  /// G4 function called at end of run
  void RunTermination() override final;

  /// We cram all of the initialization of the run manager stuff in here.
  /// This then includes some of the things that in normal G4 are called
  /// immediately before the event loop.
  void Initialize() override final;

  /// Disable G4's barrier synchronization by implementing these methods
  /// and leaving them empty
  virtual void ThisWorkerReady() override final {};
  virtual void ThisWorkerEndEventLoop() override final {};

  /// Configure the detector construction tool
  void SetDetConstructionTool(IDetectorConstructionTool* detConstruction) {
    m_detConstruction = detConstruction;
  }

  /// Configure the Physics List Tool handle
  void SetPhysListSvc(const std::string& typeAndName) {
    m_physListSvc.setTypeAndName(typeAndName);
  }

  /// Configure the QuietMode option
  void SetQuietMode(bool quietMode) {
    m_quietMode = quietMode;
  }
  /// @}

 protected:

  /// Initialize the G4 geometry on the master
  void InitializeGeometry() override final;

  // Initialize the physics list on the master
  void InitializePhysics() override final;

  /// Disable G4's barrier synchronization by implementing these methods
  /// and leaving them empty
  virtual void WaitForReadyWorkers() override final {};
  virtual void WaitForEndEventLoopWorkers() override final {};

private:

  /// Pure singleton private constructor
  G4AtlasMTRunManager();

private:
  /// Handle to the detector construction tool.
  /// Not ideal, because we can't configure this.
  IDetectorConstructionTool* m_detConstruction{nullptr};

  /// Handle to the physics list tool.
  /// Not ideal, because we can't configure this.
  ServiceHandle<IPhysicsListSvc> m_physListSvc;

  /// Quiet Mode for production
  bool m_quietMode{true};

}; // class G4AtlasMTRunManager

#endif // G4MULTITHREADED

#endif
