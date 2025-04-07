/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Include files

// local
#include "ExtraParticlesPhysicsTool.h"

#include <G4VPhysicsConstructor.hh>
#include <memory>

#include "CustomParticle.hh"

// Geant4 headers
#include "G4MuIonisation.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4Transportation.hh"
#include "G4Version.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"

using namespace ExtraParticles;

//-----------------------------------------------------------------------------
// Implementation file for class : ExtraParticlesPhysicsTool
//
// August-2019 : Miha Muskinja
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
ExtraParticlesPhysicsTool::ExtraParticlesPhysicsTool(const std::string &type,
                                                     const std::string &name,
                                                     const IInterface *parent)
    : base_class(type, name, parent) {
    m_physicsOptionType = G4AtlasPhysicsOption::Type::QS_ExtraParticles;

    declareProperty("ExtraParticlesConfig", m_extraParticlesConfig);
}

//=============================================================================
// Destructor
//=============================================================================
ExtraParticlesPhysicsTool::~ExtraParticlesPhysicsTool() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode ExtraParticlesPhysicsTool::initialize() {
  ATH_MSG_DEBUG("initializing...");
  return StatusCode::SUCCESS;
}

//=============================================================================
// GetPhysicsOption
//=============================================================================
auto ExtraParticlesPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<ExtraParticlesPhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), m_extraParticlesConfig);
}

//=============================================================================
// ConstructParticle
//=============================================================================
void ExtraParticlesPhysicsTool::PhysicsConstructor::ConstructParticle() {
  ATH_MSG_DEBUG("ExtraParticlesPhysicsTool::ConstructParticle - start");
  ATH_MSG_DEBUG(
      "ExtraParticlesPhysicsTool::ConstructParticle - m_extraParticlesConfig = "
      << m_extraParticlesConfig);

  // the existing particle table
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  for (const auto &particle : m_extraParticlesConfig) {

    G4String name = particle.first;
    G4double mass = particle.second[0];
    G4double width = particle.second[1];
    G4int charge = particle.second[2];
    G4int pdg = particle.second[3];
    G4double lifetime = particle.second[4];
    G4bool stable = false;

    // don't add if the particle already exists
    if (theParticleTable->FindParticle(pdg)) {
      ATH_MSG_DEBUG("Skipping "
                    << theParticleTable->FindParticle(pdg)->GetParticleName()
                    << " (" << pdg
                    << ") as it is already in the ParticleTable.");
      continue;
    }

    // printout
    ATH_MSG_DEBUG("Adding: " << name << " " << pdg << " " << charge << " "
                             << mass << " " << width << " " << lifetime);

    // create the new particle
    m_extraParticles.insert(
        new CustomParticle(name, mass, width, charge, pdg, stable, lifetime));
  }
  ATH_MSG_DEBUG("ExtraParticlesPhysicsTool::ConstructParticle - end");
}

//=============================================================================
// ConstructProcess
//=============================================================================
void ExtraParticlesPhysicsTool::PhysicsConstructor::ConstructProcess() {
  ATH_MSG_DEBUG("ExtraParticlesPhysicsTool::ConstructProcess - start");
  if (msgLvl(MSG::DEBUG)) {
    std::vector<std::string> extraParticleNames;
    for ( G4ParticleDefinition* extraParticle : m_extraParticles) {
      extraParticleNames.push_back(extraParticle->GetParticleName());
    }
    ATH_MSG_DEBUG("ExtraParticlesPhysicsTool::ConstructProcess - m_extraParticleNames = " << extraParticleNames);
  }
  for (auto *particle : m_extraParticles) {
    if (particle->GetPDGCharge() != 0) {
      ATH_MSG_DEBUG("Adding EM processes for "
                    << particle->GetParticleName());
      G4ProcessManager *proc = particle->GetProcessManager();
      proc->AddProcess(new G4hMultipleScattering, -1, 1, 1);
      proc->AddProcess(new G4hIonisation, -1, 2, 2);
    }
  }
  ATH_MSG_DEBUG("ExtraParticlesPhysicsTool::ConstructProcess - end");
}
