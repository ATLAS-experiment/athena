/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./ParticleKillerSimTool.h"

ISF::ParticleKillerSimTool::ParticleKillerSimTool(const std::string& type, const std::string& name,  const IInterface* parent)
  : BaseSimulatorTool(type, name, parent)
{
}

ISF::ParticleKillerSimTool::~ParticleKillerSimTool() {

}

StatusCode ISF::ParticleKillerSimTool::initialize() {
  return StatusCode::SUCCESS;
}

StatusCode ISF::ParticleKillerSimTool::simulate(const EventContext&, ISFParticle& isp, ISFParticleContainer&, McEventCollection* ) {

  // give a screen output that you entered ParticleKillerSimSvc
  ATH_MSG_VERBOSE( "Particle '" << isp << "' received for simulation." );
  ATH_MSG_VERBOSE( "Killing this particle, since this"
                   << "is the soul purpose of this 'simulation engine'." );

  // particle 'simulation' done
  // (memory management, ie delete, of the ISFParticle is done inside the ISFKernel)
  return StatusCode::SUCCESS;
}

StatusCode ISF::ParticleKillerSimTool::simulateVector(const EventContext& ctx, const ISFParticleVector& particles, ISFParticleContainer& secondaries, McEventCollection* mcEventCollection, McEventCollection *) {
  for (auto isp : particles) {
    ATH_CHECK(simulate(ctx, *isp, secondaries, mcEventCollection));
  }
  return StatusCode::SUCCESS;
}
