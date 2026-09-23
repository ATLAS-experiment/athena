/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TruthLinkRepointAlg_H
#define DERIVATIONFRAMEWORK_TruthLinkRepointAlg_H

// Interface classes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Types for functions. Note these are typedefs, so can't forward reference.
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

// STL includes
#include <string>
#include <vector>

namespace DerivationFramework {

  class TruthLinkRepointAlg : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

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

#endif // DERIVATIONFRAMEWORK_TruthLinkRepointAlg_H
