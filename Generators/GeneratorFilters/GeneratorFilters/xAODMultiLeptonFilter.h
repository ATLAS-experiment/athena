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
  virtual StatusCode filterEvent() override final;

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthElectronContKey{this, "TruthElectronContainerKey", "TruthElectrons"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonContKey{this, "TruthMuonContainerKey", "TruthMuons"};

  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 10000.};
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 10.0};
  Gaudi::Property<int> m_NLeptons{this, "NLeptons", 4};

};

#endif
