/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Include files

// local
#include "G4EMProcessesPhysicsTool.h"

// Geant4 headers
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"
#include <cmath>

//-----------------------------------------------------------------------------
// Implementation file for class : G4EMProcessesPhysicsTool
//
// August-2020 : Miha Muskinja
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
G4EMProcessesPhysicsTool::G4EMProcessesPhysicsTool(const std::string &type,
                                                   const std::string &name,
                                                   const IInterface *parent)
  : base_class(type, name, parent)
{
  m_physicsOptionType = G4AtlasPhysicsOption::Type::QS_ExtraProc;

  declareProperty("ParticleList", m_particleList);
}

//=============================================================================
// Initialize
//=============================================================================
StatusCode G4EMProcessesPhysicsTool::initialize()
{
  ATH_MSG_DEBUG("initializing...");
  return StatusCode::SUCCESS;
}

//=============================================================================
// GetPhysicsOption
//=============================================================================
auto G4EMProcessesPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<G4EMProcessesPhysicsTool::PhysicsConstructor>(
      name(), msgLevel(), m_particleList);
}

//=============================================================================
// ConstructParticle
//=============================================================================
void G4EMProcessesPhysicsTool::PhysicsConstructor::ConstructParticle() {}

//=============================================================================
// ConstructProcess
//=============================================================================
void G4EMProcessesPhysicsTool::PhysicsConstructor::ConstructProcess() {
  ATH_MSG_DEBUG("G4EMProcessesPhysicsTool::ConstructProcess() - start");
  ATH_MSG_DEBUG("G4EMProcessesPhysicsTool::ConstructProcess() - m_particleList = " << m_particleList);
  G4ParticleTable::G4PTblDicIterator *particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
  particleIterator->reset();

  while ((*particleIterator)())
    {
      G4ParticleDefinition *particle = particleIterator->value();
      if (std::find(m_particleList.begin(), m_particleList.end(), std::abs(particle->GetPDGEncoding())) != m_particleList.end())
        {
          ATH_MSG_DEBUG("Adding EM processes for "
                         << particle->GetParticleName());
          G4ProcessManager *proc = particle->GetProcessManager();
          proc->AddProcess(new G4hMultipleScattering, -1, 1, 1);
          proc->AddProcess(new G4hIonisation, -1, 2, 2);
        }
    } // End of the particle iterator
  ATH_MSG_DEBUG("G4EMProcessesPhysicsTool::ConstructProcess() - end");
}
