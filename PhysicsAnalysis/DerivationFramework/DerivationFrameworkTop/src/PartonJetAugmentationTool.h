/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_PARTONJETAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_PARTONJETAUGMENTATIONTOOL_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODJet/JetContainer.h"

#include <vector>

class TLorentzVector;


namespace DerivationFramework {

  class PartonJetAugmentationTool : public AthReentrantAlgorithm  { // FIXME RENAME

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm; 

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

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
