/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODMULTILEPTONFILTER_H
#define GENERATORFILTERS_XAODMULTILEPTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

class xAODMultiLeptonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent(const EventContext& ctx) override final;

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthElectronContKey{this, "TruthElectronContainerKey", "TruthElectrons"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonContKey{this, "TruthMuonContainerKey", "TruthMuons"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthTauContKey{this, "TruthTauContainerKey", "TruthTaus"};

  // Decide which flavours are counted
  Gaudi::Property<bool> m_countElectrons{this, "countElectrons", true};
  Gaudi::Property<bool> m_countMuons{this, "countMuons", true};
  Gaudi::Property<bool> m_countTaus{this, "countTaus", false}; //not included by default

  // Common defaults
  Gaudi::Property<double> m_Ptmin{this, "PtCut", -1.0};
  Gaudi::Property<double> m_EtaRange{this, "EtaCut", -1.0};
  Gaudi::Property<int> m_NLeptons{this, "NLeptons", 4};

  // Per flavour overrides - negative means inherit common value
  Gaudi::Property<double> m_ElePtCut{this, "ElePtCut", -1.0};
  Gaudi::Property<double> m_MuonPtCut{this, "MuonPtCut", -1.0};
  Gaudi::Property<double> m_TauPtCut{this, "TauVisPtCut", -1.0};

  Gaudi::Property<double> m_EleEtaCut{this, "EleEtaCut", -1.0};
  Gaudi::Property<double> m_MuonEtaCut{this, "MuonEtaCut", -1.0};
  Gaudi::Property<double> m_TauEtaCut{this, "TauVisEtaCut", -1.0};

};

#endif
