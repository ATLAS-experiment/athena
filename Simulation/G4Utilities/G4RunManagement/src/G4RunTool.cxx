/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunTool.h"
#include "G4RunToolWorkerRunManager.h"

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

// header files from Geant4
#include "G4MTRunManager.hh"
#include "G4StateManager.hh"
#include "G4UImanager.hh"
#include "G4EventManager.hh"

#include "G4AtlasTools/G4AtlasActionInitialization.h"

// Standard library
#include <exception>
#include <memory>
#include <stdexcept>

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
  m_statusSync.RequestFinalize();

  // Wake every worker blocked waiting for an Athena event. GetEvent() returns
  // nullptr once the queue is closed, causing SyncEventAction to abort the run.
  m_eventQueue.Close();

  if (m_thread) {
    try {
      if(m_thread->joinable()) {
        m_thread->join();
        ATH_MSG_INFO("Geant4 main thread ended");
      }

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
  m_statusSync.NotifyBeginRun();
}

// Wait until Geant4 is ready to start a run (called by Athena initialize).
StatusCode G4RunTool::WaitBeginRun() {
  std::string failureMessage;
  if (!m_statusSync.WaitBeginRun(failureMessage)) {
    ATH_MSG_ERROR("Geant4 failed to start: " << failureMessage);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
// Queue management
//---------------------------------------------------------------------------

// Check the size of the event queue
size_t G4RunTool::Size() const {
  return m_eventQueue.Size();
}

// Push an event to the queue (called from Athena threads)
void G4RunTool::PushEvent(UPEvent ev) {
  m_eventQueue.PushEvent(std::move(ev));
}

/// Get an event from the queue (called from Geant4 threads)
auto G4RunTool::GetEvent() -> UPEvent {
  return m_eventQueue.GetEvent();
}

//---------------------------------------------------------------------------
// Geant4 thread function
//---------------------------------------------------------------------------

// G4 main thread management
void G4RunTool::Geant4main() noexcept {
  std::unique_ptr<G4MTRunManager> runManager;
  try {
    // Keep ownership outside the exception boundary. On failure, the event
    // queue must be closed before the run manager tries to join workers that
    // may still be blocked in GetEvent().
    runManager = std::make_unique<G4MTRunManager>();
    Geant4mainImpl(*runManager);
  }
  catch(const std::exception& error) {
    m_statusSync.Fail(error.what());
    ATH_MSG_ERROR("Exception in Geant4 main thread: " << error.what());
  }
  catch(...) {
    m_statusSync.Fail("Unknown exception in Geant4 main thread");
    ATH_MSG_ERROR("Unknown exception in Geant4 main thread");
  }

  // Teardown ordering is significant: wake blocked workers, join them while
  // destroying the run manager, publish the terminal lifecycle state, and
  // only then release Athena event waiters and their associated resources.
  m_eventQueue.Close();
  runManager.reset();
  m_statusSync.NotifyThreadExit();
  m_eventQueue.CompleteOutstandingEvents();
}

void G4RunTool::Geant4mainImpl(G4MTRunManager& runManager) {

  ATH_MSG_INFO("Geant4 main thread starts with id " << std::this_thread::get_id());

  runManager.SetNumberOfThreads(m_nG4threads);
  constexpr int seedFirstEventOnly = 1; // name the magic number
  // we will take care of reseeding each event, turn off Geant4 reseeding
  runManager.SetSeedOncePerCommunication(seedFirstEventOnly);

  if(m_detConstruction.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve DetectorGeometryService");
    m_statusSync.Fail("Failed to retrieve DetectorGeometryService");
    return;
  }

  if(m_physicsListSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve PhysicsListService");
    m_statusSync.Fail("Failed to retrieve PhysicsListService");
    return;
  }

  if(m_userActionSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve UserActionService");
    m_statusSync.Fail("Failed to retrieve UserActionService");
    return;
  }

  if(m_actionTools.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve ActionTools");
    m_statusSync.Fail("Failed to retrieve ActionTools");
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
  runManager.SetUserInitialization(m_detConstruction->GetDetectorConstruction().release());

  // The actual physics list object must be created in the same thread as the run manager
  runManager.SetUserInitialization(m_physicsListSvc->GetPhysicsList());

  // Set global physics-list options as soon as the list has been created and
  // before any pre-initialization UI commands are applied.
  m_physicsListSvc->SetPhysicsListOptions();

  runManager.SetUserInitialization(
    std::make_unique<G4RunToolWorkerThreadInitialization>().release());
  runManager.SetUserInitialization(
    std::make_unique<G4AtlasActionInitialization>(m_userActionSvc.get()).release());

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
  runManager.Initialize();

  // Process-specific UI commands require the processes to exist first. They
  // are forwarded to the workers with the command stack at the next BeamOn.
  m_physicsListSvc->SetPhysicsProcessOptions();

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
  while (!m_statusSync.StopRequested()) {
    runManager.BeamOn(m_nG4eventsPerRun);
  }
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
