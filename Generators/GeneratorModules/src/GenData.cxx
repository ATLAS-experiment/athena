/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorModules/GenData.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"
#include <atomic>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <mutex>
GenData::GenData():  m_service_name("PartPropSvc"), m_name("GenData"), m_ppSvc("PartPropSvc", "GenData") {}

GenData::GenData(const std::string& sname, const std::string& name): m_service_name(sname),  m_name(name), m_ppSvc(sname, name) {}

GenData::ParticleInfo GenData::particleInfo(int absPid) const {

  if (!m_initialized) {
    if (m_ppSvc.retrieve().isFailure()) {
      std::cerr << "GenData: failed to retrieve IPartPropSvc(" << m_service_name << ", " << m_name << ")\n";
      std::abort();
    }
    m_initialized = true;
  }

    ParticleInfo info;
    if (const HepPDT::ParticleData* particle = m_ppSvc->PDT()->particle(HepPDT::ParticleID(absPid))) {
      info.mass = particle->mass().value();
      info.lifetime = particle->lifetime();
      info.name = particle->name();
    }  
    return info;
}

std::optional<double> GenData::particleMass(int pdgId) const {
  return particleInfo(std::abs(pdgId)).mass;
}

std::optional<double> GenData::particleLifetime(int pdgId) const {
  return particleInfo(std::abs(pdgId)).lifetime;
}

std::optional<std::string> GenData::particleName(int pdgId) const {
  return particleInfo(std::abs(pdgId)).name;
}
