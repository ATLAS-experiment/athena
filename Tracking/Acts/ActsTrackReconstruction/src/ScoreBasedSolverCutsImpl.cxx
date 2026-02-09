/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ScoreBasedSolverCutsImpl.h"

// ACTS
#include "Acts/AmbiguityResolution/ScoreBasedAmbiguityResolution.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/TrackSummaryContainer.h"

unsigned int remapLayer(unsigned int iVolume, unsigned int iLayer) {

  // Barrel region inner + outer
  if ((iVolume == 9) || (iVolume == 16)) {
    return iVolume * 100 + iLayer;
  }

  unsigned int outerEndsCapVolumes[6] = {13, 14, 15, 18, 19, 20};

  // endcap outer
  if (std::find(std::begin(outerEndsCapVolumes), std::end(outerEndsCapVolumes),
                iVolume) != std::end(outerEndsCapVolumes)) {
    return iVolume * 100;
  }

  unsigned int innerEndsCapVolumes[2] = {8, 10};

  // TODO : check if the bins a re correct.
  unsigned int layerIDBins[4] = {0, 30, 40, 60};

  // endcap inner
  // TODO : replace this logic with one that depend on radius.
  if (std::find(std::begin(innerEndsCapVolumes), std::end(innerEndsCapVolumes),
                iVolume) != std::end(innerEndsCapVolumes)) {
    auto it = std::upper_bound(std::begin(layerIDBins), std::end(layerIDBins),
                               iLayer);

    // Handle edge cases
    if (it == std::begin(layerIDBins)) {
      // iLayer is less than the first bin (0)
      return iVolume * 100;  // Use the first bin (0)
    }
    if (it == std::end(layerIDBins)) {
      // iLayer is greater than the last bin (100)
      return iVolume * 100 + (std::size(layerIDBins) - 1);  // Use the last bin
    }

    // iLayer is within the range of layerIDBins
    std::size_t layerIDBin = std::distance(std::begin(layerIDBins), it) - 1;
    return iVolume * 100 + layerIDBin;
  }
  return 0;
}

