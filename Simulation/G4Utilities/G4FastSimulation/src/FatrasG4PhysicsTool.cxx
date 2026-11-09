/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FatrasG4PhysicsTool.h"
#include "FatrasG4Regions.h"

#include "G4DummyModel.hh"
#include "G4EmConfigurator.hh"
#include "G4LossTableManager.hh"
#include "G4Exception.hh"
#include "G4Gamma.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessVector.hh"
#include "G4Region.hh"
#include "G4VEmProcess.hh"
#include "G4VEnergyLossProcess.hh"
#include "G4VMultipleScattering.hh"

#include "CLHEP/Units/SystemOfUnits.h"

#include <algorithm>
#include <cfloat>
#include <set>

namespace {
  // G4DummyModel with a vanishing but non-zero stopping power, for the energy
  // loss processes: Geant4 10.6 G4LossTableBuilder::BuildRangeTable returns on
  // the first all-zero dE/dx couple, leaving every later couple without a range
  // table and the next e+- step to dereference it. Made inactive once
  // registered, so that AlongStepDoIt returns before asking it for a
  // fluctuation model it has not got.
  class NoEnergyLossModel final : public G4DummyModel {
   public:
    G4double ComputeDEDXPerVolume(const G4Material*, const G4ParticleDefinition*,
                                  G4double, G4double) override {
      return 1.e-20 * CLHEP::MeV / CLHEP::mm;
    }
  };
}

FatrasG4PhysicsTool::FatrasG4PhysicsTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
}

auto FatrasG4PhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<FatrasG4PhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), m_envelopeVolumes.value(), m_disableAll.value(),
      m_inactiveProcesses.value());
}

FatrasG4PhysicsTool::PhysicsConstructor::PhysicsConstructor(
    const std::string& name, MSG::Level level,
    const std::vector<std::string>& envelopeVolumes,
    bool disableAll, const std::vector<std::string>& inactiveProcesses)
    : IPhysicsContructor(name, level),
      m_envelopeVolumes(envelopeVolumes),
      m_disableAll(disableAll),
      m_inactiveProcesses(inactiveProcesses) {}

void FatrasG4PhysicsTool::PhysicsConstructor::ConstructParticle() {}

void FatrasG4PhysicsTool::PhysicsConstructor::ConstructProcess() {
  // Physics options are constructed after the reference physics list, so every
  // process is in place by now. The configurator is per thread and only adds
  // the models when the processes prepare their physics tables.
  G4EmConfigurator* configurator = G4LossTableManager::Instance()->EmConfigurator();
  std::set<std::string> switchedOff;

  // Every region inside the envelopes, so that nothing in them keeps its physics
  std::vector<std::string> regionNames;
  for (const G4Region* region : fatrasG4RegionsInside(m_envelopeVolumes)) regionNames.push_back(region->GetName());
  if (regionNames.empty()) {
    G4Exception("FatrasG4PhysicsTool", "NoFatrasG4Regions", FatalException,
                "No Geant4 region inside the FatrasG4 envelope volumes.");
  }

  G4ParticleTable::G4PTblDicIterator* particles = G4ParticleTable::GetParticleTable()->GetIterator();
  particles->reset();
  while ((*particles)()) {
    const G4ParticleDefinition* particle = particles->value();
    const G4ProcessManager* processManager = particle->GetProcessManager();
    if (!processManager) continue;

    G4ProcessVector* processes = processManager->GetProcessList();
    for (std::size_t i = 0; i < processes->size(); ++i) {
      G4VProcess* process = (*processes)[i];
      const std::string name = process->GetProcessName();
      const bool energyLoss = dynamic_cast<G4VEnergyLossProcess*>(process) != nullptr;
      if (!energyLoss && !dynamic_cast<G4VEmProcess*>(process) &&
          !dynamic_cast<G4VMultipleScattering*>(process)) continue;
      if (!m_disableAll && std::find(m_inactiveProcesses.begin(), m_inactiveProcesses.end(), name) ==
          m_inactiveProcesses.end()) continue;

      for (const std::string& region : regionNames) {
        G4VEmModel* model = energyLoss ? static_cast<G4VEmModel*>(new NoEnergyLossModel())
                                       : new G4DummyModel();
        configurator->SetExtraEmModel(particle->GetParticleName(), name, model, region, 0., DBL_MAX);
        // SetExtraEmModel overwrites the activation limit with its emax
        if (energyLoss) model->SetActivationHighEnergyLimit(0.);
      }
      switchedOff.insert(name);
    }
  }

  for (const std::string& name : m_inactiveProcesses) {
    if (!switchedOff.count(name)) ATH_MSG_WARNING("No Geant4 electromagnetic process called " << name);
  }
  std::string names, regions;
  for (const std::string& name : switchedOff) names += " " + name;
  for (const std::string& region : regionNames) regions += " " + region;
  ATH_MSG_INFO("Switching off" << names << " in regions" << regions);
}
