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

#include <map>

/// Filter events based on presence of charged tracks
class xAODChargedTracksFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent(const EventContext& ctx) override final;

private:

  /// true if a particle with this PDG ID and pt should be excluded from the
  /// charged-track count: |pdgId| is a key of m_excludedPdgIdPtMin AND pt is
  /// at or above the pt threshold mapped to that PDG ID
  bool isExcludedParticle(int pdgId, double pt) const;

  /// true if any ancestor of this (stable) particle is itself excluded per
  /// isExcludedParticle(), e.g. a stable pion from a tau decay is excluded
  /// if the (non-stable) tau ancestor's own |pdgId|/pt match
  /// m_excludedPdgIdPtMin. Walks the full decay chain up to the beam
  /// particles, not just the immediate parent.
  bool hasExcludedAncestor(const xAOD::TruthParticle* part) const;

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  // Minimum pT for a track to count
  Gaudi::Property<double> m_Ptmin{this, "Ptcut", 50.0};

  // Maximum |pseudorapidity| for a track to count
  Gaudi::Property<double> m_EtaRange{this, "Etacut", 2.5};

  // Minimum number of tracks
  Gaudi::Property<double> m_NTracks{this, "NTracks", 40};

  // Map of |pdgId| -> minimum pt (MeV) for particles to exclude from the
  // charged-track count (matched by absolute value, so a key excludes both
  // charge states), e.g. {23: 5000.0} excludes Z bosons with pt >= 5 GeV.
  // A particle is excluded from the count if either it, or any of its
  // ancestors in the decay chain (e.g. a decayed tau parent of a stable
  // pion), matches an entry with pt at or above the mapped threshold.
  Gaudi::Property<std::map<int, double>> m_excludedPdgIdPtMin{this, "ExcludedPdgIdPtMin", {}};

};

#endif
