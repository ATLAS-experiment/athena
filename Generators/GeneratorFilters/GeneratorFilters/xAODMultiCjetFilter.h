/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Written by Dominik Derendarz (dominik.derendarz@cern.ch)
// Based on MultiBjetFilter by Bill Balunas

#ifndef GENERATORFILTERSXAODMULTICJETFILTER_H
#define GENERATORFILTERSXAODMULTICJETFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

class xAODMultiCjetFilter:public GenFilter {

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterFinalize() override final;
  virtual StatusCode filterEvent() override final;

private:

  // Basic jet requirements
  Gaudi::Property<double> m_deltaRFromTruth{this,"DeltaRFromTruth",0.3,"Delta R from truth particle"};
  Gaudi::Property<double> m_jetPtMin{this,"JetPtMin",15000.,"Minimal jet Pt"};
  Gaudi::Property<double> m_jetEtaMax{this,"JetEtaMax",2.7,"Maximal jet eta"};
  Gaudi::Property<int> m_nJetsMin{this,"NJetsMin",0,"Minimal jet multiplicity"};
  Gaudi::Property<int> m_nJetsMax{this,"NJetsMax",-1,"Maximal jet multiplicity"};

  // Variables for cutting sample into pt slices
  Gaudi::Property<double> m_leadJet_ptMin{this,"LeadJetPtMin",0,"Minimal leading b-jet Pt"};
  Gaudi::Property<double> m_leadJet_ptMax{this,"LeadJetPtMax",-1,"Maximal leading b-jet Pt"};

  // Flavor filter variables
  Gaudi::Property<double> m_bottomPtMin{this,"BottomPtMin",5000.,"Minimal bottom Pt"};
  Gaudi::Property<double> m_bottomEtaMax{this,"BottomEtaMax",3.0,"Maximal bottom eta"};
  Gaudi::Property<int> m_nCJetsMin{this,"NCJetsMin",0,"Minimal c-jet multiplicity"};
  Gaudi::Property<int> m_nCJetsMax{this,"NCJetsMax",-1,"Maximal c-jet multiplicity"};
  Gaudi::Property<double> m_charmPtMin{this,"CharmPtMin",5000.,"Minimal charm Pt"};
  Gaudi::Property<double> m_charmEtaMax{this,"CharmEtaMax",3.0,"Maximal charm eta"};

  SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainer", "AntiKt4TruthJets"}; // Name of the truth jet container
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  // Internal bookkeeping variables
  int    m_NPass{};
  int    m_Nevt{};
  double m_SumOfWeights_Pass{};
  double m_SumOfWeights_Evt{};
};

#endif
