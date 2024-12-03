/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ScoreBasedSolverCutsImpl.h"

#include "ScoreBasedAmbiguityResolutionAlg.h"

// ACTS
#include "Acts/AmbiguityResolution/ScoreBasedAmbiguityResolution.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackSummaryContainer.h"
#include "ActsInterop/Logger.h"

// Athena
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

// Gaudi
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
namespace ActsTrk {

namespace ScoreBasedSolverCutsImpl {
// Add the summary information to the track container for OptionalCuts, This likely needs to be moved outside ambiguity resolution
ActsTrk::MutableTrackContainer addSummaryInformation(
    ActsTrk::TrackContainer trackContainer) {

  ActsTrk::MutableTrackContainer updatedTracks;
  updatedTracks.ensureDynamicColumns(trackContainer);

  updatedTracks.addColumn<unsigned int>("nInnermostPixelLayerHits");
  updatedTracks.addColumn<unsigned int>("nSCTDoubleHoles");
  updatedTracks.addColumn<unsigned int>("nContribPixelLayers");

  for (auto track : trackContainer) {
    auto iTrack = track.index();
    auto destProxy = updatedTracks.getTrack(updatedTracks.addTrack());
    destProxy.copyFrom(trackContainer.getTrack(iTrack));
  }

  unsigned int pixelVoulmeIds[] = {8, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20};

  for (auto track : updatedTracks) {
    bool doubleFlag = false;
    int nDoubleHoles = 0;
    int nInnermostPixelLayerHits = 0;
    int nContribPixelLayers = 0;
    for (const auto &ts : track.trackStatesReversed()) {
      if (!ts.hasReferenceSurface()) {
        continue;
      }
      // Check if the volume is the innermost pixel layer
      // Compute the number of hits in the innermost pixel layer
      auto iVolume = ts.referenceSurface().geometryId().volume();
      if (iVolume == 8) {
        nInnermostPixelLayerHits++;
      }
      if (std::find(std::begin(pixelVoulmeIds), std::end(pixelVoulmeIds),
                    iVolume) != std::end(pixelVoulmeIds)) {
        nContribPixelLayers++;
      }

      // Check if the track state has a hole flag
      // Compute the number of double holes
      auto iTypeFlags = ts.typeFlags();
      if (!iTypeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
        doubleFlag = false;
      }
      if (iTypeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
        if (doubleFlag) {
          nDoubleHoles++;
          doubleFlag = false;
        } else {
          doubleFlag = true;
        }
      }
    }
    track.template component<unsigned int>(Acts::hashString(
        "nInnermostPixelLayerHits")) = nInnermostPixelLayerHits;
    track.template component<unsigned int>(
        Acts::hashString("nSCTDoubleHoles")) = nDoubleHoles;
    track.template component<unsigned int>(
        Acts::hashString("nContribPixelLayers")) = nContribPixelLayers;
  }

  return updatedTracks;
}

// This optionalCut removes tracks that have a number of SCT double holes greater than 2
bool doubleHolesFilter(const trackProxy_t &track) {

  int n_doubleHoles = 0;
  int max_doubleHoles = 2;
  Acts::HashedString hash_nMeasSubDet =
      Acts::hashString("nSCTDoubleHoles");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    n_doubleHoles = track.component<unsigned int>(hash_nMeasSubDet);
  } else {
    return false;
  }
  if (n_doubleHoles > max_doubleHoles) {
    return true;
  }
  return false;
}

// This optional score modifies the score based on the number of hits in the innermost pixel layer
void innermostPixelLayerHitsScore(const trackProxy_t &track, double &score) {
  int bLayerHits = 0;
  const int maxB_LayerHitsITk = 8;
  auto maxB_LayerHits = maxB_LayerHitsITk;
  const double blayModi[maxB_LayerHitsITk + 1] = {0.25, 4.0, 4.5, 5.0, 5.5,
                                                  6.0,  6.5, 7.0, 7.5};

  std::vector<double> factorB_LayerHits;

  for (int i = 0; i <= maxB_LayerHits; ++i)
    factorB_LayerHits.push_back(blayModi[i]);
  Acts::HashedString hash_nMeasSubDet =
      Acts::hashString("nInnermostPixelLayerHits");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    bLayerHits = track.component<unsigned int>(hash_nMeasSubDet);
  } else {
    return;
  }

