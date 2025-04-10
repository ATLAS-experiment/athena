/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODxAODParticleDecayFILTER_H
#define XAODxAODParticleDecayFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/**
   Class to filter events based on the decay of a parent particle into a set of children particles.
   User specifies the parent particle pdg ID and a list of children particle pdg IDs to filter on.
   It was designed with the single particle gun generator in mind, but should in principle
   work in any sample.
   It has been tested, when generating 10k events, to give filter efficiencies consistent with
   the expected branching ratios from the PDG in the cases of w(782)->pi+pi-pi0, w(782)->pi0 + gamma
*/

class xAODParticleDecayFilter : public GenFilter {

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  /** Particle ID of parent particle */
  Gaudi::Property<unsigned int> m_pdgIdParent{this,"ParentPdgId",0,"Particle ID of parent particle"};

  /** Particle IDs of children of parent particle */
  Gaudi::Property<std::vector<int> > m_pdgIdChildren{this,"ChildrenPdgIds",std::vector<int>(),"Particle IDs of children of parent particle"};

  /** Toggle whether we check the charge of children particles */
  Gaudi::Property<bool> m_checkCharge{this,"CheckCharge",true,"Toggle whether we check the charge of children particles"};

};
#endif
