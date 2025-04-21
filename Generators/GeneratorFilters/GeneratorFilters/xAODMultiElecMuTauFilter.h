/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODMULTIELECMUTAUFILTER_H
#define GENERATORFILTERS_XAODMULTIELECMUTAUFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Select multiple charged leptons taking into account electrons, muons and taus (inc. hadronic decays).
///
///  The user can steer the maximum |eta| and minimum pt and there is a separate
/// (visible) pt cut for hadronic taus.
///
/// @author Carl Gwilliam
class xAODMultiElecMuTauFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<double> m_minPt{this, "MinPt", 5000.};
  Gaudi::Property<double> m_maxEta{this, "MaxEta", 10.0};
  Gaudi::Property<double> m_minVisPtHadTau{this, "MinVisPtHadTau", 10000.};
  Gaudi::Property<int> m_NLeptons{this, "NLeptons", 4};
  Gaudi::Property<bool> m_incHadTau{this, "IncludeHadTaus", true};
  Gaudi::Property<bool> m_TwoSameSignLightLeptonsOneHadTau{this, "TwoSameSignLightLeptonsOneHadTau", false};
};

#endif
