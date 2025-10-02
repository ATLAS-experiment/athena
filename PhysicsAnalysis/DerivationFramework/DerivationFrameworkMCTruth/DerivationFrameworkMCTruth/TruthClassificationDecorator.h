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
      TruthClassificationDecorator(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthClassificationDecorator();
      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      mutable std::atomic<unsigned int> m_ntotpart;
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
      ToolHandle<IMCTruthClassifier> m_classifier;
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHCLASSIFICATIONDECORATOR_H 
