/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4ATLASUSERWORKERINITIALIZATION_H
#define G4ATLASALG_G4ATLASUSERWORKERINITIALIZATION_H

#include <G4FastSimulationManagerProcess.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ProcessManager.hh>
#include <G4UserWorkerInitialization.hh>

/// @brief ATLAS custom class for worker initialization functionality
///
/// @author Julien Esseiva <julien.esseiva@cern.ch>
///
class G4AtlasUserWorkerInitialization : public G4UserWorkerInitialization {
public:
  struct Config {
    bool m_activateFastSimulation{false}; ///< Activate fast simulation processes
  };

  explicit G4AtlasUserWorkerInitialization(const Config& config)
      : G4UserWorkerInitialization(), m_config(config) {}

  void WorkerRunStart() const override
  {
    if(!m_config.m_activateFastSimulation)
      return;
    // Enable fast simulation processes for all particle types
    // Initialized here because it needs to happen after Geant4 physics initialization to access the particle table
    G4FastSimulationManagerProcess* fastSimManagerProcess = new G4FastSimulationManagerProcess;
    G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
    G4ParticleTable::G4PTblDicIterator* theParticleIterator = theParticleTable->GetIterator();

    theParticleIterator->reset();
    while( (*theParticleIterator)() ){
      G4ParticleDefinition* particle = theParticleIterator->value();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      pmanager->AddProcess(fastSimManagerProcess, -1, 1, 1);
    }
  }

private:
  Config m_config;
};
#endif