  if (bLayerHits > -1 && maxB_LayerHits > 0) {
    if (bLayerHits > maxB_LayerHits) {
      score *= (bLayerHits - maxB_LayerHits + 1);  // hits are good !
      bLayerHits = maxB_LayerHits;
    }
    score *= factorB_LayerHits[bLayerHits];
  }
}

// This optional score modifies the score based on the number of contributing pixel layers
void ContribPixelLayersScore(const trackProxy_t &track, double &score) {

  const int maxPixelLayITk = 16;
  auto maxPixLay = maxPixelLayITk;
  double maxScore = 30.0;
  double minScore = 5.00;
  int maxBarrel = 10;
  int minBarrel = 3;
  int minEnd = 5;

  double step[2] = {(maxScore - minScore) / (maxBarrel - minBarrel),
                    (maxScore - minScore) / (maxPixelLayITk - minEnd)};

  std::vector<double> factorPixLay;
  for (int i = 0; i <= maxPixelLayITk; i++) {
    if (i < minEnd)
      factorPixLay.push_back(0.01 + (i * 0.33));
    else
      factorPixLay.push_back(minScore + ((i - minEnd) * step[1]));
  }

  int iPixLay = 0;
  Acts::HashedString hash_nMeasSubDet =
      Acts::hashString("nContribPixelLayers");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    iPixLay = track.component<unsigned int>(hash_nMeasSubDet);
  } else {
    return;
  }

  if (iPixLay > -1 && maxPixLay > 0) {
    if (iPixLay > maxPixLay) {
      score *= (iPixLay - maxPixLay + 1);  // layers are good !
      iPixLay = maxPixLay;
    }
    score *= factorPixLay[iPixLay];
  }
}

// This optional cut removes tracks based on the eta dependent cuts
// The eta dependent cuts are defined in the InDet::InDetEtaDependentCutsSvc
bool etaDependentCuts(const trackProxy_t &track,ServiceHandle<InDet::IInDetEtaDependentCutsSvc> etaDependentCutsSvc) {
  auto trackEta = Acts::VectorHelpers::eta(track.momentum());
  auto parm = track.parameters();

  double maxEta = etaDependentCutsSvc->getMaxEta();
  if (std::abs(trackEta) > maxEta) {
    return true;
  }

  double maxZ0 = etaDependentCutsSvc->getMaxZImpactAtEta(trackEta);
  if (std::abs(parm[Acts::BoundIndices::eBoundLoc1]) > maxZ0) {
    return true;
  }

  double maxD0 = etaDependentCutsSvc->getMaxPrimaryImpactAtEta(trackEta);

  if (std::abs(parm[Acts::BoundIndices::eBoundLoc0]) > maxD0) {
    return true;
  }

  return false;
}

// This optional cut removes hits from the track based on the pattern track hits
// NOTE: this operation will not be used until the pattern track hits are implemented in ActsTrk
void patternTrackHitSelection(
    const trackProxy_t &track, const trackProxy_t::ConstTrackStateProxy &ts,
    Acts::ScoreBasedAmbiguityResolution::TrackStateTypes &trackStateType) {
  bool ispatternTrack = false;
  Acts::HashedString hash_nMeasSubDet = Acts::hashString("ispatternTrack");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    ispatternTrack = track.component<unsigned int>(hash_nMeasSubDet);
  }
  if (ts.typeFlags().test(Acts::TrackStateFlag::OutlierFlag) &&
      ispatternTrack) {
    trackStateType =
        Acts::ScoreBasedAmbiguityResolution::TrackStateTypes::RejectedHit;
  } else if (ts.typeFlags().test(Acts::TrackStateFlag::OutlierFlag) &&
             !ispatternTrack) {
    // Copy the hit to the track
    trackStateType =
        Acts::ScoreBasedAmbiguityResolution::TrackStateTypes::Outlier;
  }
}

}  // namespace ScoreBasedSolverCutsImpl

}  // namespace ActsTrk
