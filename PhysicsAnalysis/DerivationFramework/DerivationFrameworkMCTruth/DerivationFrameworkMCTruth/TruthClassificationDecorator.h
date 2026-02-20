/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H
#define DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

class IMCTruthClassifier;

namespace DerivationFramework {

  class TruthClassificationDecorator : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    mutable std::atomic<unsigned int> m_ntotpart{};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
      {this, "ParticlesKey", "TruthParticles", "ReadHandleKey for input TruthParticleContainer"};
    // Decorator keys
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
      {this, "classifierParticleOrigin", m_particlesKey, "classifierParticleOrigin", "Particle origin decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_typeDecoratorKey
      {this, "classifierParticleType", m_particlesKey, "classifierParticleType", "Particle type decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_outcomeDecoratorKey
      {this, "classifierParticleOutCome", m_particlesKey, "classifierParticleOutCome", "Particle outcome decoration"};
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_classificationDecoratorKey
      {this, "Classification", m_particlesKey, "Classification", "Classification code decorator"};
    PublicToolHandle<IMCTruthClassifier> m_classifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H
