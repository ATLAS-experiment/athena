/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHNAVIGATIONDECORATOR_H
#define DERIVATIONFRAMEWORK_TRUTHNAVIGATIONDECORATOR_H

// Interface classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Handles
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

// EDM includes -- typedefs, so can't just be forward declared
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

// STL includes
#include <string>
#include <vector>
#include <map>

namespace DerivationFramework {

  class TruthNavigationDecorator : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /// Parameter: input particle collections
    SG::ReadHandleKeyArray<xAOD::TruthParticleContainer> m_inputKeys
    {this, "InputCollections", {}, "Input truth particle collection keys"};
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventKey
      {this, "TruthEventKey", "TruthEvents", "SG key for the TruthEvent container"};
    /// Decor keys
    SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer, std::vector<ElementLink<xAOD::TruthParticleContainer> >> m_parentLinksDecorKeys
      {this, "parentDecorKeys", {}, "dependent on the names in InputCollections"};
    SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer, std::vector<ElementLink<xAOD::TruthParticleContainer> >> m_childLinksDecorKeys
      {this, "childDecorKeys", {}, "dependent on the name in InputCollections"};
    /// Helper function for finding all the parents of a particle
    void find_parents( const xAOD::TruthParticle* part ,
                       std::vector<ElementLink<xAOD::TruthParticleContainer> >& parents ,
                       std::map<int,ElementLink<xAOD::TruthParticleContainer> >& linkMap ,
                       std::vector<int>& seen_particles ) const;
    /// Helper function for finding all the children of a particle
    void find_children( const xAOD::TruthParticle* part ,
                        std::vector<ElementLink<xAOD::TruthParticleContainer> >& parents ,
                        std::map<int,ElementLink<xAOD::TruthParticleContainer> >& linkMap ,
                        std::vector<int>& seen_particles ) const;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHNAVIGATIONDECORATOR_H
