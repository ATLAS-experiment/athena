/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TruthLinkRepointTool_H
#define DERIVATIONFRAMEWORK_TruthLinkRepointTool_H

// Interface classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Types for functions. Note these are typedefs, so can't forward reference.
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

// STL includes
#include <string>
#include <vector>

namespace DerivationFramework {

  class TruthLinkRepointTool : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /// Parameter: input collection key
    SG::ReadHandleKey<xAOD::IParticleContainer> m_recoKey{this,"RecoCollection", "Muons",
        "Name of reco collection for decoration"};
    /// Parameter: output decoration
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decorKey{this, "OutputDecoration", m_recoKey, "TruthLink", "Name of the output decoration on the reco object"};
    /// Parameter: target collection
    SG::ReadHandleKeyArray<xAOD::TruthParticleContainer> m_targetKeys{this, "TargetCollections", {"TruthMuons","TruthPhotons","TruthElectrons"}, "Name of target truth collections"};

    // Helper function for finding matching truth particle and warning consistently
    static int find_match(const xAOD::TruthParticle* p, const xAOD::TruthParticleContainer* c) ;
  };
}

#endif // DERIVATIONFRAMEWORK_TruthLinkRepointTool_H
