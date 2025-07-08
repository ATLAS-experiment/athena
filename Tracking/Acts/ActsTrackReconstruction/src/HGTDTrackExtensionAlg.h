/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTDTrackExtensionAlg.h
 *
 * @brief Extends tracks from the inner tracker to the HGTD using the ACTS framework.
 *
 * @details This algorithm retrieves tracks from the inner detector, accesses their last measurement parameters,
 * and extends them into the HGTD using the Combinatorial Kalman Filter (CKF) in the ACTS framework.
 */

#ifndef HGTDTRACKEXTENSIONALG_H
#define HGTDTRACKEXTENSIONALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/EventContext.h"

// Athena
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "GeoPrimitives/GeoPrimitives.h"

// STL
#include <memory>
#include <string>

// Handle Keys
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/TrackStatePrinterTool.h"

// Acts
#include "Acts/EventData/TrackContainer.hpp"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"

// Tools
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"


namespace ActsTrk {

class HGTDTrackExtensionAlg : public AthReentrantAlgorithm {

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~HGTDTrackExtensionAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override;
  using CKFOptions = Acts::CombinatorialKalmanFilterOptions<detail::RecoTrackContainer>;

private:
  xAOD::TrackParticle* CKFTrackExtension(const Acts::BoundTrackParameters* parameters);

  // Properties
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerName{this, "TrackParticleContainerName", "InDetTrackParticles", "Name of the TrackParticle container"};
  SG::ReadHandleKey<xAOD::HGTDClusterContainer> m_HGTDClusterContainerName{this, "HGTDClusterContainerName", "", "the HGTD clusters"};
  SG::WriteHandleKey<ActsTrk::TrackContainer> m_trackContainerKey{this, "ACTSTracksLocation", "HGTDExtendedTracks", "Output track collection (ActsTrk variant)"};

