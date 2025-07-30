/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H
#define ACTSTRACKRECONSTRUCTION_TRACKFINDINGALG_H

// Base Class
#include "src/TrackFindingBaseAlg.h"

// ACTS
#include "Acts/TrackFinding/TrackStateCreator.hpp"
#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

// ActsTrk
#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "src/detail/AtlasUncalibSourceLinkAccessor.h"

// Athena
#include "GaudiKernel/EventContext.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"
#include "ActsGeometry/ActsVolumeIdToDetectorElementCollectionMap.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

// STL
#include <string>
#include <vector>
#include <optional>

// Handle Keys
#include "StoreGate/CondHandleKeyArray.h"
#include "src/detail/Definitions.h"
#include "src/detail/DuplicateSeedDetector.h"

namespace ActsTrk
{
  using AtlUncalibSourceLinkAccessor = detail::UncalibSourceLinkAccessor;
  using DefaultTrackStateCreator = Acts::TrackStateCreator<ActsTrk::detail::UncalibSourceLinkAccessor::Iterator,detail::RecoTrackContainer>;
  
  class TrackFindingAlg : public TrackFindingBaseAlg
  {
  public:
    
    TrackFindingAlg(const std::string &name,
                    ISvcLocator *pSvcLocator);
    virtual ~TrackFindingAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

    using ExpectedLayerPattern = std::array<unsigned int, 4>;

  private:
    std::size_t getSeedCategory(std::size_t typeIndex,
                                const ActsTrk::Seed& seed,
                                bool useTopSp) const;
    
    void printSeed(unsigned int iseed,
                   const DetectorContextHolder& detContext,
                   const ActsTrk::SeedContainer& seeds,
                   const Acts::BoundTrackParameters &seedParameters,
                   const detail::MeasurementIndex &measurementIndex,
                   std::size_t& nPrinted,
                   const char *seedType,
                   bool isKF = false) const;
    
    StatusCode addTrack(const DetectorContextHolder& detContext,
                        detail::RecoTrackContainerProxy &track,
                        const Acts::Surface& pSurface,
                        const Acts::TrackExtrapolationStrategy& extrapolationStrategy,
                        detail::SharedHitCounter &sharedHits,
                        detail::RecoTrackContainer &actsTracksContainer,
                        const detail::MeasurementIndex& measurementIndex,
                        const detail::RecoTrackContainer& tracksContainerTemp,
                        detail::DuplicateSeedDetector& duplicateSeedDetector,
                        std::vector<int>& destiny,
                        EventStats& event_stat,
                        std::size_t& ntracks,
                        std::size_t iseed,
                        std::size_t category_i,
                        const char *seedType) const;
    
  private:
    // Handle Keys
    // Seed collections. These 2 vectors must match element for element.
    SG::ReadHandleKeyArray<ActsTrk::SeedContainer> m_seedContainerKeys{this, "SeedContainerKeys", {}, "Seed containers"};
    SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_detEleCollKeys{this, "DetectorElementsKeys", {}, "Keys of input SiDetectorElementCollection"};
    // Measurement collections. These 2 vectors must match element for element.
    SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_uncalibratedMeasurementContainerKeys{this, "UncalibratedMeasurementContainerKeys", {}, "input cluster collections"};
    SG::ReadCondHandleKey<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap> m_volumeIdToDetectorElementCollMapKey
      {this, "ActsVolumeIdToDetectorElementCollectionMapKey", "ActsVolumeIdToDetectorElementCollectionMap",
       "Map which associates Acts geometry volume IDs to detector element collections."};
    
    SG::ReadHandleKeyArray<InDet::SiDetectorElementStatus> m_detElStatus
      {this, "DetElStatus", {}, "Keys for detector element status conditions data."};
    
