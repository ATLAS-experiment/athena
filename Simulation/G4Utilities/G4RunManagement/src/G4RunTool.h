/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASSERVICES_G4RunTool_H
#define G4ATLASSERVICES_G4RunTool_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"

// Gaudi headers
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena headers
#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4AtlasInterfaces/IDetectorConstructionTool.h"
#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4AtlasInterfaces/IPhysicsListSvc.h"
#include "G4AtlasInterfaces/IPhysicsInitialization.h"
#include "G4AtlasInterfaces/IUserActionSvc.h"
#include "G4AtlasInterfaces/IUserActionTool.h"
#include "G4AtlasInterfaces/IUserLimitsSvc.h"
#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

// STL headers
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

  /// @class G4RunTool
  /// @brief Front-end service for initializing and interacting with the Geant4 run.
  ///
  /// This public tool will initialize Geant4 in a separate thread which will block until the end of the job.
  /// All the services constructing Geant4 objects (detector construction, physics list, etc.) are retrieved from the Geant4 main thread
  /// during initialize().
  /// 
  /// This tool's initialize() will spawn the Geant4 main thread, but doesn't wait for Geant4 to be initialized before returning.
  /// The reason is that the Geant4 main thread needs to initialize tools, and Gaudi tool initialization is protected by a recursive mutex,
  /// which would lead to a deadlock. Algorithms using this tool should call WaitBeginRun() in their initialize function to ensure Geant4 is ready.
  ///
  /// Athena can push events to Geant4 using the PushEvent method, and Geant4 will retrieve them using GetEvent in the generator action.
  ///
  /// @author Julien Esseiva <julien.esseiva@cern.ch>
  ///
class G4RunTool : public extends<AthAlgTool , IG4RunTool> {
 public:

  // Standard constructor
  using extends::extends;

  // Gaudi methods
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  
  // Status management
  /// Notify Athena that Geant4 is ready to start a run.
  virtual void NotifyBeginRun() override;
  virtual void WaitBeginRun() override;
  
  // Event queue management
  virtual size_t Size() const override;
  virtual void PushEvent(UPEvent ev) override;
  virtual UPEvent GetEvent() override;

 private:
  /// Geant4 main thread function, this is executed in a separate thread and blocks on BeamOn
  void Geant4main();
  /// This command prints a message about a G4Command depending on its returnCode
  void commandLog(int returnCode, const std::string& commandString) const;

  ServiceHandle<IPhysicsListSvc> m_physicsListSvc{this, "PhysicsListSvc", "PhysicsListSvc"};
  ServiceHandle<IUserLimitsSvc> m_userLimitsSvc{this, "UserLimitsSvc", "UserLimitsSvc"};
  ServiceHandle<G4UA::IUserActionSvc> m_userActionSvc{this, "UserActionSvc", "G4UA::UserActionSvc"};

  ToolHandle<IDetectorConstructionTool> m_detConstruction{this, "DetectorConstruction", "", "Tool handle of the DetectorConstruction"};
  ToolHandleArray<G4UA::IUserActionTool> m_actionTools{this, "UserActionTools", {}, "User action tools to be added to the G4 Action service."};
  PublicToolHandleArray<IPhysicsInitializationTool> m_physicsInitializationTools{this, "PhysicsInitializationTools", {}, "Physics initialization happening after Geant4 initialization"};

  Gaudi::Property<bool> m_activateParallelGeometries{this, "ActivateParallelWorlds", false, "Toggle on/off the G4 parallel geometry system"};

  Gaudi::Property<std::string> m_libList{this, "Dll", "", ""};
  Gaudi::Property<std::string> m_physList{this, "Physics", "", ""};
  Gaudi::Property<std::string> m_fieldMap{this, "FieldMap", "", ""};
  Gaudi::Property<std::vector<std::string> > m_g4commands{this, "G4Commands", {}, "Commands to be sent to Geant4 UI at initialization"};

  Gaudi::Property<int> m_nG4threads{this, "NG4threads", 1, "Number of parallel G4 worker threads to launch"};
  Gaudi::Property<int> m_nG4eventsPerRun{this, "NG4eventsPerRun", 100000, "Number of G4 events foreseen for each Run"};

  struct StateSynchronization
  {
    enum class Status {
      Created,
      BeginRun,
      AthenaFinalize,
      Size
    };
    void SetStatus(const Status&);
    void WaitStatus(const Status&);
    Status m_status{Status::Created};
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
  };

  struct EventQueueSynchronization
  {
    std::queue<UPEvent> m_events;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
  };

  // The Geant4 main thread
  std::unique_ptr<std::thread> m_thread;
  
  // Status management
  StateSynchronization m_statusSync;
  
  // Event queue management
  EventQueueSynchronization m_eventQueueSync;
};

#endif
