/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkMCTruth/HadronOriginDecorator.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  HadronOriginDecorator::HadronOriginDecorator(const std::string& t, const std::string& n, const IInterface* p):
    base_class(t,n,p)
  {
  }

  HadronOriginDecorator::~HadronOriginDecorator(){}

  StatusCode HadronOriginDecorator::initialize(){
    ATH_MSG_VERBOSE( "Initialize" );
    ATH_CHECK( m_particlesKey.initialize() );
    ATH_CHECK(m_originDecoratorKey.initialize());
    ATH_CHECK(m_Tool.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode HadronOriginDecorator::addBranches() const{
    // Event context for multi-threading
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // Retrieve truth collections
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);
    if (!truthParticles.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);
      return StatusCode::FAILURE;
    }

    std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id>  hadronMap=m_Tool->GetOriginMap();
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int> originDecorator(m_originDecoratorKey, ctx);
    for (auto* truthParticle : *truthParticles) {
      originDecorator(*truthParticle) = (hadronMap.find(truthParticle)!=hadronMap.end()) ? static_cast<int>(hadronMap[truthParticle]) : 6;
    }

    return StatusCode::SUCCESS;
  }

} /// namespace
