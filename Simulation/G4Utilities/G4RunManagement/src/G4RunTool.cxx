/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunTool.h"

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

// header files from Geant4
#include "G4MTRunManager.hh"
#include "G4StateManager.hh"
#include "G4UImanager.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4EventManager.hh"

#include "G4AtlasTools/G4AtlasActionInitialization.h"

// Standard library
#include <memory>

/// Initialize and start the Geant4 main thread, then wait until Geant4 is ready to start the run.
StatusCode G4RunTool::initialize() {

  // Gaudi will automatically initialize child tools once this tool is initialized.
  // We want these child tools to be initialized in the Geant4 main thread,
  // disable() will tell Gaudi not to initialize them now.
  // Ideally we'd move the initialization that needs to happen in the Geant4 main thread out of initialize
  // and into a separate method called from Geant4main.
  m_detConstruction.disable();
  for(auto& tool : m_actionTools) {
    tool.disable();
  }

  m_thread = std::make_unique<std::thread>(&G4RunTool::Geant4main, this);
  ATH_MSG_DEBUG("Geant4main thread created, id=" << m_thread->get_id() << ", now waiting for G4 run manager");
  return StatusCode::SUCCESS;
}

StatusCode G4RunTool::finalize(){
  // Signal finalization to G4 threads
  m_statusSync.SetStatus(StateSynchronization::Status::AthenaFinalize);
  
  // G4 worker threads should be waiting for the next event to simulate at this state
  // Pushing empty event, will cause one worker thread to realize we are done and call AbortRun for G4
  for(int i = 0; i < m_nG4threads; ++i) {
    PushEvent(nullptr);
  }

  if (m_thread) {
    try {
      if(m_thread->joinable()) m_thread->join();
    }  
    catch(const std::exception& e) {
      ATH_MSG_ERROR("Failure in G4RunTool::finalize, joining Geant4 main thread:" << e.what());
    }
  }

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
// Status management
//---------------------------------------------------------------------------

// Notify Athena that Geant4 is ready to start a run (called by Geant4 BeginOfRunAction).
void G4RunTool::NotifyBeginRun() {
  m_statusSync.SetStatus(StateSynchronization::Status::BeginRun);
}

// Wait until Geant4 is ready to start a run (called by Athena initialize).
void G4RunTool::WaitBeginRun() {
  m_statusSync.WaitStatus(StateSynchronization::Status::BeginRun);
}

void G4RunTool::StateSynchronization::SetStatus(const Status& status) {
  {
    std::scoped_lock lk(m_mutex);
    m_status = status;
  }
  m_cv.notify_all();
}

void G4RunTool::StateSynchronization::WaitStatus(const Status& status) {
  std::unique_lock lk(m_mutex);
  m_cv.wait(lk, [this, status]{ return m_status == status; });
}

//---------------------------------------------------------------------------
// Queue management
//---------------------------------------------------------------------------

// Check the size of the event queue
size_t G4RunTool::Size() const {
  std::scoped_lock lk(m_eventQueueSync.m_mutex);
  return m_eventQueueSync.m_events.size();
}

// Push an event to the queue (called from Athena threads)
void G4RunTool::PushEvent(UPEvent ev) {
  {
    std::scoped_lock lk(m_eventQueueSync.m_mutex);
    m_eventQueueSync.m_events.push(std::move(ev));
  }
  m_eventQueueSync.m_cv.notify_one();
}

/// Get an event from the queue (called from Geant4 threads)
auto G4RunTool::GetEvent() -> UPEvent {

  std::unique_lock lk(m_eventQueueSync.m_mutex);
  m_eventQueueSync.m_cv.wait(lk, [this]{ return m_eventQueueSync.m_events.size() > 0; });
  UPEvent ev = std::move(m_eventQueueSync.m_events.front());
  m_eventQueueSync.m_events.pop();
  return ev;
}

//---------------------------------------------------------------------------
// Geant4 thread function
//---------------------------------------------------------------------------

// G4 main thread management
void G4RunTool::Geant4main() {

  ATH_MSG_INFO("Geant4 main thread starts with id " << std::this_thread::get_id());
  
  // Construct the default run manager
  auto runManager = std::make_unique<G4MTRunManager>();

  runManager->SetNumberOfThreads(m_nG4threads);
  constexpr int seedFirstEventOnly = 1; // name the magic number
  // we will take care of reseeding each event, turn off Geant4 reseeding
  runManager->SetSeedOncePerCommunication(seedFirstEventOnly);

  if(m_detConstruction.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve DetectorGeometryService");
    return;
  }

  if(m_physicsListSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve PhysicsListService");
    return;
  }

  if(m_userActionSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve UserActionService");
    return;
  }

  if(m_actionTools.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve ActionTools");
    return;
  }

  // Initialize action tools for G4RunTool
  for (const auto& action_tool : m_actionTools) {
    ATH_MSG_INFO("retrieving action tool " + action_tool.name());
    if (m_userActionSvc->addActionTool(action_tool).isFailure()) {
      throw std::runtime_error("Failed to add action tool " + action_tool.name());
    }
  }
  // Set the user action service for the G4UA service
  // Having a ServiceHandle<IG4RunTool> would be a circular dependency which is not supported...
  m_userActionSvc->G4RunTool(this);

  // Many of the objects created here must be created in the same thread as the run manager
  runManager->SetUserInitialization(m_detConstruction->GetDetectorConstruction().release());

  // The actual physics list object must be created in the same thread as the run manager
  runManager->SetUserInitialization(m_physicsListSvc->GetPhysicsList());

  runManager->SetUserInitialization(std::make_unique<G4AtlasActionInitialization>(m_userActionSvc.get()).release());

  // G4 user interface commands
  G4UImanager *ui = G4UImanager::GetUIpointer();

  // Load custom libraries
  if (!m_libList.empty()) {
    ATH_MSG_INFO("G4AtlasAlg specific libraries requested ");
    std::string temp="/load "+m_libList;
    ui->ApplyCommand(temp);
  }
  // Load custom physics
  if (!m_physList.empty()) {
    ATH_MSG_INFO("requesting a specific physics list "<< m_physList);
    std::string temp="/Physics/GetPhysicsList "+m_physList;
    ui->ApplyCommand(temp);
  }
  // Load custom magnetic field
  if (!m_fieldMap.empty()) {
    ATH_MSG_INFO("requesting a specific field map "<< m_fieldMap);
    ATH_MSG_INFO("the field is initialized straight away");
    std::string temp="/MagneticField/Select "+m_fieldMap;
    ui->ApplyCommand(temp);
    ui->ApplyCommand("/MagneticField/Initialize");
  }

  // Send UI commands
  ATH_MSG_DEBUG("G4 Command: Trying at the end of initializeOnce()");
  for (const auto& g4command : m_g4commands) {
    int returnCode = ui->ApplyCommand( g4command );
    commandLog(returnCode, g4command);
  }

  // Initialize run
  runManager->Initialize();

  m_physicsListSvc->SetPhysicsOptions();

  ATH_MSG_INFO("Initializing " << m_physicsInitializationTools.size() << " physics initialization tools");
  for(auto& physicsTool : m_physicsInitializationTools) {
    if (physicsTool->initializePhysics().isFailure()) {
      throw std::runtime_error("Failed to initialize physics with tool " + physicsTool.name());
    }
  }
  // Retrieve core services needed for G4 main thread
  if(m_userLimitsSvc.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS UserLimitsSvc!");
  }

  ATH_MSG_INFO("Geant4 initialization done, BeamOn...");

  // Repeat BeamOn as long as athena event loop is not finished
  while (m_statusSync.m_status != StateSynchronization::Status::AthenaFinalize) {
    runManager->BeamOn(m_nG4eventsPerRun);
  }

  // Clean up geometry
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  ATH_MSG_INFO("Geant4 main thread ended");
}

void G4RunTool::commandLog(int returnCode, const std::string& commandString) const
{
  switch(returnCode) {
  case 0: { ATH_MSG_DEBUG("G4 Command: " << commandString << " - Command Succeeded"); } break;
  case 100: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Command Not Found!"); } break;
  case 200: {
    auto* stateManager = G4StateManager::GetStateManager();
    ATH_MSG_DEBUG("G4 Command: " << commandString << " - Illegal Application State (" <<
                    stateManager->GetStateString(stateManager->GetCurrentState()) << ")!");
  } break;
  case 300: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Out of Range!"); } break;
  case 400: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Unreadable!"); } break;
  case 500: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Out of Candidates!"); } break;
  case 600: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Alias Not Found!"); } break;
  default: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Unknown Status!"); } break;
  }

}
