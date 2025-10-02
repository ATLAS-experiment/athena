/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODPHOTONFILTER_H
#define GENERATORFILTERS_XAODPHOTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Filters and looks for photons
/// @author I Hinchliffe, May 2004
/// @author A Buckley, May 2012
class xAODPhotonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthPhotons"};
  Gaudi::Property<double> m_Ptmin{this, "PtMin", 10000.};
  Gaudi::Property<double> m_Ptmax{this, "PtMax", 100000000.};
  Gaudi::Property<double> m_EtaRange{this, "EtaCut", 2.50};
  Gaudi::Property<int> m_NPhotons{this, "NPhotons", 2};

};

#endif
