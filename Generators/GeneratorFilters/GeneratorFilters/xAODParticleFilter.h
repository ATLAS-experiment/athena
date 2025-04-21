/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_xAODParticleFilter_H
#define GENERATORFILTERS_xAODParticleFilter_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// The filter will pass only if it finds a particle with the specified properties
/// @author I Hinchliffe, May 2004
/// @author B K Gjelsten, March 2006
/// @author R Bruneliere, Aug 2008
class xAODParticleFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 10000.};
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 10.0};
  Gaudi::Property<double> m_EnergyRange{this, "Energycut", 100000000.0};
  Gaudi::Property<int>    m_PDGID{this, "PDG", 11};
  Gaudi::Property<int>    m_MinParts{this, "MinParts", 1};
  Gaudi::Property<bool>   m_Exclusive{this, "Exclusive", false};
};

#endif
