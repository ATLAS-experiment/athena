/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHPARTICLEINDEXDECORATORALG_H
#define TRUTHPARTICLEINDEXDECORATORALG_H

// STL includes
#include <string>
#include "xAODTruth/TruthParticleContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

// class to decorate xAOD::TruthParticles with additional information required by validation
class TruthParticleIndexDecoratorAlg: public AthReentrantAlgorithm {
public:
  TruthParticleIndexDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize();
  virtual StatusCode execute(const EventContext &ctx) const;

private:

  ///TruthParticle container's name needed to create decorators
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleName
    {this, "TruthParticleContainerName",  "TruthParticles", ""};

  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_indexDecor
    {this, "TruthParticleIndexDecoration", m_truthParticleName, "origTruthIndex", "decoration name for the original truth particle index."};
};
#endif
