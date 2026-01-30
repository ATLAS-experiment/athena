/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHISOLATIONTOOL_H
#define DERIVATIONFRAMEWORK_TRUTHISOLATIONTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "Gaudi/Property.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "DerivationFrameworkMCTruth/DecayGraphHelper.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "GaudiKernel/ToolHandle.h"

namespace DerivationFramework {

  class TruthIsolationTool : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /// Parameter: input collection key
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_isoParticlesKey
    {this, "isoParticlesKey", "TruthParticles",  "Name of TruthParticle key for input"};
    /// Parameter: input collection key for particles used in calculation
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_allParticlesKey
      {this, "allParticlesKey", "TruthParticles", "Name of Truthparticle key to find in iso cone"};
    /// Decor handle key array
    SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer, float> m_isoDecorKeys
      {this, "DoNotSet_isoDecorKeys", {}, "WriteDecorHandleKeyArray - set internally but must be property"};
    /// Parameter: Cone size for Isolation
    Gaudi::Property<std::vector<float> > m_coneSizes
      {this, "IsolationConeSizes", {0.2}, "Vector of sizes of dR cone in which to include particles"};
    /// Parameter: only use charged particles for iso?
    Gaudi::Property<bool> m_chargedOnly
      {this, "ChargedParticlesOnly", false, "Only keep charged particles in isolation cone"};
    /// Parameter: List of pdgIDs of particles to dress
    Gaudi::Property<std::vector<int> > m_listOfPIDs
      {this, "particleIDsToCalculate", {11,13,22}, "List of the pdgIDs of particles for which to calculate isolation"};
    /// Parameter: List of pdgIDs to exclude from cone calculation
    Gaudi::Property<std::vector<int> > m_excludeFromCone
      {this, "excludeIDsFromCone", {}, "List of the pdgIDs of particles to exclude from the cone when calculating isolation"};
    /// Parameter: name of output variable
    Gaudi::Property<std::string> m_isoVarNamePrefix
      {this, "IsolationVarNamePrefix", "", "Prefix of name of the variable to add to output xAOD"};
    /// Parameter: Include non-interacting particles?
    Gaudi::Property<bool> m_includeNonInteracting
      {this, "IncludeNonInteracting", false, "Include non-interacting particles in the isolation definition"};
    /// Parameter: Use variable radius?
    Gaudi::Property<bool>  m_variableR
      {this, "VariableR", false, "Use radius that shrinks with pT in isolation"};

    std::vector<float> m_coneSizesSort;

    //private helper functions
    void calcIsos(const xAOD::TruthParticle* particle,
                  const std::vector<const xAOD::TruthParticle*> &,
                  std::vector<float>&) const;
    static float calculateDeltaR2(const xAOD::IParticle *p1, float eta2, float phi2) ;
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHISOLATIONTOOL_H
