/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATORFILTERS_XAODDIRECTPHOTONFILTER_H
#define GENERATORFILTERS_XAODDIRECTPHOTONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <limits>

/// Filters and looks for photons from brem or hadr proc
/// @author L. Carminati, June 2010
class xAODDirectPhotonFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<std::vector<double>> m_Ptmin{this, "Ptmin", {10000.}};
  Gaudi::Property<std::vector<double>> m_Ptmax{this, "Ptmax", {std::numeric_limits<double>::max()}};
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 2.50};
  Gaudi::Property<unsigned int> m_NPhotons{this, "NPhotons", 1};
  Gaudi::Property<bool> m_AllowSUSYDecay{this, "AllowSUSYDecay", false};
  Gaudi::Property<bool> m_OrderPhotons{this, "OrderPhotons", true};
};

#endif