namespace ActsTrk {

namespace ScoreBasedSolverCutsImpl {
// Add the summary information to the track container for OptionalCuts, This
// likely needs to be moved outside ambiguity resolution
trackContainer_t
addSummaryInformation(const ActsTrk::TrackContainer& trackContainer)
{
  Acts::VectorTrackContainer updatedTrackBackend;
  Acts::VectorMultiTrajectory updatedTrackStateBackend;
  Acts::TrackContainer<Acts::VectorTrackContainer,
                       Acts::VectorMultiTrajectory,
                       Acts::detail::ValueHolder> updatedTracksContainer( std::move(updatedTrackBackend),
                                                                          std::move(updatedTrackStateBackend) );
  
  // need centralized function here
  updatedTracksContainer.ensureDynamicColumns(trackContainer);
    
  updatedTracksContainer.addColumn<unsigned int>("nInnermostPixelLayerHits");
  updatedTracksContainer.addColumn<unsigned int>("nSCTDoubleHoles");
  updatedTracksContainer.addColumn<unsigned int>("nContribPixelLayers");

  for (auto track : trackContainer) {
    auto iTrack = track.index();
    auto destProxy = updatedTracksContainer.getTrack(updatedTracksContainer.addTrack());
    destProxy.copyFrom(trackContainer.getTrack(iTrack));
  }

  std::vector<unsigned int> pixelBarrelLayerIDs = {9, 16};

  unsigned int innermostPixelVolumeID = 9;
  unsigned int innermostPixelLayerID = 2;

  for (auto track : updatedTracksContainer) {
    bool doubleFlag = false;
    int nDoubleHoles = 0;
    int nInnermostPixelLayerHits = 0;
    int nContribPixelLayers = 0;

    std::vector<unsigned int> contribPixelLayers = {};

    for (const auto ts : track.trackStatesReversed()) {
      if (!ts.hasReferenceSurface()) {
        continue;
      }

      if (ts.typeFlags().hasMeasurement()) {

        // Check if the volume is the innermost pixel volume
        // Compute the number of hits in the innermost pixel layer
        auto iVolume = ts.referenceSurface().geometryId().volume();
        auto iLayer = ts.referenceSurface().geometryId().layer();
        // 9 is the volume id for the innermost pixel layer in the barrel
        if (iVolume == innermostPixelVolumeID) {
          if (iLayer == innermostPixelLayerID) {
            nInnermostPixelLayerHits++;
          }
        }

        unsigned int remappedLayer = remapLayer(iVolume, iLayer);
        if (remappedLayer != 0) {
          contribPixelLayers.push_back(remappedLayer);
        }
      }

      std::set<unsigned int> uniqueSet(contribPixelLayers.begin(),
                                       contribPixelLayers.end());
      nContribPixelLayers = uniqueSet.size();
      // Check if the track state has a hole flag
      // Compute the number of double holes
      auto iTypeFlags = ts.typeFlags();
      if (!iTypeFlags.isHole()) {
        doubleFlag = false;
      }
      if (iTypeFlags.isHole()) {
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

  return updatedTracksContainer;
}

// This optionalCut removes tracks that have a number of SCT double holes
// greater than 2
void doubleHolesScore(const trackProxy_t &track, double &score) {
  int iDblHoles = 0;
  std::vector<double> factorDblHoles;

  const int maxDblHoles = 3;
  const double goodDblHoles[maxDblHoles + 1] = {1., 0.03, 0.007, 0.0003};
  const double fakeDblHoles[maxDblHoles + 1] = {1., 0.09, 0.09, 0.008};

  for (int i = 0; i <= maxDblHoles; ++i) {
    factorDblHoles.push_back(goodDblHoles[i] / fakeDblHoles[i]);
  }

  Acts::HashedString hash_nMeasSubDet = Acts::hashString("nSCTDoubleHoles");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    iDblHoles = track.component<unsigned int>(hash_nMeasSubDet);

    if (iDblHoles > -1 && maxDblHoles > 0) {
      if (iDblHoles > maxDblHoles) {
        score /= (iDblHoles - maxDblHoles + 1);  // holes are bad !
        iDblHoles = maxDblHoles;
      }
      score *= factorDblHoles[iDblHoles];
    }
  }
}

// This optional score modifies the score based on the number of hits in the
// innermost pixel layer
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

// This optional score modifies the score based on the number of contributing
// pixel layers
void ContribPixelLayersScore(const trackProxy_t &track, double &score) {

  const int maxPixLay = 16;
  double maxScore = 30.0;
  double minScore = 5.00;
  int maxBarrel = 10;
  int minBarrel = 3;
  int minEnd = 5;

  std::vector<double> factorPixLay;

  double step[2] = {(maxScore - minScore) / (maxBarrel - minBarrel),
                    (maxScore - minScore) / (maxPixLay - minEnd)};

  for (int i = 0; i <= maxPixLay; i++) {
    if (i < minEnd)
      factorPixLay.push_back(0.01 + (i * 0.33));
    else
      factorPixLay.push_back(minScore + ((i - minEnd) * step[1]));
  }

  int iPixLay = 0;

  Acts::HashedString hash_nMeasSubDet = Acts::hashString("nContribPixelLayers");
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

// This optional score modifies the score based on the number of SCT and pixel
// hits
void nSCTPixelHitsScore(const trackProxy_t &track, double &score) {

  const int maxHits = 26;
  double maxScore = 40.0;
  double minScore = 5.00;
  int maxBarrel = 25;
  int minBarrel = 9;
  int minEnd = 12;

  int iHits = track.nMeasurements();  // This includes SCT, Pixel and HGTD hits

  std::vector<double> factorHits;

  double step[2] = {(maxScore - minScore) / (maxBarrel - minBarrel),
                    (maxScore - minScore) / (maxHits - minEnd)};

  for (int i = 0; i <= maxHits; i++) {
    if (i < minEnd)
      factorHits.push_back(0.01 + (i * 1.0 / minEnd));
    else
      factorHits.push_back(minScore + ((i - minEnd) * step[1]));
  }
  if (iHits > -1 && maxHits > 0) {
    if (iHits > maxHits) {
      score *= (iHits - maxHits + 1);  // hits are good !
      iHits = maxHits;
    }
    score *= factorHits[iHits];
  }
}

// This optional cut removes tracks based on the eta dependent cuts
bool etaDependentCuts(const trackProxy_t &track) {
  auto eta = Acts::VectorHelpers::eta(track.momentum());
  auto parm = track.parameters();

  std::vector<double> etaBins = {-4.0, -2.6, -2.0, 0.0, 2.0, 2.6, 4.0};
  std::vector<double> maxD0 = {10.0, 2.0, 2.0, 2.0, 2.0, 10.0};
  std::vector<double> maxZ0 = {200, 200, 200, 200, 200, 200};
  std::vector<unsigned int> maxDoubleHoles = {2, 2, 2, 2, 2, 2};
  std::vector<unsigned int> minSiClusters = {9, 8, 4, 4, 8, 9};

  double maxEta = 4.0;

  auto it = std::upper_bound(etaBins.begin(), etaBins.end(), eta);
  if (it == etaBins.begin() || it == etaBins.end()) {
    return false;  // eta out of range
  }
  std::size_t etaBin = std::distance(etaBins.begin(), it) - 1;

  if (std::abs(eta) > maxEta) {
    return true;
  }

  if (std::abs(parm[Acts::BoundIndices::eBoundLoc1]) > maxZ0[etaBin]) {
    return true;
  }

  if (std::abs(parm[Acts::BoundIndices::eBoundLoc0]) > maxD0[etaBin]) {
    return true;
  }

  unsigned int numSiClusters = track.nMeasurements();

  if (numSiClusters < minSiClusters[etaBin]) {
    return true;
  }

  unsigned int numSCTDoubleHoles = 0;
  Acts::HashedString hash_nMeasSubDet = Acts::hashString("nSCTDoubleHoles");
  if (track.container().hasColumn(hash_nMeasSubDet)) {
    numSCTDoubleHoles = track.component<unsigned int>(hash_nMeasSubDet);
  }

  if (numSCTDoubleHoles > maxDoubleHoles[etaBin]) {
    return true;
  }

  return false;
}

}  // namespace ScoreBasedSolverCutsImpl

}  // namespace ActsTrk
