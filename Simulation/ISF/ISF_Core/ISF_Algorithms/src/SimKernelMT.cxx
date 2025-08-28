/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Elmar Ritsch <Elmar.Ritsch@cern.ch>
 * @date October 2016
 * @brief Core Athena algorithm for the Integrated Simulation Framework
 */

#include "SimKernelMT.h"

// ISF includes
#include "AtlasDetDescr/AtlasRegionHelper.h"
#include "HitManagement/HitCollectionMap.h"
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleOrderedQueue.h"
#include "ISF_Interfaces/BaseSimulatorG4Tool.h"

// STL
#include <memory>
#include <queue>
#include <utility>

#undef ISFDEBUG

ISF::SimKernelMT::SimKernelMT( const std::string& name, ISvcLocator* pSvcLocator ) :
    ::AthAlgorithm( name, pSvcLocator ),
    m_simSelectors() //FIXME make private
{
    // routing tools
    declareProperty("BeamPipeSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasForward] );
    declareProperty("IDSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasID] );
    declareProperty("CaloSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasCalo] );
    declareProperty("MSSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasMS] );
    declareProperty("CavernSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasCavern] );
    // tuning parameters
    declareProperty("MaximumParticleVectorSize"  , m_maxParticleVectorSize             );
}


ISF::SimKernelMT::~SimKernelMT() {
}


StatusCode ISF::SimKernelMT::initialize() {

  ATH_CHECK( m_simulationTools.retrieve() );
  for ( auto& curSimTool : m_simulationTools )
  {
    if ( curSimTool )
    {
      const auto flavor = curSimTool->simFlavor();
      auto itr = m_simToolMap.find(flavor);
      if (itr != m_simToolMap.end() )
      {
        ATH_MSG_FATAL("Two ISimulatorTool instances (" << itr->second->name() << "," << curSimTool->name() << ") with the same flavor in this job!\n Check your configuration!");
        return StatusCode::FAILURE;
      }
      // New flavour add it to the map.
      m_simToolMap[flavor] = &*curSimTool;
      if (flavor == ISF::ParticleKiller) {
        m_particleKillerTool = &*curSimTool;
      }
    }
  }
  // Check that a particle killer simulator tool was provided
  if (!m_particleKillerTool) {
    ATH_MSG_FATAL("No fallback ParticleKiller Simulator Tool provided in SimulationTools, the job will bail out now.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("The following Simulators will be used in this job: \t" << m_simulationTools);
  // retrieve simulation selectors (i.e. the "routing chain")
  for ( auto& selectorsToolHandleArray: m_simSelectors )
  {
    ATH_CHECK( selectorsToolHandleArray.retrieve() );
  }

  // info screen output
  ATH_MSG_INFO( "The following routing chains are defined:" );
  for ( short geoID=AtlasDetDescr::fFirstAtlasRegion; geoID<AtlasDetDescr::fNumAtlasRegions; ++geoID )
  {
    auto& localSelectors = m_simSelectors[geoID];
    ATH_MSG_INFO( AtlasDetDescr::AtlasRegionHelper::getName(geoID) << " (GeoID=" << geoID << "): \t" << localSelectors );

    for ( auto& selector : localSelectors )
    {
      const auto flavor = selector->simFlavor();
      auto itr = m_simToolMap.find(flavor);
      if (itr == m_simToolMap.end() )
      {
        ATH_MSG_WARNING( "Map from SimulationFlavor to SimulatorTool:" );
        for ( auto& entry : m_simToolMap )
        {
          ATH_MSG_WARNING( "SimulationFlavor: " << entry.first << ", SimulatorTool Name: " << entry.second->name() );
        }
        ATH_MSG_FATAL( "No SimulationTool with flavor " << flavor << " expected by " << selector->name() << " found in this job!\n Check your configuration!" );
        return StatusCode::FAILURE;
      }
    }
  }

  ATH_CHECK( m_entryLayerTool.retrieve() );

  if ( not m_orderingTool.empty() ) {
    ATH_CHECK( m_orderingTool.retrieve() );
  }

  ATH_CHECK( m_inputEvgenKey.initialize() );
  ATH_CHECK( m_outputTruthKey.initialize() );

  ATH_CHECK( m_caloEntryLayerKey.initialize() );
  ATH_CHECK( m_muonEntryLayerKey.initialize() );
  ATH_CHECK( m_muonExitLayerKey.initialize() );

  ATH_CHECK( m_inputConverter.retrieve() );
  if ( not m_truthPreselectionTool.empty() ) {
    ATH_CHECK(m_truthPreselectionTool.retrieve());
  }

  ATH_CHECK ( m_truthRecordSvc.retrieve() );

  if ( not m_qspatcher.empty() ) {
    ATH_CHECK( m_qspatcher.retrieve() );
  }

  ATH_CHECK( m_geoIDSvc.retrieve() );

  return StatusCode::SUCCESS;
}


StatusCode ISF::SimKernelMT::execute() {

  const EventContext& ctx = Gaudi::Hive::currentContext();
  auto hitCollections = std::make_shared<HitCollectionMap>();
  // Call setupEvent for all simulators (TODO: make the tools do this)
  for (auto& curSimTool: m_simulationTools) {
    if ( curSimTool ) {
      if (auto* g4sim =
              dynamic_cast<ISF::BaseSimulatorG4Tool*>(curSimTool.get())) {
        ATH_CHECK(g4sim->setupEvent(ctx, *hitCollections));
      } else {
        ATH_CHECK(curSimTool->setupEvent(ctx));
      }
      ATH_MSG_DEBUG( "Event setup done for " << curSimTool->name() );
    }
  }

  SG::ReadHandle<McEventCollection> inputEvgen(m_inputEvgenKey);
  if (!inputEvgen.isValid()) {
    ATH_MSG_FATAL("Unable to read input GenEvent collection '" << inputEvgen.key() << "'");
    return StatusCode::FAILURE;
  }

  // create output Truth collection
  SG::WriteHandle<McEventCollection> outputTruth(m_outputTruthKey, ctx);
  std::unique_ptr<McEventCollection> shadowTruth{};
  if (m_useShadowEvent) {
    outputTruth = std::make_unique<McEventCollection>();
    // copy input Evgen collection to shadow Truth collection
    shadowTruth = std::make_unique<McEventCollection>(*inputEvgen);
    for (HepMC::GenEvent* currentGenEvent : *shadowTruth ) {
      // Apply QS patch if required
      if ( not m_qspatcher.empty() ) {
        ATH_CHECK(m_qspatcher->applyWorkaround(*currentGenEvent));
      }
      // Copy GenEvent and remove daughters of quasi-stable particles to be simulated
      std::unique_ptr<HepMC::GenEvent> outputEvent = m_truthPreselectionTool->filterGenEvent(*currentGenEvent);
      outputTruth->push_back(outputEvent.release());
    }
  }
  else {
    // copy input Evgen collection to output Truth collection
    outputTruth = std::make_unique<McEventCollection>(*inputEvgen);
    // Apply QS patch if required
    if ( not m_qspatcher.empty() ) {
      for (HepMC::GenEvent* currentGenEvent : *outputTruth ) {
        ATH_CHECK(m_qspatcher->applyWorkaround(*currentGenEvent));
      }
    }
  }

  const int largestGeneratedParticleBC = (outputTruth->empty()) ? HepMC::UNDEFINED_ID
    : HepMC::maxGeneratedParticleBarcode(outputTruth->at(0)); // TODO make this more robust
  const int largestGeneratedVertexBC =  (outputTruth->empty()) ? HepMC::UNDEFINED_ID
    : HepMC::maxGeneratedVertexBarcode(outputTruth->at(0)); // TODO make this more robust
  // tell TruthService we're starting a new event
  ATH_CHECK( m_truthRecordSvc->initializeTruthCollection(largestGeneratedParticleBC, largestGeneratedVertexBC) );

  // Create TrackRecordCollections and pass them to the entryLayerTool
  SG::WriteHandle<TrackRecordCollection> caloEntryLayer(m_caloEntryLayerKey, ctx);
  caloEntryLayer = std::make_unique<TrackRecordCollection>(caloEntryLayer.name());
  ATH_CHECK(m_entryLayerTool->registerTrackRecordCollection(caloEntryLayer.ptr(), fAtlasCaloEntry));
  SG::WriteHandle<TrackRecordCollection> muonEntryLayer(m_muonEntryLayerKey, ctx);
  muonEntryLayer = std::make_unique<TrackRecordCollection>(muonEntryLayer.name());
  ATH_CHECK(m_entryLayerTool->registerTrackRecordCollection(muonEntryLayer.ptr(), fAtlasMuonEntry));
  SG::WriteHandle<TrackRecordCollection> muonExitLayer(m_muonExitLayerKey, ctx);
  muonExitLayer = std::make_unique<TrackRecordCollection>(muonExitLayer.name());
  ATH_CHECK(m_entryLayerTool->registerTrackRecordCollection(muonExitLayer.ptr(), fAtlasMuonExit));

  // read and convert input
  ISFParticleContainer simParticles;
  ATH_CHECK( m_inputConverter->convert(*outputTruth, simParticles) );

  // create an ordered queue of particles
  ISFParticleOrderedQueue particleQueue;
  for ( auto& particle : simParticles ) {

    if ( m_orderingTool.empty() ) {
      // Without a defined ordering, preserve old FIFO behaviour
      particle->setOrder( -particleQueue.size() );
    } else {
      m_orderingTool->setOrder( *particle );
    }

    particleQueue.push( particle );
  }

  unsigned int loopCounter{0};
  // loop until there are no more particles to simulate
  ISF::ISFParticleVector particles{};
  ISimulatorTool* lastSimulator{};
  ISFParticleContainer newSecondaries{};
  while ( particleQueue.size() ) {
    ++loopCounter;
    ATH_MSG_VERBOSE("Main Loop pass no. " << loopCounter);
    ATH_MSG_VERBOSE("Queue starts with " << particleQueue.size() << " particles.");
    // Create a vector of particles with the same simulator
    ISFParticleOrderedQueue tempQueue;
    while ( particleQueue.size() ) {
      auto particlePtr = particleQueue.top();
      ISFParticle& curParticle( *particlePtr );
      particleQueue.pop();

      // Get the geo ID for the particle
      if ( m_forceGeoIDSvc || !validAtlasRegion( curParticle.nextGeoID() ) ) {
        m_geoIDSvc->identifyAndRegNextGeoID( curParticle );
      }

      // Get the simulator using the GeoID
      auto& simTool = identifySimulator(curParticle);

      // Fill the vector
      if ( particles.empty() ) {
        // First particle in the vector defines the simulator
        particles.push_back(particlePtr);
        lastSimulator=&simTool;
      }
      else if (&simTool!=lastSimulator || particles.size() >= m_maxParticleVectorSize ) {
        // Change of simulator, end the current vector
        tempQueue.push(particlePtr);
      }
      else {
        particles.push_back(particlePtr);
      }
    }
    particleQueue = std::move(tempQueue);
    #ifdef ISFDEBUG
    if (loopCounter>100 && particles.size()<3) {
      ATH_MSG_INFO("Main Loop pass no. " << loopCounter);
      ATH_MSG_INFO("Selected " << particles.size() << " particles to be processed by " << lastSimulator->name());
      for ( const ISFParticle *particle : particles ) {
        ATH_MSG_INFO(*particle);
      }
    }
    #endif // ISFDEBUG

    ATH_MSG_VERBOSE("Selected " << particles.size() << " particles to be processed by " << lastSimulator->name());
    // Run the simulation
    if (auto* g4sim = dynamic_cast<ISF::BaseSimulatorG4Tool*>(lastSimulator)) {
      ATH_CHECK(g4sim->simulateVector(ctx, particles, newSecondaries,
                                      outputTruth.ptr(), hitCollections,
                                      shadowTruth.get()));
    } else {
      ATH_CHECK(lastSimulator->simulateVector(ctx, particles, newSecondaries,
                                              outputTruth.ptr(),
                                              shadowTruth.get()));
    }

    ATH_MSG_VERBOSE(lastSimulator->name() << " returned " << newSecondaries.size() << " new particles to be added to the queue." );
    // Register returned particles with the entry layer tool, set their order and enqueue them
    for ( auto* secondary : newSecondaries ) {

      // Set up particle in ISF
      m_entryLayerTool->registerParticle( *secondary );
      if ( m_forceGeoIDSvc || !validAtlasRegion( secondary->nextGeoID() ) ) {
        m_geoIDSvc->identifyAndRegNextGeoID( *secondary );
      }

      if ( m_orderingTool.empty() ) {
        // Without a defined ordering, preserve old FIFO behaviour
        secondary->setOrder( -particleQueue.size() );
      } else {
        m_orderingTool->setOrder( *secondary );
      }

      particleQueue.push( secondary );
    }
    newSecondaries.clear();

    // Delete simulated particles
    for ( auto usedParticle : particles ) {
      delete usedParticle;
    }
    particles.clear();
  }
  ATH_MSG_VERBOSE("Final status: queue contains " << particleQueue.size() << " particles.");

  // Release the event from all simulators (TODO: make the tools do this)
  for (auto& curSimTool: m_simulationTools) {
    if ( curSimTool ) {
      if (auto* g4sim =
              dynamic_cast<ISF::BaseSimulatorG4Tool*>(curSimTool.get())) {
        ATH_CHECK(g4sim->releaseEvent(ctx, *hitCollections));
      } else {
        ATH_CHECK(curSimTool->releaseEvent(ctx));
      }
      ATH_MSG_DEBUG( "releaseEvent() completed for " << curSimTool->name() );
    }
  }

  // Remove QS patch if required
  if(!m_qspatcher.empty()) {
    for (HepMC::GenEvent* currentGenEvent : *outputTruth ) {
      ATH_CHECK(m_qspatcher->removeWorkaround(*currentGenEvent));
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode ISF::SimKernelMT::finalize() {
  return StatusCode::SUCCESS;
}


/// Returns the simulator to use for the given particle
ISF::ISimulatorTool& ISF::SimKernelMT::identifySimulator(const ISF::ISFParticle& particle) {
  AtlasDetDescr::AtlasRegion geoID = particle.nextGeoID();

  auto& localSelectors = m_simSelectors[geoID];
  for (auto& selector: localSelectors) {
    bool selected = selector->selfSelect(particle);
    if (selected) {
      return *m_simToolMap.at(selector->simFlavor());
    }
  }

  ATH_MSG_WARNING("No simulator found for particle (" << particle << ")."
                  << " Will send it to " << m_particleKillerTool->name());
  return *m_particleKillerTool;
}
