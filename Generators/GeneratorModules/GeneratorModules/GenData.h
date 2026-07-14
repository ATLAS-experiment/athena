/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULES_GENDATA_H
#define GENERATORMODULES_GENDATA_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandleKey.h"

#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"

#include <atomic>
#include <string>
#include <vector>
#include <map>
#include <cmath> //for std::abs
#include <stdexcept>
#include <stdlib.h>
#include <cstdio>
#include <iostream>

///GenData is a class for particle data access

class GenData  {
public:

  GenData() { };

  virtual ~GenData() { }

private:
  /// Access an element in the particle data table
  const HepPDT::ParticleData* particleData(int pid) const {
    if (!m_initialized) {
      if (m_ppSvc.retrieve().isFailure()) {
            std::cerr<<  "GenData: failed to retrieve PartPropSvc\n";
            std::abort();
      }
      m_initialized = true;
    }
    const int absPid = std::abs(pid);
    return m_ppSvc->PDT()->particle(HepPDT::ParticleID(absPid));
  }
public:
  
  std::optional<double> particleMass(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    return particle ? std::optional<double>{particle->mass().value()} : std::nullopt;
  }

  std::optional<double> particleLifetime(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    return particle ? std::optional<double>{particle->lifetime()} : std::nullopt;
  }

  std::optional<std::string> particleName(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    return particle ? std::optional<std::string>{particle->name()} : std::nullopt;
  }

private:

  /// Handle on the particle property service
  ServiceHandle<IPartPropSvc> m_ppSvc{"PartPropSvc", "GenData"};
  mutable std::atomic<bool> m_initialized{false};

};


#endif
