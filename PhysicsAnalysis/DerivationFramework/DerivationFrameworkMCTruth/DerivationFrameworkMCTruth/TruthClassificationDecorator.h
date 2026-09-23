/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H
#define DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"

class IMCTruthClassifier;

namespace DerivationFramework {

  class TruthClassificationDecorator : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual StatusCode finalize() override final;

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
    ServiceHandle<IChronoStatSvc>      m_chronoSvc{this, "ChronoStatSvc",  "ChronoStatSvc"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H
