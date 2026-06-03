/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_PARTONJETAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_PARTONJETAUGMENTATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"

#include <vector>

class TLorentzVector;


namespace DerivationFramework {

  class PartonJetAugmentationTool : public extends<AthAlgTool, IAugmentationTool>  {

  public:
    using base_class::base_class; 

    virtual StatusCode initialize() override;
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{
      this, "TruthParticles", "TruthParticles", "Input truth particles"
    };

    SG::WriteHandleKey<xAOD::JetContainer> m_partonJetsKey{
      this, "PartonJets", "PartonJets", "Output parton jets"
    };
    bool extrajet(const xAOD::TruthParticleContainer* truthParticles,
                  TLorentzVector& lj,
                  TLorentzVector& slj,
                  TLorentzVector& tlj,
                  const std::vector<TLorentzVector>& ttbarDecayProducts,
                  const std::vector<int>& decayProduct_pdgID,
                  double Rparam,
                  double pt_min) const;

    bool isLastBeforeHadronization(const xAOD::TruthParticle* p) const;


  };
}

#endif