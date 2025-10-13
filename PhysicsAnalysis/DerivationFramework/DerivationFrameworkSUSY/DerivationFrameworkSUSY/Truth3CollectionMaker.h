/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"
class IMCTruthClassifier;

namespace DerivationFramework {

  class Truth3CollectionMaker : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool> {
  public:
    Truth3CollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
    ~Truth3CollectionMaker();
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode addBranches() const override;

  private:
    mutable std::atomic<unsigned int> m_ntotpart{};
    mutable std::atomic<unsigned int> m_npasspart{};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey{this, "ParticlesKey", "TruthParticles"};
    SG::WriteHandleKey<xAOD::TruthParticleContainer> m_collectionName{this, "NewCollectionName", ""};
    PublicToolHandle<IMCTruthClassifier> m_classifier{this, "MCTruthClassifier", "MCTruthClassifier/MCTruthClassifier"};
    Gaudi::Property<std::string> m_partString{this, "ParticleSelectionString", ""};
    Gaudi::Property<bool> m_runClassifier{this, "RunClassifier", true};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTH3COLLECTIONMAKER_H
