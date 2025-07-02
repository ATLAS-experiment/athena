/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthUtils/ParticleConstants.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "HepPDT/ParticleDataTable.hh"
#include <limits>
#include <format>

int main ()
{
  ISvcLocator* svcLocator = Gaudi::svcLocator();
  if (!svcLocator) {
    std::cerr << "Could not get svcLocator." << std::endl;
    return 1;
  }

  SmartIF<IPartPropSvc> partPropSvc{svcLocator->service( "PartPropSvc" )};
  if(!partPropSvc) {
    std::cerr << "Could not get PartPropSvc." << std::endl;
    return 1;
  }

  if (partPropSvc->initialize().isFailure()) {
    std::cerr << "Could not initialize PartPropSvc." << std::endl;
    return 1;
  }

  const HepPDT::ParticleDataTable* particleDataTable = partPropSvc->PDT();
  if (!particleDataTable) {
    std::cerr << "Could not get ParticleDataTable." << std::endl;
    return 1;
  }

  std::vector<std::pair<int,double>> particleMasses = {
    {11, ParticleConstants::electronMassInMeV},
    {13, ParticleConstants::muonMassInMeV},
    {211, ParticleConstants::chargedPionMassInMeV}
  };

  bool error = false;
  for (const auto& [pid, expectedMass] : particleMasses) {
    auto particle = particleDataTable->particle(HepPDT::ParticleID(pid));
    if (!particle) {
      std::cerr << "Could not get particle data for " << pid << "." << std::endl;
      error = true;
      continue;
    }

    if (std::abs(particle->mass().value() - expectedMass) > std::numeric_limits<double>::epsilon()) {
      std::cerr << "Mass mismatch for " << particle->name() << ": PDT=" << std::format("{}", particle->mass().value()) << " ParticleConstants=" << std::format("{}", expectedMass) << " difference=" << std::format("{}", std::abs(particle->mass().value() - expectedMass)) << std::endl;
      error = true;
    }
  }

  if (error) {
    return 1;
  }

  std::cout << "All particle constants match the ParticleDataTable." << std::endl;
  return 0;
}