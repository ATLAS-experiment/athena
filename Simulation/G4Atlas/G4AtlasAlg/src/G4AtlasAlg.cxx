/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "G4AtlasAlg.h"
#include "G4AtlasFluxRecorder.h"
#include "G4AtlasAlg/G4AtlasActionInitialization.h"
#include "G4AtlasTools/G4AtlasUserWorkerInitialization.h"

#include "AthenaKernel/RNGWrapper.h"
#include "CxxUtils/checker_macros.h"

// Can we safely include all of these?
#include "G4AtlasAlg/G4AtlasMTRunManager.h"
#include "G4AtlasAlg/G4AtlasRunManager.h"
#include "G4AtlasAlg/G4AtlasUserWorkerThreadInitialization.h"
#include "G4AtlasAlg/G4AtlasWorkerRunManager.h"

// Geant4 includes
#include <G4Event.hh>

#include "G4EventManager.hh"
#include "G4GDMLParser.hh"
#include "G4Navigator.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4PropagatorInField.hh"
#include "G4RunManagerKernel.hh"
#include "G4ScoringManager.hh"
#include "G4StackManager.hh"
#include "G4StateManager.hh"
#include "G4TrackingManager.hh"
#include "G4TransportationManager.hh"
#include "G4UImanager.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VUserPhysicsList.hh"

// CLHEP includes
#include "CLHEP/Random/RandomEngine.h"

// Athena includes
#include "GaudiKernel/IThreadInitTool.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "HitManagement/HitCollectionMap.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "MCTruthBase/TruthStrategyManager.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// standard library
#include <memory>
#include <mutex>
static std::once_flag initializeOnceFlag;
static std::once_flag finalizeOnceFlag;
static std::once_flag releaseGeoModelOnceFlag;

/////////////////////////////////////////////////////////////////////////////


