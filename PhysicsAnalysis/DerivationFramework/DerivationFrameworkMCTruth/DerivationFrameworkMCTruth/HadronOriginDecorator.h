/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file DerivationFrameworkTop/TopHeavyFlavorFilterAugmentation.h
 * @date Apr. 2015
 * @brief tool to add a variable to the TruthParticles corresponding to the HF hadrons origin flag
 */


#ifndef DerivationFrameworkMCTruth_HadronOriginDecorator_H
#define DerivationFrameworkMCTruth_HadronOriginDecorator_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"

namespace DerivationFramework {

  class HadronOriginDecorator : public extends<AthAlgTool, IAugmentationTool> {
  public:
    HadronOriginDecorator(const std::string& t, const std::string& n, const IInterface* p);
    ~HadronOriginDecorator();
    StatusCode initialize();
    virtual StatusCode addBranches(const EventContext& ctx) const;

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
