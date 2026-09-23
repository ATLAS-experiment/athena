/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file DerivationFrameworkTop/TopHeavyFlavorFilterAugmentation.h
 * @date Apr. 2015
 * @brief tool to add a variable to the TruthParticles corresponding to the HF hadrons origin flag
 */


#ifndef DerivationFrameworkMCTruth_HadronOriginDecorator_H
#define DerivationFrameworkMCTruth_HadronOriginDecorator_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"

namespace DerivationFramework {

  class HadronOriginDecorator : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_particlesKey
    {this, "TruthEventName", "TruthParticles", "ReadHandleKey for input TruthParticleContainer"};
    // Decorator keys
    SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_originDecoratorKey
      {this, "classifierParticleOrigin", m_particlesKey, "TopHadronOriginFlag", "Top Hadron origin decoration"};
    PublicToolHandle<DerivationFramework::HadronOriginClassifier> m_Tool{this, "ToolName", ""};


  }; /// class

} /// namespace


#endif
