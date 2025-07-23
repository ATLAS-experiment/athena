/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "TransportTool.h"

//package includes
#include "AthenaKernel/RNGWrapper.h"
#include "G4AtlasAlg/G4AtlasActionInitialization.h"
#include "G4AtlasAlg/G4AtlasMTRunManager.h"
#include "G4AtlasAlg/G4AtlasRunManager.h"
#include "G4AtlasTools/G4AtlasUserWorkerInitialization.h"
#include "G4AtlasAlg/G4AtlasUserWorkerThreadInitialization.h"
#include "G4AtlasAlg/G4AtlasWorkerRunManager.h"
#include "ISFFluxRecorder.h"

// ISF classes
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleContainer.h"

// Athena classes
#include "AtlasDetDescr/AtlasRegionHelper.h"
#include "GaudiKernel/IThreadInitTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "MCTruth/AtlasG4EventUserInfo.h"
#include "HitManagement/HitCollectionMap.h"
#include "MCTruth/PrimaryParticleInformation.h"

// HepMC classes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"

// Geant4 classes
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4LorentzVector.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4ParticleTable.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SDManager.hh"
#include "G4ScoringManager.hh"
#include "G4StateManager.hh"
#include "G4Timer.hh"
#include "G4Trajectory.hh"
#include "G4TransportationManager.hh"
#include "G4UImanager.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VUserPhysicsList.hh"

// std library
#include <G4Event.hh>
#include <memory>
#include <mutex>
static std::once_flag initializeOnceFlag;
static std::once_flag finalizeOnceFlag;

