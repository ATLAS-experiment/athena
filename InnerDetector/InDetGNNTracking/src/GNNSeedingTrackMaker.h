/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNNSeedingTrackMaker_H
#define GNNSeedingTrackMaker_H

#include <optional>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/DataHandle.h"

// data containers
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkTrack/TrackCollection.h"

// Tool handles
#include "IGNNTrackReaderTool.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"
#include "InDetRecToolInterfaces/ISeedFitter.h"
#include "InDetRecToolInterfaces/ISiDetElementsRoadMaker.h"
#include "SiSPSeededTrackFinderData/SiCombinatorialTrackFinderData_xk.h"
#include "SiSPSeededTrackFinderData/SiDetElementBoundaryLinks_xk.h"
#include "TrkExInterfaces/IPatternParametersPropagator.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/IBoundaryCheckTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

namespace InDet {
class TrackQualityCuts;
}

namespace InDet {
/**
 * @class InDet::GNNSeedingTrackMaker
 *
 * @brief InDet::GNNSeedingTrackMaker is an algorithm that uses the GNN-based
 * track finding tool to reconstruct tracks and the use track fitter to obtain
 * track parameters. It turns a collection of Trk::Tracks.
 *
 * @author xiangyang.ju@cern.ch
 */
class GNNSeedingTrackMaker : public AthReentrantAlgorithm {
 public:
  GNNSeedingTrackMaker(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

  /// Make this algorithm clonable.
  virtual bool isClonable() const override { return true; };
  //@}

 private:
  /// --------------------
  /// @name Data handles
  /// --------------------
  //@{

  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixelClusterKey{
      this, "PixelClusterKey", "ITkPixelClusters"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_stripClusterKey{
      this, "StripClusterKey", "ITkStripClusters"};

  // input containers
  SG::ReadHandleKey<SpacePointContainer> m_SpacePointsPixelKey{
      this, "SpacePointsPixelName", "ITkPixelSpacePoints"};
  SG::ReadHandleKey<SpacePointContainer> m_SpacePointsStripKey{
      this, "SpacePointsStripName", "ITkStripSpacePoints"};
  //@}
  SG::ReadCondHandleKey<InDet::SiDetElementBoundaryLinks_xk> m_boundaryPixelKey{
      this, "PixelDetElementBoundaryLinks_xk",
      "ITkPixelDetElementBoundaryLinks_xk",
      "Key of InDet::SiDetElementBoundaryLinks_xk for Pixel"};
  SG::ReadCondHandleKey<InDet::SiDetElementBoundaryLinks_xk> m_boundaryStripKey{
      this, "StripDetElementBoundaryLinks_xk",
      "ITkStripDetElementBoundaryLinks_xk",
      "Key of InDet::SiDetElementBoundaryLinks_xk for Strip"};
  // For P->T converter of SCT_Clusters
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{
      this, "AtlasFieldCacheCondObj", "fieldCondObj",
      "Name of the Magnetic Field conditions object key"};
  //@}

  /** @brief Optional read handle to get status data to test whether a pixel
   * detector element is good. If set to e.g. PixelDetectorElementStatus the
   * event data will be used instead of the pixel conditions summary tool.
   */
  SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatus{
      this, "PixelDetElStatus", "", "Key of SiDetectorElementStatus for Pixel"};

  /** @brief Optional read handle to get status data to test whether a Strip
   * detector element is good. If set to e.g. StripDetectorElementStatus the
   * event data will be used instead of the Strip conditions summary tool.
   */
  SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_stripDetElStatus{
      this, "StripDetElStatus", "", "Key of SiDetectorElementStatus for Strip"};

  // output container
  SG::WriteHandleKey<TrackCollection> m_outputTracksKey{this, "TracksLocation",
                                                        "SiSPGNNTracks"};

  /// --------------------
  /// @name Tool handles
  /// --------------------
  //@{
  ToolHandle<IGNNTrackFinder> m_gnnTrackFinder{this, "GNNTrackFinderTool",
                                               "InDet::SiGNNTrackFinderTool",
                                               "Track Finder"};

  /// GNN-based track finding tool that produces track candidates
  ToolHandle<ISeedFitter> m_seedFitter{
      this, "SeedFitterTool", "InDet::InDetGNNTracking/InDetSiSeedFitter",
      "Seed Fitter"};

  ToolHandle<IGNNTrackReaderTool> m_gnnTrackReader{
      this, "GNNTrackReaderTool", "InDet::GNNTrackReaderTool", "Track Reader"};
  /// Track Fitter
  ToolHandle<Trk::ITrackFitter> m_trackFitter{
      this, "TrackFitter", "Trk::GlobalChi2Fitter/InDetTrackFitter",
      "Track Fitter"};

  PublicToolHandle<Trk::IRIO_OnTrackCreator> m_riocreator{
      this, "RIOonTrackTool", "Trk::RIO_OnTrackCreator/RIO_OnTrackCreator"};
  PublicToolHandle<Trk::IPatternParametersPropagator> m_proptool{
      this, "PropagatorTool", "Trk::RungeKuttaPropagator/InDetPropagator"};
  PublicToolHandle<Trk::IPatternParametersUpdator> m_updatortool{
      this, "UpdatorTool", "Trk::KalmanUpdator_xk/InDetPatternUpdator"};

  ToolHandle<Trk::IBoundaryCheckTool> m_boundaryCheckTool{
      this, "BoundaryCheckTool", "InDet::InDetBoundaryCheckTool",
      "Boundary checking tool for detector sensitivities"};
  ToolHandle<IInDetConditionsTool> m_pixelCondSummaryTool{
      this, "PixelSummaryTool", "PixelConditionsSummaryTool"};
  ToolHandle<IInDetConditionsTool> m_stripCondSummaryTool{
      this, "StripSummaryTool",
      "InDetSCT_ConditionsSummaryTool/StripConditionsSummaryTool",
      "Tool to retrieve Strip Conditions summary"};
  ToolHandle<InDet::ISiDetElementsRoadMaker> m_roadmaker{
      this, "RoadTool", "InDet::SiDetElementsRoadMaker_xk"};

  /// @name Other properties
  StringProperty m_fieldmode{this, "MagneticFieldMode", "MapSolenoid",
                             "Mode of magnetic field"};

  Trk::MagneticFieldProperties m_fieldprop;  //!< Magnetic field properties

  void magneticFieldInit();

  static bool spacePointsToClusters(
      const std::vector<const Trk::SpacePoint*>&,
      std::vector<const InDet::SiCluster*>&,
      std::optional<std::reference_wrapper<
          std::vector<const InDetDD::SiDetectorElement*>>> = std::nullopt);

  void detectorElementLinks(
      std::vector<const InDetDD::SiDetectorElement*>& DE,
      std::vector<const InDet::SiDetElementBoundaryLink_xk*>& DEL,
      const EventContext& ctx) const;

  void initializeCombinatorialData(
      const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data) const;

  void getTrackQualityCuts(SiCombinatorialTrackFinderData_xk& data) const;

  // define quality cuts for track selection
  IntegerProperty m_nclusmin{this, "nClustersMin", 6, "Min number clusters"};
  IntegerProperty m_nwclusmin{this, "nWeightedClustersMin", 6,
                              "Min number weighted clusters(pix=2 strip=1)"};
  IntegerProperty m_nholesmax{this, "nHolesMax", 2, "Max number holes"};
  IntegerProperty m_dholesmax{this, "nHolesGapMax", 2, "Max holes gap"};

  DoubleProperty m_pTmin{this, "pTmin", 500., "min pT"};
  DoubleProperty m_pTminBrem{this, "pTminBrem", 1000., "min pT for Brem mode"};
  DoubleProperty m_xi2max{this, "Xi2max", 9., "max Xi2 for updators"};
  DoubleProperty m_xi2maxNoAdd{this, "Xi2maxNoAdd", 25.,
                               "max Xi2 for clusters"};
  DoubleProperty m_xi2maxlink{this, "Xi2maxlink", 200., "max Xi2 for clusters"};
  DoubleProperty m_xi2multitracks{this, "Xi2maxMultiTracks", 9.,
                                  "max Xi2 for multi tracks"};
  IntegerProperty m_doMultiTracksProd{this, "doMultiTracksProd", 0,
                                      "do multi tracks production"};
};

}  // namespace InDet

#endif
