/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelNNMonitoring/PixelNNMonitorAlg.h
 * @author Max Hart
 * @date July 2026
 * @brief Monitoring of the pixel-cluster neural networks (number and position).
 */

#ifndef PIXELNNMONITORING_PIXELNNMONITORALG_H
#define PIXELNNMONITORING_PIXELNNMONITORALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "SiClusterizationTool/NnClusterizationFactory.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkEventUtils/ClusterSplitProbabilityContainer.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

#include <vector>

class PixelID;
namespace InDetDD { class SiDetectorElement; }

namespace InDet {

/**
 * @brief Performance monitor for the pixel-cluster neural networks.
 *
 * NumberNet: the number-of-particles classification is evaluated with the
 * track estimate on on-track clusters, as in the ambiguity solver, and
 * compared to the Geant4 truth multiplicity on simulation. PositionNet: the
 * per-particle positions and uncertainties are evaluated with the track
 * estimate and compared to the Geant4 truth positions, never to the track.
 * The split-fraction profiles fill on data as well, where the truth curve
 * simply stays empty.
 */
class PixelNNMonitorAlg : public AthMonitorAlgorithm {
public:
  PixelNNMonitorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~PixelNNMonitorAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;

private:
  /// True particle positions (local mm) in the cluster, one per particle,
  /// from the matching Geant4 SiHits on the module.
  std::vector<Amg::Vector2D> truthPositions(
      const InDet::PixelCluster& cluster,
      const InDetDD::SiDetectorElement& element,
      const std::vector<std::vector<const SiHit*>>& siHitsByHash) const;

  ToolHandle<NnClusterizationFactory> m_nnFactory{
    this, "NnClusterizationFactory", "InDet::NnClusterizationFactory/NnClusterizationFactory",
    "NN clusterization factory"};

  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixelClusterKey{
    this, "PixelClusterContainer", "PixelClusters", "Pixel cluster container"};

  SG::ReadHandleKey<TrackCollection> m_trackCollectionKey{
    this, "TrackCollection", "CombinedInDetTracks", "Track collection (position-net input only)"};

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
    this, "BeamSpotKey", "BeamSpotData", "Beam spot data"};

  SG::ReadHandleKey<SiHitCollection> m_siHitKey{
    this, "SiHitCollection", "ITkPixelHits", "Pixel Geant4 hits for truth positions"};

  SG::ReadHandleKey<Trk::ClusterSplitProbabilityContainer> m_splitProbKey{
    this, "ClusterSplitProbContainer", "",
    "Reco cluster split-probability container (isSplit); empty = disabled"};

  ToolHandle<ISiLorentzAngleTool> m_lorentzTool{
    this, "PixelLorentzAngleTool", "SiLorentzAngleTool/PixelLorentzAngleTool",
    "Lorentz angle tool, to reproduce the with-track NN incidence angle"};

  const PixelID* m_pixelID{nullptr};

  Gaudi::Property<bool> m_doTruth{this, "doTruth", true,
    "Use Geant4 SiHit truth for number-net confusion and position residuals/pulls"};

};

} // namespace InDet

#endif
