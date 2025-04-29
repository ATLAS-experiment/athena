/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODMULTIMUONFILTER_H
#define GENERATORFILTERS_XAODMULTIMUONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Filters and looks for muons
/// @author I Hinchliffe,  December 2001
/// @author G. Watts, Sep 2006
class xAODMultiMuonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthMuons"};
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 10000.};
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 10.0};
  Gaudi::Property<int> m_NMuons{this, "NMuons", 2};

};

#endif
