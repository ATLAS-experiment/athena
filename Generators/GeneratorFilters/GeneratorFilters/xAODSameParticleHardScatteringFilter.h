/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATORFILTERS_XAODSAMEPARTICLEHARDSCATTERINGFILTER_H
#define GENERATORFILTERS_XAODSAMEPARTICLEHARDSCATTERINGFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticleContainer.h"

/// Allows the user to search for any given production Parent1 + Parent 2 -> Child1

/// @author Krystsina Petukhova, Oct 2020
class xAODSameParticleHardScatteringFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<std::vector<int>> m_PDGParent{this, "PDGParent", {}};
  Gaudi::Property<std::vector<int>> m_PDGChild{this, "PDGChild", {}};

};

#endif

