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
#include "CxxUtils/checker_macros.h"
#include "StoreGate/ReadHandleKey.h"
#include "GeneratorObjects/McEventCollection.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"

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
/// @class GenData
/// @brief Class for particle data access in GenBase

class GenData  {
public:

  /// @name Construction/destruction
  //@{

  /// Constructor
  GenData() { };

  /// Virtual destructor
  virtual ~GenData() { }

  //@}

  /// @name Particle data accessors
  //@{

  /// Access the particle property service
  const ServiceHandle<IPartPropSvc> partPropSvc() const {
    return m_ppSvc;
  }

  /// Get a particle data table
  const HepPDT::ParticleDataTable& particleTable() const {
    if (!m_initialized) {
      if (m_ppSvc.retrieve().isFailure()) {
            std::cerr<<  "GenData: failed to retrieve PartPropSvc\n";
            std::abort();
      }
      m_initialized = true;
    }
    return *(m_ppSvc->PDT());
  }

  /// Shorter alias to get a particle data table
  const HepPDT::ParticleDataTable& pdt() const { return particleTable(); }

  /// Access an element in the particle data table
  const HepPDT::ParticleData* particleData(int pid) const {
    return pdt().particle(HepPDT::ParticleID(std::abs(pid)));
  }
  
  std::optional<double> particleMass(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    if (!particle) {
        return std::nullopt;
    }
    return particle->mass().value();
  }

  std::optional<double> particleLifetime(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    if (!particle) {
        return std::nullopt;
    }
    return particle->lifetime();
  }
  std::optional<std::string> particleName(int pdgId) const {
    const HepPDT::ParticleData* particle = particleData(std::abs(pdgId));
    if (!particle) {
        return std::nullopt;
    }
    return particle->name();
  }

  //@}



private:

  /// Handle on the particle property service
  ServiceHandle<IPartPropSvc> m_ppSvc{"PartPropSvc", "GenData"};
  mutable std::atomic<bool> m_initialized{false};

};


#endif
