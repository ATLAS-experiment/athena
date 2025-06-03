/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "Geant4SimSvc.h"

#include "HitManagement/HitCollectionMap.h"

/** Constructor **/
iGeant4::Geant4SimSvc::Geant4SimSvc(const std::string& name, ISvcLocator* svc)
    : BaseSimulationG4Svc(name, svc) {}

iGeant4::Geant4SimSvc::~Geant4SimSvc()
{}

/** framework methods */
StatusCode iGeant4::Geant4SimSvc::initialize()
{
  ATH_CHECK (m_simulatorTool.retrieve());
  m_simulatorG4Tool =
      dynamic_cast<ISF::BaseSimulatorG4Tool*>(m_simulatorTool.get());
  if (!m_simulatorG4Tool) {
    ATH_MSG_FATAL("SimulatorTool is not of type ISF::BaseSimulatorG4Tool");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/** framework methods */
StatusCode iGeant4::Geant4SimSvc::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode iGeant4::Geant4SimSvc::setupEvent(HitCollectionMap& hitCollections) {
  return m_simulatorG4Tool->setupEventST(hitCollections);
}

StatusCode iGeant4::Geant4SimSvc::releaseEvent(
    HitCollectionMap& hitCollections) {
  return m_simulatorG4Tool->releaseEventST(hitCollections);
}

/** Simulation Call */
StatusCode iGeant4::Geant4SimSvc::simulate(ISF::ISFParticle& isp,
                                           McEventCollection* mcEventCollection,
                                           std::shared_ptr<HitCollectionMap> hitCollections) {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ISF::ISFParticleContainer secondaries; // filled, but not used
  ATH_CHECK(m_simulatorG4Tool->simulate(ctx, isp, secondaries,
                                        mcEventCollection, hitCollections));
  return StatusCode::SUCCESS;
}

/** Simulation Call */
StatusCode iGeant4::Geant4SimSvc::simulateVector(
    const ISF::ISFParticleVector& particles,
    McEventCollection* mcEventCollection, std::shared_ptr<HitCollectionMap> hitCollections,
    McEventCollection* shadowTruth) {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ISF::ISFParticleContainer secondaries; // filled, but not used
  ATH_CHECK(m_simulatorG4Tool->simulateVector(
      ctx, particles, secondaries, mcEventCollection, hitCollections, shadowTruth));
  return StatusCode::SUCCESS;
}