G4AtlasAlg::G4AtlasAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
  // Verbosities
  declareProperty("Verbosities", m_verbosities);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode G4AtlasAlg::initialize ATLAS_NOT_THREAD_SAFE ()
{
  ATH_MSG_DEBUG("Start of initialize()");

  // Read the simplified geometry for FastCaloSim track transportation if requested
  if(!m_simplifiedGeoPath.empty()) {
    std::string geoFile = PathResolverFindCalibFile(m_simplifiedGeoPath);
    
    if (geoFile.empty()) {
      ATH_MSG_FATAL("Could not find simplified geometry file: " << m_simplifiedGeoPath);
      return StatusCode::FAILURE;
    }

    G4GDMLParser parser;
    parser.Read(geoFile, false);
  }

  // Create the scoring manager if requested
  if (m_recordFlux) G4ScoringManager::GetScoringManager();

  ATH_CHECK( m_userActionSvc.retrieve() );
  // One-time initialization
  try {
    std::call_once(initializeOnceFlag, &G4AtlasAlg::initializeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in G4AtlasAlg::initializeOnce: " << e.what());
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_rndmGenSvc.retrieve() );
  ATH_CHECK(m_actionTools.retrieve());

  ATH_CHECK(m_senDetTool.retrieve());
  ATH_CHECK(m_fastSimTool.retrieve());

  // Truth
  ATH_CHECK( m_truthRecordSvc.retrieve() );
  ATH_MSG_INFO( "- Using ISF TruthRecordSvc : " << m_truthRecordSvc.typeAndName() );
  ATH_CHECK( m_geoIDSvc.retrieve() );
  ATH_MSG_INFO( "- Using ISF GeoIDSvc       : " << m_geoIDSvc.typeAndName() );

  TruthStrategyManager& sManager = TruthStrategyManager::GetStrategyManager_nc();
  sManager.SetISFTruthSvc( &(*m_truthRecordSvc) );
  sManager.SetISFGeoIDSvc( &(*m_geoIDSvc) );

  // I/O
  ATH_CHECK( m_inputTruthCollectionKey.initialize());
  ATH_CHECK( m_outputTruthCollectionKey.initialize());
  ATH_CHECK( m_eventInfoKey.initialize() );

  ATH_CHECK(m_inputConverter.retrieve());
  if ( not m_truthPreselectionTool.empty() ) {
    ATH_CHECK(m_truthPreselectionTool.retrieve());
  }

  if ( not m_qspatcher.empty() ) {
    ATH_CHECK( m_qspatcher.retrieve() );
  }

  ATH_MSG_DEBUG("End of initialize()");
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void G4AtlasAlg::initializeOnce()
{
  // Assign physics list
  if(m_physListSvc.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS PhysicsListSvc!");
  }
  for (const auto& action_tool : m_actionTools) {
    if (m_userActionSvc->addActionTool(action_tool).isFailure()) {
      throw std::runtime_error("Failed to add action tool "+action_tool.name());
    }
  }

  ATH_MSG_INFO( "retrieving the Detector Construction tool" );
  if(m_detConstruction.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS DetectorConstruction!");
  }

  // Create the (master) run manager
  if(m_useMT) {
#ifdef G4MULTITHREADED
    auto* runMgr ATLAS_THREAD_SAFE = // protected by std::call_once above
      G4AtlasMTRunManager::GetG4AtlasMTRunManager();
    m_physListSvc->SetPhysicsList();
    runMgr->SetDetConstructionTool( m_detConstruction.get() );
    runMgr->SetPhysListSvc( m_physListSvc.typeAndName() );
    runMgr->SetQuietMode( m_quietMode );
    // Worker Thread initialization used to create worker run manager on demand.
    std::unique_ptr<G4AtlasUserWorkerThreadInitialization> workerInit =
      std::make_unique<G4AtlasUserWorkerThreadInitialization>();
    workerInit->SetQuietMode( m_quietMode );
    runMgr->SetUserInitialization( workerInit.release() );
    std::unique_ptr<G4AtlasActionInitialization> actionInitialization =
      std::make_unique<G4AtlasActionInitialization>(m_userActionSvc.get());
    runMgr->SetUserInitialization(actionInitialization.release());
    runMgr->SetUserInitialization(new G4AtlasUserWorkerInitialization({.m_activateFastSimulation = m_fastSimTool->HasFastSimulationModels()}));
#else
    throw std::runtime_error("Trying to use multi-threading in non-MT build!");
#endif
  }
  // Single-threaded run manager
  else {
    auto* runMgr ATLAS_THREAD_SAFE = // safe because single-threaded
      G4AtlasRunManager::GetG4AtlasRunManager();
    m_physListSvc->SetPhysicsList();
    runMgr->SetRecordFlux( m_recordFlux, std::make_unique<G4AtlasFluxRecorder>() );
    runMgr->SetLogLevel( int(msg().level()) ); // Synch log levels
    runMgr->SetDetConstructionTool( m_detConstruction.get() );
    runMgr->SetPhysListSvc(m_physListSvc.typeAndName() );
    runMgr->SetQuietMode( m_quietMode );
    std::unique_ptr<G4AtlasActionInitialization> actionInitialization =
      std::make_unique<G4AtlasActionInitialization>(m_userActionSvc.get());
    runMgr->SetUserInitialization(actionInitialization.release());
    runMgr->SetUserInitialization(new G4AtlasUserWorkerInitialization({.m_activateFastSimulation = m_fastSimTool->HasFastSimulationModels()}));
  }

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

  // Code from G4AtlasSvc
  auto* rm = G4RunManager::GetRunManager();
  if(!rm) {
    throw std::runtime_error("Run manager retrieval has failed");
  }
  rm->Initialize();     // Initialization differs slightly in multi-threading.
  // TODO: add more details about why this is here.
  if(!m_useMT && rm->ConfirmBeamOnCondition()) {
    rm->RunInitialization();
  }

  ATH_MSG_INFO("Initializing " << m_physicsInitializationTools.size() << " physics initialization tools");
  for(auto& physicsTool : m_physicsInitializationTools) {
    if (physicsTool->initializePhysics().isFailure()) {
      throw std::runtime_error("Failed to initialize physics with tool " + physicsTool.name());
    }
  }

  if(m_userLimitsSvc.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS UserLimitsSvc!");
  }

  if (m_activateParallelGeometries) {
    G4VModularPhysicsList* thePhysicsList=dynamic_cast<G4VModularPhysicsList*>(m_physListSvc->GetPhysicsList());
    if (!thePhysicsList) {
      throw std::runtime_error("Failed dynamic_cast!! this is not a G4VModularPhysicsList!");
    }
#if G4VERSION_NUMBER >= 1010
    std::vector<std::string>& parallelWorldNames=m_detConstruction->GetParallelWorldNames();
    for (auto& it: parallelWorldNames) {
      thePhysicsList->RegisterPhysics(new G4ParallelWorldPhysics(it,true));
    }
#endif
  }

  return;
}

void G4AtlasAlg::initializeG4()
{
  if (m_verbosities.size()>0) {
    G4TransportationManager *tm = G4TransportationManager::GetTransportationManager();
    G4RunManagerKernel *rmk = G4RunManagerKernel::GetRunManagerKernel();
    G4EventManager *em = G4EventManager::GetEventManager();

    auto itr = m_verbosities.end();
    if ((itr = m_verbosities.find("Navigator")) != m_verbosities.end()) {
      tm->GetNavigatorForTracking()->SetVerboseLevel( atof(itr->second.data()) );
    }
    if ((itr = m_verbosities.find("Propagator")) != m_verbosities.end()) {
      tm->GetPropagatorInField()->SetVerboseLevel( atof(itr->second.data()) );
    }
    if ((itr = m_verbosities.find("Tracking")) != m_verbosities.end()) {
      rmk->GetTrackingManager()->SetVerboseLevel( atof(itr->second.data()) );
    }
    if ((itr = m_verbosities.find("Stepping")) != m_verbosities.end()) {
      rmk->GetTrackingManager()->GetSteppingManager()->
        SetVerboseLevel( atof(itr->second.data()) );
    }
    if ((itr = m_verbosities.find("Stacking")) != m_verbosities.end()) {
      rmk->GetStackManager()->SetVerboseLevel( atof(itr->second.data()) );
    }
    if ((itr = m_verbosities.find("Event")) != m_verbosities.end()) {
      em->SetVerboseLevel( atof(itr->second.data()) );
    }
  } // End of the setting of verbosities

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode G4AtlasAlg::finalize()
{
  ATH_MSG_DEBUG(std::endl<<std::endl<<std::endl);
  ATH_MSG_DEBUG("++++++++++++  G4AtlasAlg finalized  ++++++++++++" <<std::endl<<std::endl);

  // One time finalization
  try {
    std::call_once(finalizeOnceFlag, &G4AtlasAlg::finalizeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in G4AtlasAlg::finalizeOnce: " << e.what());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void G4AtlasAlg::finalizeOnce()
{
  ATH_MSG_DEBUG("\t terminating the current G4 run");
  auto runMgr = G4RunManager::GetRunManager();
  runMgr->RunTermination();
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode G4AtlasAlg::execute()
{
  static std::atomic<unsigned int> n_Event=0;
  ATH_MSG_DEBUG("++++++++++++  G4AtlasAlg execute  ++++++++++++");

  n_Event += 1;

  if (n_Event<=10 || (n_Event%100) == 0) {
    ATH_MSG_ALWAYS("G4AtlasAlg: Event num. "  << n_Event << " start processing");
  }

  // Release GeoModel Geometry if necessary
  if (m_releaseGeoModel) {
    try {
      std::call_once(releaseGeoModelOnceFlag, &G4AtlasAlg::releaseGeoModel, this);
    }
    catch(const std::exception& e) {
      ATH_MSG_ERROR("Failure in G4AtlasAlg::releaseGeoModel: " << e.what());
      return StatusCode::FAILURE;
    }
  }

  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Set the RNG to use for this event. We need to reset it for MT jobs
  // because of the mismatch between Gaudi slot-local and G4 thread-local RNG.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName,  ctx);
  G4Random::setTheEngine(*rngWrapper);

  ATH_MSG_DEBUG("Calling SimulateG4Event");

  auto eventInfo = std::make_unique<AtlasG4EventUserInfo>();
  // get a shared pointer to the hit collection map because we will need it after the G4Event is destroyed
  std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();

  ATH_CHECK(m_senDetTool->BeginOfAthenaEvent(*hitCollections));
  ATH_CHECK(m_fastSimTool->BeginOfAthenaEvent());

  SG::ReadHandle<McEventCollection> inputTruthCollection(m_inputTruthCollectionKey);
  if (!inputTruthCollection.isValid()) {
    ATH_MSG_FATAL("Unable to read input GenEvent collection " << inputTruthCollection.name() << " in store " << inputTruthCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found input GenEvent collection " << inputTruthCollection.name() << " in store " << inputTruthCollection.store());
  // create the output Truth collection
  SG::WriteHandle<McEventCollection> outputTruthCollection(m_outputTruthCollectionKey);
  std::unique_ptr<McEventCollection> shadowTruth{};
  if (m_useShadowEvent) {
    outputTruthCollection = std::make_unique<McEventCollection>();
    // copy input Evgen collection to shadow Truth collection
    shadowTruth = std::make_unique<McEventCollection>(*inputTruthCollection);
    for (HepMC::GenEvent* currentGenEvent : *shadowTruth ) {
      // Apply QS patch if required
      if ( not m_qspatcher.empty() ) {
        ATH_CHECK(m_qspatcher->applyWorkaround(*currentGenEvent));
      }
      // Copy GenEvent and remove daughters of quasi-stable particles to be simulated
      std::unique_ptr<HepMC::GenEvent> outputEvent = m_truthPreselectionTool->filterGenEvent(*currentGenEvent);
      outputTruthCollection->push_back(outputEvent.release());
    }
  }
  else {
    // copy input Evgen collection to output Truth collection
    outputTruthCollection = std::make_unique<McEventCollection>(*inputTruthCollection);
    // empty shadow Truth collection
    shadowTruth = std::make_unique<McEventCollection>();
    // Apply QS patch if required
    if ( not m_qspatcher.empty() ) {
      for (HepMC::GenEvent* currentGenEvent : *outputTruthCollection ) {
        ATH_CHECK(m_qspatcher->applyWorkaround(*currentGenEvent));
      }
    }
  }

  ATH_MSG_DEBUG("Recorded output GenEvent collection " << outputTruthCollection.name() << " in store " << outputTruthCollection.store());

  const int largestGeneratedParticleBC =  (outputTruthCollection->empty()) ? HepMC::UNDEFINED_ID
    : HepMC::maxGeneratedParticleBarcode(outputTruthCollection->at(0)); // TODO make this more robust
  const int largestGeneratedVertexBC =  (outputTruthCollection->empty()) ? HepMC::UNDEFINED_ID
    : HepMC::maxGeneratedVertexBarcode(outputTruthCollection->at(0)); // TODO make this more robust
  // tell TruthService we're starting a new event
  ATH_CHECK( m_truthRecordSvc->initializeTruthCollection(largestGeneratedParticleBC, largestGeneratedVertexBC) );

  bool abort = false;

  {

    auto inputEvent = std::make_unique<G4Event>(ctx.eventID().event_number());
    inputEvent->SetUserInformation(eventInfo.release());

    ATH_CHECK(m_inputConverter->convertHepMCToG4Event(
        *outputTruthCollection, *inputEvent, *shadowTruth));
    // Worker run manager
    // Custom class has custom method call: ProcessEvent.
    // So, grab custom singleton class directly, rather than base.
    // Maybe that should be changed! Then we can use a base pointer.
    if (m_useMT) {
#ifdef G4MULTITHREADED
      auto* workerRM = G4AtlasWorkerRunManager::GetG4AtlasWorkerRunManager();
      abort = workerRM->ProcessEvent(inputEvent.release());
#else
      ATH_MSG_ERROR("Trying to use multi-threading in non-MT build!");
      return StatusCode::FAILURE;
#endif
    } else {
      auto* workerRM ATLAS_THREAD_SAFE =  // single-threaded case
          G4AtlasRunManager::GetG4AtlasRunManager();
      abort = workerRM->ProcessEvent(inputEvent.release());
    }

    if (abort) {
      ATH_MSG_WARNING("Event was aborted !! ");
      ATH_MSG_WARNING("Simulation will now go on to the next event ");
      if (m_killAbortedEvents) {
        ATH_MSG_WARNING("setFilterPassed is now False");
        setFilterPassed(false);
      }
      if (m_flagAbortedEvents) {
        SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
        if (!eventInfo.isValid()) {
          ATH_MSG_FATAL(
              "Failed to retrieve xAOD::EventInfo while trying to update the "
              "error state!");
          return StatusCode::FAILURE;
        } else {
          eventInfo->updateErrorState(xAOD::EventInfo::Core,
                                      xAOD::EventInfo::Error);
          ATH_MSG_WARNING("Set error state in xAOD::EventInfo!");
        }
      }
    }

    ATH_CHECK(m_senDetTool->EndOfAthenaEvent(*hitCollections));
    ATH_CHECK(m_fastSimTool->EndOfAthenaEvent());

    ATH_CHECK(m_truthRecordSvc->releaseEvent());
  }
  // Remove QS patch if required
  if(!m_qspatcher.empty()) {
    for (HepMC::GenEvent* currentGenEvent : *outputTruthCollection ) {
      ATH_CHECK(m_qspatcher->removeWorkaround(*currentGenEvent));
    }
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void G4AtlasAlg::releaseGeoModel()
{
  SmartIF<IGeoModelSvc> geoModel{Gaudi::svcLocator()->service("GeoModelSvc")};
  if (!geoModel) {
    ATH_MSG_WARNING( " ----> Unable to retrieve GeoModelSvc" );
  }
  else {
    if (geoModel->clear().isFailure()) {
      ATH_MSG_WARNING( " ----> GeoModelSvc::clear() failed" );
    }
    else {
      ATH_MSG_INFO( " ----> GeoModelSvc::clear() succeeded " );
    }
  }
  m_releaseGeoModel=false; // Don't do that again...
  return;
}

void G4AtlasAlg::commandLog(int returnCode, const std::string& commandString) const
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
