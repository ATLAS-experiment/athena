/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FatrasG4PhysicsTool.h"

#include "G4DummyModel.hh"
#include "G4PairProductionRelModel.hh"
#include "G4EmConfigurator.hh"
#include "G4LossTableManager.hh"
#include "G4Exception.hh"
#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"

#include <algorithm>

FatrasG4PhysicsTool::FatrasG4PhysicsTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
}

auto FatrasG4PhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<FatrasG4PhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), m_regionNames.value(), m_minEnergy.value(), m_maxEnergy.value());
}

FatrasG4PhysicsTool::PhysicsConstructor::PhysicsConstructor(
    const std::string& name, MSG::Level level,
    const std::vector<std::string>& regionNames,
    double minEnergy, double maxEnergy)
    : IPhysicsContructor(name, level),
      m_regionNames(regionNames),
      m_minEnergy(minEnergy),
      m_maxEnergy(maxEnergy) {}

void FatrasG4PhysicsTool::PhysicsConstructor::ConstructParticle() {}

void FatrasG4PhysicsTool::PhysicsConstructor::ConstructProcess() {
  // The configurator is per thread and only adds the models when the
  // processes prepare their physics tables, so the order with respect to the
  // EM physics constructor does not matter. A region with models of its own
  // uses only those, so the energies around the window get the model
  // G4GammaConversion uses everywhere in this Geant4 version.
  G4EmConfigurator* configurator = G4LossTableManager::Instance()->EmConfigurator();
  for (const std::string& region : m_regionNames) {
    ATH_MSG_DEBUG("Switching off gamma conversion in region " << region << " between "
                  << m_minEnergy << " and " << m_maxEnergy << " MeV");
    configurator->SetExtraEmModel("gamma", "conv", new G4PairProductionRelModel(), region, 0., m_minEnergy);
    configurator->SetExtraEmModel("gamma", "conv", new G4DummyModel(), region, m_minEnergy, m_maxEnergy);
    configurator->SetExtraEmModel("gamma", "conv", new G4PairProductionRelModel(), region, m_maxEnergy);
  }
}

GammaConversionOnlyPhysicsTool::GammaConversionOnlyPhysicsTool(const std::string& type,
                                                               const std::string& name,
                                                               const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
}

auto GammaConversionOnlyPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<GammaConversionOnlyPhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), m_processNames.value());
}

GammaConversionOnlyPhysicsTool::PhysicsConstructor::PhysicsConstructor(
    const std::string& name, MSG::Level level, const std::vector<std::string>& processNames)
    : IPhysicsContructor(name, level), m_processNames(processNames) {}

void GammaConversionOnlyPhysicsTool::PhysicsConstructor::ConstructParticle() {}

void GammaConversionOnlyPhysicsTool::PhysicsConstructor::ConstructProcess() {
  // Physics options are registered after the reference physics list, so the
  // photon processes are all in place by now. They are removed, not
  // inactivated: inactivation is refused before the run is initialised.
  G4ProcessManager* processManager = G4Gamma::Gamma()->GetProcessManager();
  G4ProcessVector* processes = processManager->GetProcessList();

  std::vector<G4VProcess*> removed;
  for (std::size_t i = 0; i < processes->size(); ++i) {
    G4VProcess* process = (*processes)[i];
    if (process->GetProcessName() == "GammaGeneralProc") {
      G4Exception("GammaConversionOnlyPhysicsTool", "GammaGeneralProcessOn", FatalException,
                  "G4GammaGeneralProcess holds all photon processes in one: disable it.");
    }
    if (std::find(m_processNames.begin(), m_processNames.end(), process->GetProcessName()) != m_processNames.end()) {
      removed.push_back(process);
    }
  }
  for (G4VProcess* process : removed) {
    ATH_MSG_INFO("Removing gamma process " << process->GetProcessName());
    processManager->RemoveProcess(process);
  }
}
