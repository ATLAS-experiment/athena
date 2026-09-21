/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NoG4PhysicsTool.h"

#include "G4DummyModel.hh"
#include "G4EmConfigurator.hh"
#include "G4Exception.hh"
#include "G4LossTableManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4VEmProcess.hh"
#include "G4VEnergyLossProcess.hh"
#include "G4VMultipleScattering.hh"

#include <cfloat>
#include <set>

NoG4PhysicsTool::NoG4PhysicsTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
}

auto NoG4PhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<NoG4PhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), m_regionNames.value());
}

NoG4PhysicsTool::PhysicsConstructor::PhysicsConstructor(
    const std::string& name, MSG::Level level,
    const std::vector<std::string>& regionNames)
    : IPhysicsContructor(name, level), m_regionNames(regionNames) {}

void NoG4PhysicsTool::PhysicsConstructor::ConstructParticle() {}

void NoG4PhysicsTool::PhysicsConstructor::ConstructProcess() {
  // Physics options are constructed after the reference physics list, so every
  // process is in place by now and the ones to switch off can be collected from
  // the particle table. Only the electromagnetic ones take a model per region.
  std::set<std::string> processNames;

  G4ParticleTable::G4PTblDicIterator* particles =
      G4ParticleTable::GetParticleTable()->GetIterator();
  particles->reset();
  while ((*particles)()) {
    const G4ParticleDefinition* particle = particles->value();
    G4ProcessManager* processManager = particle->GetProcessManager();
    if (!processManager) continue;

    G4ProcessVector* processes = processManager->GetProcessList();
    for (std::size_t i = 0; processes && i < processes->size(); ++i) {
      G4VProcess* process = (*processes)[i];
      if (!process) continue;
      if (process->GetProcessName() == "GammaGeneralProc") {
        G4Exception("NoG4PhysicsTool", "GammaGeneralProcessOn", FatalException,
                    "G4GammaGeneralProcess holds all photon processes in one, so they "
                    "cannot be switched off per region: disable it.");
      }
      if (dynamic_cast<G4VEmProcess*>(process) ||
          dynamic_cast<G4VEnergyLossProcess*>(process) ||
          dynamic_cast<G4VMultipleScattering*>(process)) {
        processNames.insert(process->GetProcessName());
      }
    }
  }

  // G4DummyModel has no cross section and no stopping power, and is itself a
  // G4VMscModel, so the one model covers the discrete, the energy loss and the
  // multiple scattering processes alike. The configurator is per thread and
  // only adds the models when the processes prepare their physics tables, so
  // the order with respect to the EM physics constructor does not matter.
  // "all" matches every particle that carries the process.
  G4EmConfigurator* configurator = G4LossTableManager::Instance()->EmConfigurator();
  for (const std::string& region : m_regionNames) {
    for (const std::string& process : processNames) {
      ATH_MSG_INFO("Switching off " << process << " in region " << region);
      configurator->SetExtraEmModel("all", process, new G4DummyModel(), region, 0., DBL_MAX);
    }
  }
}