    SG::WriteHandleKeyArray< std::vector<int> > m_seedDestiny {this, "SeedDestiny", {}}; 
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    
    // Configuration
    Gaudi::Property<bool> m_skipDuplicateSeeds{this, "skipDuplicateSeeds", true, "skip duplicate seeds before calling CKF"};
    Gaudi::Property<unsigned int> m_seedMeasOffset{this,"seedMeasOffset", 0, "Reduce the requirement on the space points on seed to mark a seed as duplicate, e.g seedMeasOffset=1, only N-1 measurements on seed are sufficient deduplicate the seed"};
    Gaudi::Property<std::vector<bool>> m_refitSeeds{this, "refitSeeds", {}, "Run KalmanFitter on seeds before passing to CKF, specified separately for each seed collection"};
    Gaudi::Property<std::vector<double>> m_useTopSpRZboundary {this, "useTopSpRZboundary", {350. * Acts::UnitConstants::mm, 1060. * Acts::UnitConstants::mm}, "R/Z boundary for using the top space point in the track parameter estimation"};
    
    
    // Ambiguity resolution
    Gaudi::Property<unsigned int> m_maximumSharedHits
      {this, "MaximumSharedHits", 3u, "Maximum number of shared hits per track."};
    Gaudi::Property<unsigned int> m_maximumIterations
      {this, "MaximumIterations", 10000u, "Maximum number of iterations to resolve ambiguities among all tracks."};
    Gaudi::Property<unsigned int> m_nMeasurementsMin
      {this, "NMeasurementsMin", 7u, "Minimum number of measurements per track."};
    Gaudi::Property<std::size_t> m_ambiStrategy {this, "ambiStrategy", 0, "0 - Do ambiguity resolution outside track finding; 1 - Do ambiguity in track finding using GreedyAmbiguitySolver tool; 2 - Do shared hit cut during track candidate selection"};
    
    
    StatusCode propagateDetectorElementStatusToMeasurements(const ActsTrk::ActsVolumeIdToDetectorElementCollectionMap &volume_id_to_det_el_coll,
                                                            const std::vector< const InDet::SiDetectorElementStatus *> &det_el_status_arr,
                                                            detail::TrackFindingMeasurements &measurements) const;
    
    bool shouldReverseSearch(const ActsTrk::Seed& seed) const;
    
    std::optional<Acts::GreedyAmbiguityResolution> m_ambi;


    Acts::Result<void> extrapolateTrackToReferenceSurface(
      const DetectorContextHolder& detContext,
      detail::RecoTrackContainerProxy &track, 
      const Acts::Surface &referenceSurface,
      const detail::Extrapolator &propagator,
      Acts::TrackExtrapolationStrategy strategy,
      ExpectedLayerPattern& expectedLayerPattern) const;

    /**
     * @brief invoke track finding procedure
     *
     * @param ctx - event context
     * @param detectorElementToGeoId - map Trk detector element to Acts Geometry id
     * @param measurements - measurements container used in MeasurementSelector
     * @param sharedHits - measurements container used for shared hit counting
     * @param duplicateSeedDetector - duplicate seed detector
     * @param seeds - spacepoint triplet seeds
     * @param detElements - Trk detector elements
     * @param actsTracksContainer - output tracks
     * @param seedCollectionIndex - index of this collection of seeds
     * @param seedType - name of type of seeds (strip or pixel) - only used for messages
     * @param event_stat - stats, just for this event
     */
    StatusCode
    findTracks(const DetectorContextHolder& detContext,
               const detail::TrackFindingMeasurements &measurements,
               const detail::MeasurementIndex &measurementIndex,
               detail::SharedHitCounter &sharedHits,
               detail::DuplicateSeedDetector &duplicateSeedDetector,
               const ActsTrk::SeedContainer &seeds,
               const InDetDD::SiDetectorElementCollection& detElements,
               detail::RecoTrackContainer &actsTracksContainer,
               std::size_t seedCollectionIndex,
               const char *seedType,
               EventStats &event_stat,
               std::vector<int>& destiny,
               const Acts::PerigeeSurface& pSurface) const;
    
    // Create tracks from one seed's CKF result, appending to tracksContainer
    void storeSeedInfo(const detail::RecoTrackContainer &tracksContainer,
                       const detail::RecoTrackContainerProxy &track,
                       detail::DuplicateSeedDetector &duplicateSeedDetector,
                       const detail::MeasurementIndex &measurementIndex) const;
    
    using TrackFindingBaseAlg::CKF_pimpl;
    
    enum DestinyType : int {UNKNOWN=0, SUCCEED, DUPLICATE, FAILURE};
  };
  
} // namespace

#endif
