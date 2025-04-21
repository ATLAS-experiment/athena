/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
      TruthLinkRepointTool(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthLinkRepointTool();
      virtual StatusCode addBranches() const override final;
      virtual StatusCode initialize() override final;

    private:
      /// Parameter: input collection key
      SG::ReadHandleKey<xAOD::IParticleContainer> m_recoKey{this,"RecoCollection", "Muons", 
                                            "Name of reco collection for decoration"};
      /// Parameter: output decoration
      Gaudi::Property<std::string> m_decOutput{this, "OutputDecoration",
                                  "TruthLink", "Name of the output decoration on the reco object"};
      /// Parameter: target collection
      SG::ReadHandleKeyArray<xAOD::TruthParticleContainer> m_targetKeys{this, "TargetCollections", {"TruthMuons","TruthPhotons","TruthElectrons"}, "Name of target truth collections"};
      
      SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decorKey{this, "TargetDecorKeys", "", "Will be overwritten during initialize"};
      // Helper function for finding matching truth particle and warning consistently
      static int find_match(const xAOD::TruthParticle* p, const xAOD::TruthParticleContainer* c) ;
  }; 
}

#endif // DERIVATIONFRAMEWORK_TruthLinkRepointTool_H