  // WriteDecorHandleKeys for decorating tracks
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_timeDecorationKey { this, "TimeDecoration", m_trackParticleContainerName, "time", "Decoration for track time" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey { this, "LayerHasExtension", m_trackParticleContainerName, "layerHasExtension", "Decoration for layer extension" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerExtensionChi2Key { this, "LayerExtensionChi2", m_trackParticleContainerName, "layerExtensionChi2", "Decoration for chi2 of extension" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterRawTimeKey { this, "LayerClusterRawTime", m_trackParticleContainerName, "layerClusterRawTime", "Decoration for raw time of cluster" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey { this, "LayerClusterTime", m_trackParticleContainerName, "layerClusterTime", "Decoration for cluster time" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTruthClassKey { this, "LayerClusterTruthClass", m_trackParticleContainerName, "layerClusterTruthClass", "Decoration for cluster truth classification" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterShadowedKey { this, "LayerClusterShadowed", m_trackParticleContainerName, "layerClusterShadowed", "Decoration for shadowed cluster" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterMergedKey { this, "LayerClusterMerged", m_trackParticleContainerName, "layerClusterMerged", "Decoration for merged cluster" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerPrimaryExpectedKey { this, "LayerPrimaryExpected", m_trackParticleContainerName, "layerPrimaryExpected", "Decoration for primary expected cluster" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapXKey { this, "ExtrapX", m_trackParticleContainerName, "extrapX", "Decoration for extrapolated X coordinate" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapYKey { this, "ExtrapY", m_trackParticleContainerName, "extrapY", "Decoration for extrapolated Y coordinate" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapZKey { this, "ExtrapZ", m_trackParticleContainerName, "extrapZ", "Decoration for extrapolated Z coordinate" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_numHGTDHitsKey{this, "numHGTDHits", m_trackParticleContainerName, "numHGTDHits", "Number of HGTD hits on the track extension"};

  // Tool Handles
  ToolHandle<GenericMonitoringTool> 
      m_monTool{this, "MonTool", "", "Monitoring tool"};
  ToolHandle<ActsTrk::ITrackingGeometryTool> 
      m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  ToolHandle<IActsExtrapolationTool> 
      m_extrapolationTool{this, "ExtrapolationTool", ""};
  
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_pixelCalibTool{this, "PixelCalibrator", "", "Opt. pixel measurement calibrator"};
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_stripCalibTool{this, "StripCalibrator", "", "Opt. strip measurement calibrator"};
  ToolHandle<ActsTrk::IOnTrackCalibratorTool<detail::RecoTrackStateContainer>>
      m_hgtdCalibTool{this, "HGTDCalibrator", "", "Opt. HGTD measurement calibrator"}; 

  ToolHandle<ActsTrk::TrackStatePrinterTool> 
      m_trackStatePrinter{this, "TrackStatePrinter", "", "optional track state printer"};

  ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};

  ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfAcc{};

  //HGTD
  SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_uncalibratedMeasurementContainerKey_HGTD{this, "UncalibratedMeasurementContainerKey_HGTD", "", "input cluster collections for HGTD"};
  // Removed DetectorElementToActsGeometryIdMap - modern approach uses surface accessor

  // logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  std::unique_ptr<detail::CKF_config> m_trackFinder;

  detail::TrackFindingMeasurements collectMeasurements(
      const EventContext& context) const;

  /// @brief Data structure to hold HGTD track extension results
  /// Contains information about hits, timing, and extrapolation for each HGTD layer
  struct TrackExtensionData {
    std::vector<bool> hasClusterVec = {false, false, false, false};  ///< Whether track has cluster in each HGTD layer
    std::vector<float> chi2Vec = {0.0, 0.0, 0.0, 0.0};             ///< Chi2 contribution per HGTD layer
    std::vector<float> rawTimeVec = {0.0, 0.0, 0.0, 0.0};          ///< Raw measured time per HGTD layer
    std::vector<float> timeVec = {0.0, 0.0, 0.0, 0.0};             ///< TOF-corrected time per HGTD layer
    std::vector<int> truthClassVec = {-1, -1, -1, -1};             ///< Truth classification per HGTD layer
    std::vector<bool> isShadowedVec = {false, false, false, false}; ///< Whether cluster is shadowed per layer
    std::vector<bool> isMergedVec = {false, false, false, false};   ///< Whether cluster is merged per layer
    std::vector<bool> primaryExistsVec = {false, false, false, false}; ///< Whether primary is expected per layer
    float extrapX = 0.0;  ///< Extrapolated X position at HGTD
    float extrapY = 0.0;  ///< Extrapolated Y position at HGTD
    float extrapZ = 0.0;  ///< Extrapolated Z position at HGTD
    int numHGTDHits = 0;  ///< Total number of HGTD hits on extended track
  };

  TrackExtensionData processTrackExtension(
    const EventContext& ctx,
    const xAOD::TrackParticle* trackParticle,
    detail::RecoTrackContainer::TrackProxy& trackProxy) const;

  std::pair<float, float> correctTOF(
    const xAOD::TrackParticle* trackParticle,
    const xAOD::HGTDCluster* cluster,
    float measuredTime,
    float measuredTimeErr,
    const Acts::TrackingGeometry* trackingGeometry,
    const Acts::GeometryContext& geoContext) const;

    const xAOD::HGTDCluster* getHGTDClusterFromState(const ActsTrk::detail::RecoConstTrackStateContainerProxy& state) const;

  Acts::CalibrationContext m_calibrationContext; 

  std::size_t getHGTDLayerIndex(Acts::GeometryIdentifier geoID) const;
  bool isHGTDSurface(Acts::GeometryIdentifier geoID) const;
  bool getExtrapolationPosition(const EventContext& ctx, 
                              const detail::RecoTrackContainer::TrackProxy& track, 
                              float& x, float& y, float& z) const;

};
} // namespace ActsTrk


#endif // HGTDTRACKEXTENSIONALG_H

