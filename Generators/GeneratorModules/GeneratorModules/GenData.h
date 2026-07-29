/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULES_GENDATA_H
#define GENERATORMODULES_GENDATA_H

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "CxxUtils/checker_macros.h"

#include <atomic>
#include <optional>
#include <string>

///GenData is a class for particle data access

class GenData  {
public:

  GenData();

  GenData(const std::string& sname, const std::string& name);

  virtual ~GenData() = default;

  /// Get the mass of a particle by PDG ID
  std::optional<double> particleMass(int pdgId) const;

  /// Get the lifetime of a particle by PDG ID
  std::optional<double> particleLifetime(int pdgId) const;

  /// Get the name of a particle by PDG ID
  std::optional<std::string> particleName(int pdgId) const;
  
private:

  struct ParticleInfo {
    std::optional<double> mass;
    std::optional<double> lifetime;
    std::optional<std::string> name;
  };
  ParticleInfo particleInfo(int absPid) const;
  
  /// Handle on the particle property service
  std::string m_service_name{"PartPropSvc"};
  std::string m_name{"GenData"};
  ServiceHandle<IPartPropSvc> m_ppSvc;
  mutable std::atomic<bool> m_initialized{false};

};

#endif
