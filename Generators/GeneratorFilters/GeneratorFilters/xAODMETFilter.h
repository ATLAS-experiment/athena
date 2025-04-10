/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODMETFILTER_H
#define GENERATORFILTERS_XAODMETFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Filters on total missing energy from nus and LSPs
/// @author Seth Zenz, December 2005
class xAODMETFilter:public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

 private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthMET"};
  Gaudi::Property<double> m_METmin{this, "METCut", 10000.};
  // Normally we'd include them, but this is unstable if using EvtGen
  Gaudi::Property<bool> m_useHadronicNu{this, "UseNeutrinosFromHadrons", false};

};

#endif
