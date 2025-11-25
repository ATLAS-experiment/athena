/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "FastSimulationConstructorTool.h"

// G4 headers
#include "G4FastSimulationManagerProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FastSimulationConstructorTool::FastSimulationConstructorTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::UnknownType;
}

//=============================================================================
// Initialize
//=============================================================================
StatusCode FastSimulationConstructorTool::initialize() {
  ATH_MSG_VERBOSE("FastSimulationConstructorTool initialize(  )");

  return StatusCode::SUCCESS;
}

auto FastSimulationConstructorTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<FastSimulationConstructorTool::PhysicsConstructor>(
      m_initializeFastSimulation, name(), this->msgLevel());
}

//=============================================================================
// Physics Constructor implementation
//=============================================================================

FastSimulationConstructorTool::PhysicsConstructor::PhysicsConstructor(
    bool initializeFastSimulation, const std::string& name, MSG::Level level)
    : IPhysicsContructor(name, level), m_initializeFastSimulation(initializeFastSimulation) {}

void FastSimulationConstructorTool::PhysicsConstructor::ConstructParticle() {}

void FastSimulationConstructorTool::PhysicsConstructor::ConstructProcess() {

  if(!m_initializeFastSimulation) {
    ATH_MSG_INFO("Fast simulation initialization flag is set to false. Skipping fast simulation setup.");
    return;
  }

  ATH_MSG_INFO("ConstructProcess for FastSimulation being run");
  // Enable fast simulation processes for all particle types
  G4FastSimulationManagerProcess* fastSimManagerProcess =
      new G4FastSimulationManagerProcess;
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator* theParticleIterator =
      theParticleTable->GetIterator();

  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    // TOD: magic numbers?
    pmanager->AddProcess(fastSimManagerProcess, -1, 1, 1);
  }
}
