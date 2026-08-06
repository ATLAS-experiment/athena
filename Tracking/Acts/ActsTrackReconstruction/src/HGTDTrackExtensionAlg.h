/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTDTrackExtensionAlg.h
 *
 * @brief Extends tracks from the inner tracker to the HGTD using the ACTS framework.
 *
 * @details This algorithm retrieves tracks from the inner detector, accesses their last measurement parameters,
 * and extends them into the HGTD using the Combinatorial Kalman Filter (CKF) in the ACTS framework. The information
 * about the extension (e.g. track time) are written as decorations of the TrackParticle associated with it. 
 */

#ifndef HGTDTRACKEXTENSIONALG_H
#define HGTDTRACKEXTENSIONALG_H

// Base Class
#include "TrackFindingBaseAlg.h"

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
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"
#include "ActsGeometry/ActsVolumeIdToDetectorElementCollectionMap.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

// STL
#include <memory>
#include <string>

// Handle Keys
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"
#include "src/detail/TrackFindingMeasurements.h"
#include "src/TrackStatePrinterTool.h"

// Acts
#include "Acts/TrackFinding/TrackStateCreator.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "ActsEvent/TrackParameters.h"
#include "ActsEvent/TrackContainer.h"

// Tools
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"


namespace ActsTrk {

using AtlUncalibSourceLinkAccessor = detail::UncalibSourceLinkAccessor;
using DefaultTrackStateCreator = Acts::TrackStateCreator<ActsTrk::detail::UncalibSourceLinkAccessor::Iterator,detail::RecoTrackContainer>; 

class HGTDTrackExtensionAlg : public TrackFindingBaseAlg {

public:
  using TrackFindingBaseAlg::TrackFindingBaseAlg;
  virtual ~HGTDTrackExtensionAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext &ctx) const override;

  using ExpectedLayerPattern = std::array<unsigned int, 4>;

private:
  // Properties
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerName{this, "TrackParticleContainerName", "InDetTrackParticles", "Name of the TrackParticle container"};
  SG::ReadHandleKey<xAOD::HGTDClusterContainer> m_HGTDClusterContainerName{this, "HGTDClusterContainerName", "HGTD_Clusters", "Name of the HGTD_Cluster container"};

  // Measurement collections
  SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_uncalibratedMeasurementContainerKeys{this, "UncalibratedMeasurementContainerKeys", {}, "input cluster collections"};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_actsTrackLinkKey {this, "ActsTrackLink", m_trackParticleContainerName, "actsTrack", "Link to Acts track"};
  
  // Configuration
  Gaudi::Property<float> m_minEtaAcceptance {this, "MinEtaAcceptance", 2.38, "Minimum eta to consider a track for extension"};
  Gaudi::Property<float> m_maxEtaAcceptance {this, "MaxEtaAcceptance", 4.00, "Maximum eta to consider a track for extension"};

  // Tool Handles
  ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfAcc{};
  

