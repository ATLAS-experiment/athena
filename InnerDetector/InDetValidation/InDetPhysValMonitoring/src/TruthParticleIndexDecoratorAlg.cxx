/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthParticleIndexDecoratorAlg.h"
#include "StoreGate/WriteDecorHandle.h"

TruthParticleIndexDecoratorAlg::TruthParticleIndexDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode
TruthParticleIndexDecoratorAlg::initialize() {

  ATH_CHECK( m_truthParticleName.initialize());
  if (m_indexDecor.key().empty()) {
     ATH_MSG_FATAL("Empty truth index name.");
     return StatusCode::FAILURE;
  }
  m_indexDecor = m_truthParticleName.key()+"."+m_indexDecor.key();
  ATH_CHECK( m_indexDecor.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode
TruthParticleIndexDecoratorAlg::execute(const EventContext &ctx) const {
  SG::ReadHandle<xAOD::TruthParticleContainer> ptruth(m_truthParticleName, ctx);
  if ((not ptruth.isValid())) {
    return StatusCode::FAILURE;
  }
  SG::WriteDecorHandle<xAOD::TruthParticleContainer,unsigned int> index_decorator( m_indexDecor, ctx);

  for (const xAOD::TruthParticle *truth_particle : *ptruth) {
     index_decorator(*truth_particle) = truth_particle->index();
  }
  return StatusCode::SUCCESS;
}
