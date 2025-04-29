/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODTTBARWTOLEPTONFILTER_H
#define GENERATORFILTERS_XAODTTBARWTOLEPTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Require the event to contain at least one charged lepton (from W decay, which should come from top) with pt at or above Ptcut.
///
/// Events that do not contain t AND t CLHEP::bar quarks are rejected.
/// Only tops decaying to W X are analyzed and counted in this algorithm.
/// @author Gia Khoriauli, June 2008
/// @author Andy Buckley, extension to specific lepton multiplicities, April 2012
class xAODTTbarWToLeptonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 200000.};
  Gaudi::Property<int> m_numLeptons{this, "NumLeptons", -1}; // Negative for >0, positive integers for the specific number
  Gaudi::Property<bool> m_fourTopsFilter{this, "fourTopsFilter", false}; // four top filter or not
  Gaudi::Property<bool> m_SSMLFilter{this, "SSMLFilter", false}; // Same sign multilepton filter or not

};

#endif
