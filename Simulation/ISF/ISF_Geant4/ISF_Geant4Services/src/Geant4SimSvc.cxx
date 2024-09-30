/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "Geant4SimSvc.h"

/** Constructor **/
iGeant4::Geant4SimSvc::Geant4SimSvc(const std::string& name,ISvcLocator* svc) :
  BaseSimulationSvc(name, svc)
{
}

iGeant4::Geant4SimSvc::~Geant4SimSvc()
{}

/** framework methods */
StatusCode iGeant4::Geant4SimSvc::initialize()
{
  ATH_CHECK (m_simulatorTool.retrieve());
  return StatusCode::SUCCESS;
}

/** framework methods */
StatusCode iGeant4::Geant4SimSvc::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode iGeant4::Geant4SimSvc::setupEvent()
{
  return m_simulatorTool->setupEventST();
}

StatusCode iGeant4::Geant4SimSvc::releaseEvent()
{
  return m_simulatorTool->releaseEventST();
}

/** Simulation Call */
StatusCode iGeant4::Geant4SimSvc::simulate(ISF::ISFParticle& isp, McEventCollection* mcEventCollection)
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ISF::ISFParticleContainer secondaries; // filled, but not used
  ATH_CHECK(m_simulatorTool->simulate(ctx, isp, secondaries, mcEventCollection));
  return StatusCode::SUCCESS;
}

/** Simulation Call */
StatusCode iGeant4::Geant4SimSvc::simulateVector(const ISF::ISFParticleVector& particles, McEventCollection* mcEventCollection, McEventCollection *shadowTruth)
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ISF::ISFParticleContainer secondaries; // filled, but not used
  ATH_CHECK (m_simulatorTool->simulateVector(ctx, particles,secondaries, mcEventCollection, shadowTruth));
  return StatusCode::SUCCESS;
}