  // WriteDecorHandleKeys for decorating tracks
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_numHGTDHitsKey{this, "numHGTDHits", m_trackParticleContainerName, "numHGTDHits", "Number of HGTD hits on the track extension"};  
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerHasExtensionKey { this, "HGTD_has_extension", m_trackParticleContainerName, "HGTD_has_extension", "Decoration for layer extension" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerExtensionChi2Key { this, "HGTD_extension_chi2", m_trackParticleContainerName, "HGTD_extension_chi2", "Decoration for chi2 of extension" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterRawTimeKey { this, "HGTD_cluster_raw_time", m_trackParticleContainerName, "HGTD_cluster_raw_time", "Decoration for raw time of cluster" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_layerClusterTimeKey { this, "HGTD_cluster_time", m_trackParticleContainerName, "HGTD_cluster_time", "Decoration for cluster time" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapXKey { this, "HGTD_extrap_x", m_trackParticleContainerName, "HGTD_extrap_x", "Decoration for extrapolated X coordinate" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_extrapYKey { this, "HGTD_extrap_y", m_trackParticleContainerName, "HGTD_extrap_y", "Decoration for extrapolated Y coordinate" };
  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_hgtdTrackLinkKey {this, "hgtdTrackLink", m_trackParticleContainerName, "hgtdTrackLink", "Link to Hgtd track"};


  /// @brief Data structure to hold HGTD track extension results
  /// Contains information about hits, timing, and extrapolation for each HGTD layer
  struct TrackExtensionData {
    std::vector<char> hasClusterVec = {false, false, false, false};  ///< Whether extension has cluster in each HGTD layer
    std::vector<float> chi2Vec = {0.0, 0.0, 0.0, 0.0};               ///< Chi2 contribution per HGTD layer
    std::vector<float> rawTimeVec = {0.0, 0.0, 0.0, 0.0};            ///< Raw measured time per HGTD layer
    std::vector<float> timeVec = {0.0, 0.0, 0.0, 0.0};               ///< TOF-corrected time per HGTD layer
    float extrapX = 0.0;                                             ///< Extrapolated X position at HGTD
    float extrapY = 0.0;                                             ///< Extrapolated Y position at HGTD
    float extrapZ = 0.0;                                             ///< Extrapolated Z position at HGTD
    int numHGTDHits = 0;                                             ///< Total number of HGTD hits on extended track
  };

  using TrackFindingBaseAlg::CKF_pimpl;

  Gaudi::Property< float > m_memorySafetyMargin {this, "MemorySafetyMargin", 1.2};
  mutable std::atomic<std::size_t> m_nTrackReserve ATLAS_THREAD_SAFE {0ul};
  mutable std::atomic<std::size_t> m_nTrackStateReserve ATLAS_THREAD_SAFE {0ul};

  /**
    * @brief invoke track finding procedure to extend ITk tracks to HGTD layers
    * using CKF. Extrapolation start with the last hit of ITk track.
    *
    * @param ctx - event context
    * @param detContext - detector context
    * @param measurements - measurements to be used at the extension finding
    * @param measurementIndex - helper with measurement indices
    * @param lastMeasurementStateParameters - State parameters of ITk track last Hit
    * @param tracksContainerTemp - extensions found by CKF (before track selection)
    * @param actsTracksContainer - output extensions container (after selection and aggregated of all event)
    * @param event_stat - stats, just for this event
    * @param refSurface - - reference surface from ITk track, used by extrapolator
    * @param extension_index - index of the found extension at the tracksContainerTemp
    *
    * @return true if a valid extension was found, false otherwise
    */
  bool findExtension(
    const EventContext &ctx,
    const DetectorContextHolder& detContext,
    const detail::TrackFindingMeasurements &measurements,
    const detail::MeasurementIndex& measurementIndex,
    const Acts::BoundTrackParameters & lastMeasurementStateParameters,
    detail::RecoTrackContainer &tracksContainerTemp,
    detail::RecoTrackContainer &actsTracksContainer,
    EventStats& event_stat,
    const Acts::Surface& refSurface,
    int& extension_index) const;

  /**
    * @brief add extension to track container if it passes the track selector criteria
    *
    * @param detContext - detector context
    * @param track - track to be added
    * @param refSurface - reference surface from ITk track, used by extrapolator
    * @param extrapolationStrategy - extrapolation direction
    * @param actsTracksContainer - output extensions container (after selection and aggregated of all event)
    * @param measurementIndex - helper with measurement indices
    * @param tracksContainerTemp - extensions found by CKF (before track selection)
    *
    * @return true if track was accepted and added to container, false otherwise
    */
  bool addTrack(
    const DetectorContextHolder& detContext,
    detail::RecoTrackContainerProxy &track,
    const Acts::Surface& refSurface,
    const Acts::TrackExtrapolationStrategy& extrapolationStrategy,
    detail::RecoTrackContainer &actsTracksContainer,
    const detail::MeasurementIndex& measurementIndex,
    const detail::RecoTrackContainer& tracksContainerTemp) const;
  
  /**
    * @brief it can happen that the last hit of an extension doesn't have a
    * surface associated with it, this function then extrapolates the track
    * to the next valid surface 
    *
    * @param detContext - detector context
    * @param track - track to be extrapolated
    * @param referenceSurface - perigee surface (beamspot)
    * @param propagator - propagator to be used for the extrapolation
    * @param strategy - extrapolation strategy (can define if it is outwards or inwards for example)
    * @param expectedLayerPattern - output of expected layer pattern
    */
  Acts::Result<void> extrapolateTrackToReferenceSurface(
    const DetectorContextHolder& detContext,
    detail::RecoTrackContainerProxy &track,
    const Acts::Surface &referenceSurface,
    const detail::Extrapolator &propagator,
    Acts::TrackExtrapolationStrategy strategy,
    ExpectedLayerPattern& expectedLayerPattern) const;

  /**
    * @brief Create and fills the TrackExtensionData with HGTD hits at the extension.
    * The TrackExtensionData object will later be used to fill the TrackParticle decorations
    *
    * @param trackParticle - trackParticle associated with this hit
    * @param measurements - measurements to be used at the extension finding
    */
  StatusCode collectMeasurements(
    const EventContext& context,
		detail::TrackFindingMeasurements& measurements) const;


  /**
    * @brief Create and fills the TrackExtensionData with HGTD hits at the extension.
    * The TrackExtensionData object will later be used to fill the TrackParticle decorations
    *
    * @param trackParticle - trackParticle associated with this hit
    * @param cluster - HGTD cluster of the hit, used to access its surface and position
    * @param measuredTime - time measured (ns)
    * @param measuredTimeErr - resolution of time measured (ns)
    * @param trackingGeometry - detector geometry
    * @param geoContext - geometry context
    */
  TrackExtensionData processTrackExtension(
    const EventContext& ctx,
    const xAOD::TrackParticle* trackParticle,
    const detail::RecoTrackContainer::TrackProxy& trackProxy,
    const xAOD::HGTDClusterContainer* hgtdClusters) const;

  /**
    * @brief subtracts the time of flight (TOF) from a measured hit time. It uses an estimate
    * of track particle vertex to calculate the lenght of the track up to this hit.
    * The corrected measurement will be the one to be assigned as the time of the track
    *
    * @param trackParticle - trackParticle associated with this hit
    * @param cluster - HGTD cluster of the hit, used to access its surface and position
    * @param measuredTime - time measured (ns)
    * @param measuredTimeErr - resolution of time measured (ns)
    * @param trackingGeometry - detector geometry
    * @param geoContext - geometry context
    */
  std::pair<float, float> correctTOF(
    const xAOD::TrackParticle* trackParticle,
    const xAOD::HGTDCluster* cluster,
    float measuredTime,
    float measuredTimeErr,
    const Acts::TrackingGeometry* trackingGeometry,
    const Acts::GeometryContext& geoContext) const;


  /**
    * @brief Get xAOD::HGTDCluster from track state, so it is possible
    * to retrieve its raw time and position for extension decoration
    *
    * @param ctx - event context
    * @param state - track state of HGTD hit
    */  
  const xAOD::HGTDCluster* getHGTDClusterFromState(
    const EventContext& ctx,
    const ActsTrk::detail::RecoConstTrackStateContainerProxy& state,
    const xAOD::HGTDClusterContainer* hgtdClusters) const;

  /**
    * @brief returns the index of HGTD layer where surfaces lies.
    * This index is used at to locate where in the vectors of
    * TrackExtensionData the hit information should be written
    * Returns 99 if surface is outiside of HGTD.
    *
    * @param geoID - surface geometry id
    */  
  std::size_t getHGTDLayerIndex(
    const Acts::GeometryIdentifier& geoID) const;
                          
};
} // namespace ActsTrk


#endif // HGTDTRACKEXTENSIONALG_H

