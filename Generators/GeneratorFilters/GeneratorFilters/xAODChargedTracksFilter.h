/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODCHARGEDTRACKSFILTER_H
#define GENERATORFILTERS_XAODCHARGEDTRACKSFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

/// Filter events based on presence of charged tracks
class xAODChargedTracksFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  // Minimum pT for a track to count
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 50.0};

  // Maximum |pseudorapidity| for a track to count
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 2.5};

  // Minimum number of tracks
  Gaudi::Property<double> m_NTracks{this, "NTracks", 40};

};

#endif