//________________________________________________________________________
iGeant4::G4TransportTool::G4TransportTool(const std::string& type,
                                          const std::string& name,
                                          const IInterface*  parent )
  : ISF::BaseSimulatorG4Tool(type, name, parent)
{
  //declareProperty("KillAllNeutrinos",      m_KillAllNeutrinos=true);
  //declareProperty("KillLowEPhotons",       m_KillLowEPhotons=-1.);
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::initialize()
{
  ATH_MSG_VERBOSE("initialize");

  ATH_CHECK( ISF::BaseSimulatorTool::initialize() );

  // create G4Timers if enabled
  if (m_doTiming) {
    m_runTimer   = new G4Timer();
    m_eventTimer = new G4Timer();
    m_runTimer->Start();
  }

  // Create the scoring manager if requested
  if (m_recordFlux) G4ScoringManager::GetScoringManager();

  // One-time initialization
  try {
    std::call_once(initializeOnceFlag, &iGeant4::G4TransportTool::initializeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in iGeant4::G4TransportTool::initializeOnce: " << e.what());
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_rndmGenSvc.retrieve() );
  ATH_CHECK( m_userActionSvc.retrieve() );

  ATH_CHECK(m_senDetTool.retrieve());
  ATH_CHECK(m_fastSimTool.retrieve());

  ATH_CHECK(m_inputConverter.retrieve());

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
void iGeant4::G4TransportTool::initializeOnce ATLAS_NOT_THREAD_SAFE ()
{
  // get G4AtlasRunManager
  ATH_MSG_DEBUG("initialize G4AtlasRunManager");

  if(m_physListSvc.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS PhysicsListSvc!");
  }
  ATH_MSG_INFO( "retireving the Detector Construction tool" );
  if(m_detConstruction.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS DetectorConstruction!");
  }


  // Create the (master) run manager
  if(m_useMT) {
#ifdef G4MULTITHREADED
    auto* runMgr = G4AtlasMTRunManager::GetG4AtlasMTRunManager();
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
      std::make_unique<G4AtlasActionInitialization>(&*m_userActionSvc);
    runMgr->SetUserInitialization(actionInitialization.release());
    runMgr->SetUserInitialization(new G4AtlasUserWorkerInitialization({.m_activateFastSimulation = m_fastSimTool->HasFastSimulationModels()}));
#else
    throw std::runtime_error("Trying to use multi-threading in non-MT build!");
#endif
  }
  // Single-threaded run manager
  else {
    auto* runMgr = G4AtlasRunManager::GetG4AtlasRunManager();
    m_physListSvc->SetPhysicsList();
    runMgr->SetRecordFlux( m_recordFlux, std::make_unique<ISFFluxRecorder>() );
    runMgr->SetLogLevel( int(msg().level()) ); // Synch log levels
    runMgr->SetDetConstructionTool( m_detConstruction.get() );
    runMgr->SetPhysListSvc(m_physListSvc.typeAndName() );
    runMgr->SetQuietMode( m_quietMode );
    std::unique_ptr<G4AtlasActionInitialization> actionInitialization =
      std::make_unique<G4AtlasActionInitialization>(&*m_userActionSvc);
    runMgr->SetUserInitialization(actionInitialization.release());
    runMgr->SetUserInitialization(new G4AtlasUserWorkerInitialization({.m_activateFastSimulation = m_fastSimTool->HasFastSimulationModels()}));
  }

  G4UImanager *ui = G4UImanager::GetUIpointer();

  if (!m_libList.empty()) {
    ATH_MSG_INFO("G4AtlasAlg specific libraries requested ") ;
    std::string temp="/load "+m_libList;
    ui->ApplyCommand(temp);
  }

  if (!m_physList.empty()) {
    ATH_MSG_INFO("requesting a specific physics list "<< m_physList) ;
    std::string temp="/Physics/GetPhysicsList "+m_physList;
    ui->ApplyCommand(temp);
  }

  if (!m_fieldMap.empty()) {
    ATH_MSG_INFO("requesting a specific field map "<< m_fieldMap) ;
    ATH_MSG_INFO("the field is initialized straight away") ;
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

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::finalize()
{
  ATH_MSG_VERBOSE("++++++++++++  ISF G4 G4TransportTool finalized  ++++++++++++");

  // One time finalization
  try {
    std::call_once(finalizeOnceFlag, &iGeant4::G4TransportTool::finalizeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in iGeant4::G4TransportTool::finalizeOnce: " << e.what());
    return StatusCode::FAILURE;
  }

  if (m_doTiming) {
    m_runTimer->Stop();
    const float numEntriesFloat(m_nrOfEntries);
    const float runTime=m_runTimer->GetUserElapsed()+m_runTimer->GetSystemElapsed();
    const float avgTimePerEvent=(m_nrOfEntries>1) ? m_accumulatedEventTime/(numEntriesFloat-1.f) : runTime;
    const float avgTimeSqPerEvent=(m_nrOfEntries>1) ? m_accumulatedEventTimeSq/(numEntriesFloat-1.f) : runTime*runTime;
    const float sigma=(m_nrOfEntries>2) ? std::sqrt(std::abs(avgTimeSqPerEvent - avgTimePerEvent*avgTimePerEvent)/(numEntriesFloat-2.f)) : 0;
    ATH_MSG_INFO("*****************************************"<<endmsg<<
                 "**                                     **"<<endmsg<<
                 "    End of run - time spent is "<<std::setprecision(4) <<
                 runTime<<endmsg<<
                 "    Average time per event was "<<std::setprecision(4) <<
                 avgTimePerEvent <<" +- "<< std::setprecision(4) << sigma<<endmsg<<
                 "**                                     **"<<endmsg<<
                 "*****************************************");
  }

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
void iGeant4::G4TransportTool::finalizeOnce()
{
  ATH_MSG_DEBUG("\t terminating the current G4 run");
  auto runMgr = G4RunManager::GetRunManager();
  runMgr->RunTermination();
  return;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::simulate(
    const EventContext& ctx, ISF::ISFParticle& isp,
    ISF::ISFParticleContainer& secondaries,
    McEventCollection* mcEventCollection, std::shared_ptr<HitCollectionMap> hitCollections) {

  // give a screen output that you entered Geant4SimSvc
  ATH_MSG_VERBOSE( "Particle " << isp << " received for simulation." );

  /** Process ParticleState from particle stack */
  // wrap the given ISFParticle into a STL vector of ISFParticles with length 1
  // (minimizing code duplication)
  const ISF::ISFParticleVector ispVector(1, &isp);
  StatusCode success = this->simulateVector(ctx, ispVector, secondaries,
                                            mcEventCollection, hitCollections);
  ATH_MSG_VERBOSE( "Simulation done" );

  // Geant4 call done
  return success;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::simulateVector(
    const EventContext& ctx, const ISF::ISFParticleVector& particles,
    ISF::ISFParticleContainer& secondaries,
    McEventCollection* mcEventCollection, std::shared_ptr<HitCollectionMap> hitCollections,
    McEventCollection* shadowTruth) {

  ATH_MSG_DEBUG (name() << ".simulateVector(...) : Received a vector of " << particles.size() << " particles for simulation.");
  /** Process ParticleState from particle stack */

  bool abort = [&] ATLAS_NOT_THREAD_SAFE {
    auto eventInfo = std::make_unique<AtlasG4EventUserInfo>();
    eventInfo->SetHitCollectionMap(hitCollections);

    auto inputEvent = std::make_unique<G4Event>(ctx.eventID().event_number());
    inputEvent->SetUserInformation(eventInfo.release());

    HepMC::GenEvent* shadowGenEvent =
        (shadowTruth && !shadowTruth->empty())
            ? static_cast<HepMC::GenEvent*>(shadowTruth->back())
            : nullptr;
    m_inputConverter->ISF_to_G4Event(
        *inputEvent, particles, genEvent(mcEventCollection), shadowGenEvent);

    ATH_MSG_DEBUG("Calling ISF_Geant4 ProcessEvent");
    // Worker run manager
    // Custom class has custom method call: ProcessEvent.
    // So, grab custom singleton class directly, rather than base.
    // Maybe that should be changed! Then we can use a base pointer.
    if (m_useMT) {
#     ifdef G4MULTITHREADED
        auto* workerRM = G4AtlasWorkerRunManager::GetG4AtlasWorkerRunManager();
        return workerRM->ProcessEvent(inputEvent.release());
#     else
        ATH_MSG_ERROR("Trying to use multi-threading in non-MT build!");
        return true;
#     endif
    } else {
      auto* workerRM ATLAS_THREAD_SAFE =
          G4AtlasRunManager::GetG4AtlasRunManager();  // non-MT case
      return workerRM->ProcessEvent(inputEvent.release());
    }
  }();

  if (abort) {
    ATH_MSG_WARNING("Event was aborted !! ");
    // ATH_MSG_WARNING("Simulation will now go on to the next event ");
    // ATH_MSG_WARNING("setFilterPassed is now False");
    // setFilterPassed(false);
    return StatusCode::FAILURE;
  }

  // Get user actions that return secondaries
  auto actionsFound = m_secondaryActions.find( std::this_thread::get_id() );
  if ( actionsFound == m_secondaryActions.end() ) {
    // Get all UAs
    std::vector< G4UserSteppingAction* > allActions;
    StatusCode sc = m_userActionSvc->getSecondaryActions( allActions );
    if ( !sc.isSuccess() ) {
      ATH_MSG_ERROR( "Failed to retrieve secondaries from UASvc" );
      return sc;
    }

    // Find the UAs that can return secondaries
    for ( G4UserSteppingAction* action : allActions ) {
      passbackAction_t* castAction = dynamic_cast< passbackAction_t* >( action );
      if ( castAction ) {
        m_secondaryActions[ std::this_thread::get_id() ].push_back( castAction );
      }
    }

    actionsFound = m_secondaryActions.find( std::this_thread::get_id() );
  }

  // Retrieve secondaries from user actions
  for ( auto* action : actionsFound->second ) {
    for ( auto& parent : particles ) {

      ISF::ISFParticleContainer someSecondaries = action->ReturnSecondaries( parent );

      secondaries.splice( begin(secondaries), std::move(someSecondaries) );
    }
  }

  // const DataHandle <TrackRecordCollection> tracks;

  // StatusCode sc = evtStore()->retrieve(tracks,m_trackCollName);

  // if (sc.isFailure()) {
  //   ATH_MSG_WARNING(" Cannot retrieve TrackRecordCollection " << m_trackCollName);
  // }

  // not implemented yet... need to get particle stack from Geant4 and convert to ISFParticle
  ATH_MSG_VERBOSE( "Simulation done" );

  // Geant4 call done
  return StatusCode::SUCCESS;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::setupEvent(
    const EventContext& ctx, HitCollectionMap& hitCollections) {
  ATH_MSG_DEBUG ( "setup Event" );

  // Set the RNG to use for this event. We need to reset it for MT jobs
  // because of the mismatch between Gaudi slot-local and G4 thread-local RNG.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  G4Random::setTheEngine(rngWrapper->getEngine(ctx));

  ATH_CHECK(m_senDetTool->BeginOfAthenaEvent(hitCollections));

  m_nrOfEntries++;
  if (m_doTiming) m_eventTimer->Start();

  // make sure SD collections are properly initialized in every Athena event
  G4SDManager::GetSDMpointer()->PrepareNewEvent();

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::releaseEvent(
    const EventContext& ctx, HitCollectionMap& hitCollections) {
  ATH_MSG_DEBUG ( "release Event" );
  /** @todo : strip hits of the tracks ... */

  /* todo: ELLI: the following is copied in from the PyG4AtlasAlg:
     -> this somehow needs to be moved into C++
     and put into releaseEvent() ( or setupEvent() ?)

     from ISF_Geant4Example import AtlasG4Eng
     from ISF_Geant4Example.ISF_SimFlags import simFlags
     if self.doFirstEventG4SeedsCheck :
     if simFlags.SeedsG4.statusOn:
     rnd = AtlasG4Eng.G4Eng.menu_G4RandomNrMenu()
     rnd.set_Seed(simFlags.SeedsG4.get_Value())
     self.doFirstEventG4SeedsCheck = False
     if self.RndG4Menu.SaveStatus:
     self.RndG4Menu.Menu.saveStatus('G4Seeds.txt')
  */

  // print per-event timing info if enabled
  if (m_doTiming) {
    m_eventTimer->Stop();

    const double eventTime=m_eventTimer->GetUserElapsed()+m_eventTimer->GetSystemElapsed();
    if (m_nrOfEntries>1) {
      m_accumulatedEventTime  +=eventTime;
      m_accumulatedEventTimeSq+=eventTime*eventTime;
    }

    const float numEntriesFloat(m_nrOfEntries);
    const float avgTimePerEvent=(m_nrOfEntries>1) ? m_accumulatedEventTime/(numEntriesFloat-1.f) : eventTime;
    const float avgTimeSqPerEvent=(m_nrOfEntries>1) ? m_accumulatedEventTimeSq/(numEntriesFloat-1.f) : eventTime*eventTime;
    const float sigma=(m_nrOfEntries>2) ? std::sqrt(std::abs(avgTimeSqPerEvent - avgTimePerEvent*avgTimePerEvent)/(numEntriesFloat-2.f)) : 0;
    ATH_MSG_INFO("\t Run:Event "<<ctx.eventID().run_number()<<":"<<ctx.eventID().event_number() << "\t ("<<m_nrOfEntries<<"th event for this worker) took " << std::setprecision(4) <<
                 eventTime << " s. New average " << std::setprecision(4) <<
                 avgTimePerEvent<<" +- "<<std::setprecision(4) << sigma);
  }

  ATH_CHECK(m_senDetTool->EndOfAthenaEvent(hitCollections));
  ATH_CHECK(m_fastSimTool->EndOfAthenaEvent());

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
HepMC::GenEvent* iGeant4::G4TransportTool::genEvent(McEventCollection* mcEventCollection) const
{

  if(!mcEventCollection) {
    // retrieve McEventCollection from storegate
    if (evtStore()->contains<McEventCollection>(m_mcEventCollectionName)) {
      if (evtStore()->retrieve( mcEventCollection, m_mcEventCollectionName).isFailure()) {
        ATH_MSG_ERROR( "Unable to retrieve McEventCollection with name=" << m_mcEventCollectionName
                         << ".");
        return nullptr;
      }
      else {
        ATH_MSG_WARNING( "Fallback. Sucessfully retrieved McEventCollection with name=" << m_mcEventCollectionName);
      }
    }
    else { return nullptr; }
  }
  // collect last GenEvent from McEventCollection
  return mcEventCollection->back();
}

//________________________________________________________________________
void iGeant4::G4TransportTool::commandLog(int returnCode, const std::string& commandString) const
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

