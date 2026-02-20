/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "G4RunAlg.h"

// Geant4 includes
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4GDMLParser.hh"
#include "G4TrackingManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4GeometryManager.hh"

// Athena includes
#include "AthenaKernel/RNGWrapper.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"
#include "HitManagement/HitCollectionMap.h"
#include "MCTruthBase/TruthStrategyManager.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// standard library
#include <memory>
#include <mutex>

static std::once_flag releaseGeoModelOnceFlag;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode G4RunAlg::initialize ATLAS_NOT_THREAD_SAFE ()
{
  ATH_MSG_DEBUG("Start of G4RunAlg::initialize()");

  // Read the simplified geometry for FastCaloSim track transportation if requested
  // TODO: should be moved to Geant4 main thread (detector construction)
  if(!m_simplifiedGeoPath.empty()) {
    std::string geoFile = PathResolverFindCalibFile(m_simplifiedGeoPath);
    if (geoFile.empty()) {
      ATH_MSG_FATAL("Could not find simplified geometry file: " << m_simplifiedGeoPath);
      return StatusCode::FAILURE;
    }
    G4GDMLParser parser;
    parser.Read(geoFile, false);
  }

  // Truth services
  ATH_CHECK(m_truthRecordSvc.retrieve());
  ATH_MSG_INFO("- Using ISF TruthRecordSvc : " << m_truthRecordSvc.typeAndName());
  ATH_CHECK(m_geoIDSvc.retrieve());
  ATH_MSG_INFO("- Using ISF GeoIDSvc       : " << m_geoIDSvc.typeAndName());

  TruthStrategyManager& sManager = TruthStrategyManager::GetStrategyManager_nc();
  sManager.SetISFTruthSvc(&(*m_truthRecordSvc));
  sManager.SetISFGeoIDSvc(&(*m_geoIDSvc));

  // Retrieve the G4RunTool. This will start the G4 main thread
  ATH_CHECK(m_g4RunTool.retrieve());
  ATH_MSG_INFO("Waiting on G4RunTool to be ready for run");
  // We have to wait on the Geant4 main thread to finish initializing.
  // Wait has to be done here because Gaudi tool initialization are protected by a recursive mutex
  // which would lead to a deadlock between the Geant4 main thread and the Athena thread
  m_g4RunTool->WaitBeginRun();
  
  // Initialize algorithm-specific services
  ATH_CHECK(m_rndmGenSvc.retrieve());
  ATH_CHECK(m_userActionSvc.retrieve());
  // These have already been initialized by G4RunTool, just populate the handles
  ATH_CHECK(m_senDetTool.retrieve());
  ATH_CHECK(m_fastSimTool.retrieve());

  // I/O
  ATH_CHECK(m_inputTruthCollectionKey.initialize());
  ATH_CHECK(m_outputTruthCollectionKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  ATH_CHECK(m_inputConverter.retrieve());
  if (!m_truthPreselectionTool.empty()) {
    ATH_CHECK(m_truthPreselectionTool.retrieve());
  }

  if (!m_qspatcher.empty()) {
    ATH_CHECK(m_qspatcher.retrieve());
  }

  ATH_MSG_DEBUG("End of G4RunAlg::initialize()");
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode G4RunAlg::execute()
{
  static std::atomic<unsigned int> n_Event=0;
  ATH_MSG_DEBUG("++++++++++++  G4RunAlg execute  ++++++++++++");

  n_Event += 1;

  if (n_Event<=10 || (n_Event%100) == 0) {
    ATH_MSG_ALWAYS("Event num. "  << n_Event << " start processing");
  }

  // Release GeoModel Geometry if necessary
  // TODO: should be moved to Geant4 main thread after run initialization
  if (m_releaseGeoModel) {
    try {
      std::call_once(releaseGeoModelOnceFlag, &G4RunAlg::releaseGeoModel, this);
    }
    catch(const std::exception& e) {
      ATH_MSG_ERROR("Failure in G4RunAlg::releaseGeoModel: " << e.what());
      return StatusCode::FAILURE;
    }
  }

  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Set the RNG to use for this event. We need to reset it for MT jobs
  // because of the mismatch between Gaudi slot-local and G4 thread-local RNG.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmGenSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName,  ctx);

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
  {

    // called by the Geant4 PrimaryGeneratorAction because primary vertices must be instantiated by Geant4 threads
    auto prepare_event = [this, &outputTruthCollection, &shadowTruth, largestGeneratedParticleBC, largestGeneratedVertexBC](G4Event& event, std::unique_ptr<AtlasG4SyncEventUserInfo> g4eventInfo) -> StatusCode {
      // tell TruthService we're starting a new event
      ATH_CHECK( m_truthRecordSvc->initializeTruthCollection(largestGeneratedParticleBC, largestGeneratedVertexBC) );
      event.SetEventID(g4eventInfo->AthenaEventID());
      event.SetUserInformation(g4eventInfo.release());
      ATH_CHECK(m_inputConverter->convertHepMCToG4Event(
          *outputTruthCollection, event, *shadowTruth));
      return StatusCode::SUCCESS;
    };
    
    auto eventInfo = std::make_unique<AtlasG4SyncEventUserInfo>(rngWrapper->getEngine(ctx), std::move(prepare_event), ctx);

    // get a shared pointer to the hit collection map because we will need it after the G4Event is destroyed
    std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();

    ATH_CHECK(m_senDetTool->BeginOfAthenaEvent(*hitCollections));
    ATH_CHECK(m_userActionSvc->BeginOfAthenaEvent(*hitCollections));
    ATH_CHECK(m_fastSimTool->BeginOfAthenaEvent());

    auto syncInterface = eventInfo->SyncInterface();

    ATH_MSG_DEBUG("Pushing Athena event " << ctx.eventID().event_number() << " onto event buffer");
    m_g4RunTool->PushEvent(std::move(eventInfo));
    ATH_MSG_DEBUG("Buffer size=" << m_g4RunTool->Size() << ", waiting for event to finish");
    //G4 should tell Athena in an EndOfEventAction that the simulation of the event is done
    syncInterface->WaitStatusDone();

    if (syncInterface->EventAborted()) {
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
    ATH_CHECK(m_userActionSvc->EndOfAthenaEvent(*hitCollections));
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

void G4RunAlg::releaseGeoModel()
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
