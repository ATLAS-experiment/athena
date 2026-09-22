#ifndef ACTSTRK_TRACKMONITORTOOL_H
#define ACTSTRK_TRACKMONITORTOOL_H

/*
  Track state print interface
  - thread local storage to point to temporary track buffer
  - associate seed to truth particle
  - buffer track per truth particle on drop or keep and keep best match and best "kept" particle
  - on finalize (and on next seed) dump all buffered tracks to ntuple.
 */
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include <set>
#include "ActsTruth/ElasticDecayUtil.h"
#include "ActsInterop/StatUtils.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "Gaudi/Property.h"

#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/Surfaces/BoundaryTolerance.hpp"
// extrapolation with direct navigation
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Utilities/Logger.hpp"

#include "ITrackFindingMonitorTool.h"
#include "TruthTrajectory.h"

#include "ROOT/RNTupleWriter.hxx"
#include "TFile.h"
using ROOT::RNTupleWriter;

#include <cstdint>
#include <limits>
#include <utility>
#include <unordered_map>

class SCT_ID;
class PixelID;
class HGTD_ID;
namespace ActsTrk
{

  namespace detail {
     using RecoTrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                                     Acts::VectorMultiTrajectory>;
  }

  class TrackMonitorTool : public extends<AthAlgTool, ITrackFindingMonitorTool>
  {
  public:
     using base_class::base_class;
     // container used during the reconstructions

     //    using AthAlgTool::AthAlgTool;
    virtual ~TrackMonitorTool() = default;

    // standard Athena methods
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    virtual void newEvent(const EventContext &ctx,
                          const Acts::GeometryContext &tgContext) const override;

    virtual void measurements(const EventContext &ctx,
                              const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                              const std::vector<size_t> &offsets) const override;

    virtual void
    newSeed(const Acts::GeometryContext &tgContext,
            const ActsTrk::Seed &seed,
            const Acts::BoundTrackParameters &initialParameters,
            const detail::MeasurementIndex &measurementIndexer,
            unsigned int iseed,
            bool isKF,
            const char *seedType,
            bool first_seed) const override;

    using track_container_t = detail::RecoTrackContainer;

    virtual void
    newTrack(const Acts::GeometryContext &tgContext,
             const track_container_t &tracks,
             const typename track_container_t::TrackProxy &track,
             const detail::MeasurementIndex &measurementIndexer,
             bool rejected = false) const override;

    using track_state_proxy_t = detail::RecoTrackContainer::TrackStateProxy;
    using const_track_state_proxy_t = detail::RecoTrackContainer::ConstTrackStateProxy;
    virtual bool
    newTrackState(const Acts::GeometryContext &tgContext,
                  const track_container_t &track_container,
                  const typename track_container_t::TrackProxy &track,
                  const track_state_proxy_t &state,
                  const detail::MeasurementIndex &measurementIndexer,
                  unsigned int status,
                  bool useFiltered = false,
                  bool newLine = true) const override;

    virtual void finalizeEvent(const EventContext &ctx) const override;

    struct EventData;
  private:
     void addTrack(const ActsTrk::TrackMonitorTool::track_container_t &tracks,
                   const typename ActsTrk::TrackMonitorTool::track_container_t::TrackProxy &track,
                   unsigned int status) const;
     void cleanupEventData() const;
     void countTruthContribution(ActsTrk::TrackMonitorTool::EventData &event_data,
                                 const xAOD::UncalibratedMeasurement &measurement,
                                 std::vector<std::pair<unsigned int, unsigned int> > &truth_counts) const;
     static std::pair<unsigned int, unsigned int> findBestTruth(std::vector<std::pair<unsigned int, unsigned int> > &truth_counts);
     unsigned int matchType(const track_container_t &track_container,
                            const typename track_container_t::TrackProxy &track,
                            TruthTrajectoryContainer truthTrajectories);
     static unsigned int matchType(const track_container_t &track_container,
                                   const typename track_container_t::TrackProxy &track,
                                   const TruthTrajectoryContainer &truthTrajectories,
                                   unsigned int truth_trajectory_i,
                                   std::vector<bool> &match_cache);

     void storeTracks(const EventContext &ctx, EventData &event_data) const;
  public:
     struct MeasurementData {
        enum EHitType { kUnknown, kCentralHit, kEdgeHit100, kEdgeHit050, kEdgeHit025, kNHitTypes };
        static constexpr std::uint16_t HIT_TYPE_SHIFT = 13;
        static constexpr std::uint16_t TYPE_WORD_SIZE = 16;
        static constexpr std::uint16_t MISSING_SHIFT = 11;
        static constexpr std::uint16_t EXTRAPOL_SHIFT = 9;
        enum EExtrapolStatus {kExtrapolUnknown,  kExtrapolFailed,  kExtrapolOk, kExtrapolBoundCheckFailed};
        enum EHitOnSameSurface {kMatchedHit, kMissingHit,kUnused, kHitOnSameSurface};
        static std::uint16_t setMissing(std::uint16_t measurement_type, bool is_missing=true, bool hit_on_same_surface=false) {
           EHitOnSameSurface missing = (is_missing
                                        ? (hit_on_same_surface ?  kHitOnSameSurface : kMissingHit)
                                        : kMatchedHit);
           return measurement_type | (static_cast<std::uint16_t>(missing)<<MISSING_SHIFT);
        }
        static std::uint16_t setExtrapolationResult(std::uint16_t measurement_type, bool extrapolation_ok, bool boundcheck_ok) {
           EExtrapolStatus status = (extrapolation_ok
                                     ? (boundcheck_ok ? kExtrapolOk : kExtrapolBoundCheckFailed )
                                     : kExtrapolFailed );
           return measurement_type | (static_cast<std::uint16_t>(status)<<EXTRAPOL_SHIFT);
        }

        static std::uint16_t makeType(std::uint16_t measurement_type, EHitType hit_type) {
           static_assert( kNHitTypes < ( 1<<  (TYPE_WORD_SIZE-HIT_TYPE_SHIFT) ));
           assert( measurement_type < (1u<<HIT_TYPE_SHIFT));
           assert( static_cast<std::int16_t>(hit_type) < ( 1<<  (TYPE_WORD_SIZE-HIT_TYPE_SHIFT)) );
           return measurement_type | (static_cast<std::int16_t>(hit_type) << HIT_TYPE_SHIFT);
        }
        EHitType hitType() const {
           return static_cast<EHitType>(type>>HIT_TYPE_SHIFT);
        }

        Acts::Vector3 globalPos;
        const Acts::Surface *surface;
        std::vector<std::int16_t> coordinates;
        std::array<float,4> localCovAndTimeCov;
        std::array<float,3> localPosAndTime;
        std::uint16_t dim;
        std::uint16_t type;
        std::array<std::uint8_t,3> subspaceIndices;
     };
  private:
     void setMeasurementData(const std::vector<const MeasurementToTruthParticleAssociation *> &measurementToTruth,
                             const Acts::GeometryContext &tgContext,
                             const ActsTrk::TruthTrajectoryContainer &truth_trajectories,
                             const std::vector<bool> &matched,
                             unsigned int trajectory_i,
                             std::vector<MeasurementData> &measurement_out) const;
     void setMeasurementData(const Acts::GeometryContext &tgContext,
                             const xAOD::UncalibratedMeasurement *measurement,
                             MeasurementData &measurement_out,
                             bool edge_check) const;
     StatusCode createNtuple();
     void fillNTuple(const EventContext &ctx,EventData &event_data, unsigned int truth_trajectory_i) const;

     void extrapolateToMissing(const EventContext &ctx,
                               const ActsTrk::IExtrapolationTool &extrapolation_tool,
                               ActsTrk::TrackMonitorTool::EventData &event_data,
                               unsigned int matched_track_i,
                               std::vector<ActsTrk::TrackMonitorTool::MeasurementData> &measurementDataOfTrajectory,
                               const std::vector<std::pair<float, unsigned int> > &missingMeasurement,
                               const std::vector<std::pair<float, ActsTrk::TrackMonitorTool::const_track_state_proxy_t> > &missingMeasurementClosestState,
                               std::vector<std::pair<unsigned int, Acts::BoundTrackParameters> > &missingMeasurementExtrapolation) const;

     Acts::Result<Acts::BoundTrackParameters> directExtrapolation(ActsTrk::TrackMonitorTool::EventData &event_data,
                                                                  [[maybe_unused]] const EventContext &ctx,
                                                                  const Acts::BoundTrackParameters& startParameters,
                                                                  const Acts::Surface& target,
                                                                  const Acts::Direction navDir,
                                                                  const double pathLimit) const;

     SG::ReadHandleKeyArray<MeasurementToTruthParticleAssociation>  m_measurementToTruth
        {this, "ClustersToTruthAssociationMap", {}, "Association map from strip measurements to generator particles." };
     SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer>  m_measurementMask
        {this, "MeasurementsForMasking", {}, "Use given measurement containers to deslect measurements which are not in the given container." };
     SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_magneticFieldKey
        {this, "MagneticContextKey", "fieldCondObj"};

     Gaudi::Property<unsigned int> m_minHitsForTruthTrajectory
        {this, "MinHitsForTruthTrajectory",5,"Minimum number of measurements for a truth particle to count as a truth trajectory."};
     Gaudi::Property<float> m_maxEnergyLossElasticDecay
        {this, "MaxEnergyLossElasticDecay",1e12,"Maximum energy loss of an elastic decay."};
     Gaudi::Property<std::string> m_stripIDName
        {this, "StripID","SCT_ID"};
     Gaudi::Property<std::string> m_pixelIDName
        {this, "PixelID","PixelID"};
     Gaudi::Property<std::string> m_hgtdIDName
        {this, "HgtdID","HGTD_ID"};
     Gaudi::Property<std::string> m_ntupleName
        {this, "NTupleFileName",""};

     Gaudi::Property<double> m_ptLoopers{this, "PtLoopers", 300, "PT loop protection threshold. Will be converted to Acts MeV unit"};
     Gaudi::Property<double> m_maxStepSize{this, "MaxStepSize", 10, "Max step size in Acts m unit"};
     Gaudi::Property<unsigned> m_maxStep{this, "MaxSteps", 100000, "Max number of steps"};
     Gaudi::Property<unsigned> m_maxSurfSkip{this, "MaxSurfaceSkip" ,100, "Maximum number of surfaces to be tried by the navigator"};
     Gaudi::Property<double> m_surfTolerance{this, "OnSurfaceTolerance", Acts::s_onSurfaceTolerance,
                                             "Tolerance to consider track parameters on surface"};

     ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
     ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };

     ElasticDecayUtil<> m_elasticDecay;
     ActsTrk::detail::xAODUncalibMeasSurfAcc m_surfAcc{};
     const SCT_ID *m_stripID{};
     const PixelID *m_pixelID{};
     const HGTD_ID *m_hgtdID{};
     std::unique_ptr<const Acts::Logger> m_logger{nullptr};

  public:
     struct EventData {
        EventData(Acts::MagneticFieldContext &&magneticFieldContext,
                  std::vector<const MeasurementToTruthParticleAssociation *> &&measurementToTruth,
                  TruthTrajectoryContainer &&truth_trajectories)
           : m_magneticFieldContext(magneticFieldContext),
             m_actsTracksContainer(m_actsTrackBackend, m_actsTrackStateBackend),
             m_measurementToTruth(std::move(measurementToTruth)),
             m_truthTrajectories(std::move(truth_trajectories))
        {}
        static EventData &currentEventData();

        Acts::MagneticFieldContext m_magneticFieldContext;
        Acts::VectorTrackContainer  m_actsTrackBackend;
        Acts::VectorMultiTrajectory m_actsTrackStateBackend;
        detail::RecoTrackContainer  m_actsTracksContainer;

        std::vector<const MeasurementToTruthParticleAssociation *> m_measurementToTruth;
        TruthTrajectoryContainer m_truthTrajectories;

        const xAOD::TruthParticle *m_seedTruthParticle=nullptr;               //< truth particle associated to current seed
        std::vector<std::pair<unsigned int, unsigned int> > m_seedCandidates; //< seed count per truth particle and
                                                                              //< max number of hits from this truth particle
        std::vector<std::pair<unsigned int,unsigned int> > m_seedAssociatedTruthCache{}; //< cache to count hits per truth particle
        std::vector<std::pair<unsigned int,unsigned int> > m_trackTruthAssociationCache{}; //< cache to count hits per truth particle
        std::vector<bool> m_matchCache;                                       //< cache to compute match type
        std::vector<MeasurementData> m_measurementDataCache;                  //< will be filled with measurement data (truth and unmatched)
        std::vector<const xAOD::UncalibratedMeasurement *> m_measurementCache;//< will be filled with pointers to measurements for unmatched measurements on track
        std::unordered_map<std::uint64_t, unsigned int> m_hitsOnSurfaceCache;//< hit counts per surface

        std::vector<std::pair<float, unsigned int> > m_missingMeasurementCache; //< measurement index for measurements not on track
        std::vector<std::pair<float, const_track_state_proxy_t> > m_missingMeasurementClosestStateCache;//< closest track state to a missing measurement
        std::vector<std::pair<Acts::Vector3,Acts::Vector3 >  > m_stateGlobalPosDirCache; //< global position per track state
        std::vector<std::pair<unsigned int, Acts::BoundTrackParameters> > m_missingMeasurementExtrapolation; //< index of missing measurement and extrapolated parameters

        std::vector<std::pair<float, unsigned int> > m_measurementOrderCache; //< will be filled with radius and measurement data index for sorting
        std::vector<unsigned int> m_measurementIndexCache;                    //< will be filled per track state with the measurement data index
        std::vector<unsigned int> m_measurementReverseIndexCache;             //< will be filled per measurement data with ordered index
        enum EStateStat {kNHoles,kNMaterial,kNMeasurements, kNOther, kNStateStat};
        std::vector<std::pair< unsigned int, std::array<std::uint8_t, kNStateStat> > > m_stateStat;


        // two entries per truth trajectory best kept and best dropped
        struct BestMatchCache {
           static constexpr unsigned int g_invalidTrack = std::numeric_limits<unsigned int>::max();
           std::vector<unsigned int> trackIndex;   //< index of kept (even), dropped (odd) tracks per truth particle
           std::vector<unsigned int> truthCount;   //< number of hits associated to same truth particle even(kept), odd(dropped)
           enum ETrackMatchType {kGoodMatch=128,
                                 kNearPerfectMatch=256,
                                 kPerfectMatch=512};
           std::vector<unsigned int> status;         //< track status per truth particle even(kept), odd(dropped)
           std::vector<unsigned int> candidateCount; //< number of candidate tracks per truth particle even(kept), odd(dropped)
           std::vector<std::array<std::uint8_t, kNStateStat> > stateStat;
        } m_bestMatchCache;
        std::vector<std::pair<unsigned int, unsigned int> > m_trackStatus; //< status from newTrackState if state is marked final

        const Acts::GeometryContext *m_currentGeometryContext{};
        MagField::AtlasFieldCache fieldCache{};

        unsigned int m_currentEventNumber{}; //< current event number from event context
        static inline unsigned int makeSeedId(unsigned int seed_type, unsigned int iseed) {
           assert(seed_type < (1u<<4) && iseed < (1u<<28));
           return (seed_type << 28) | iseed;
        }
        unsigned int m_currentSeed{};    //< index of current seed
        unsigned int m_unmatched{};      //< number of tracks not associated to any truth particle

        bool m_dynamicColumnsInitialized{}; //< indicates whether the dynamic columns of the track container have been initialized

        static std::vector<std::unique_ptr<EventData> > s_eventData ATLAS_THREAD_SAFE;
        static std::mutex s_mutex;
     };
     struct Tuple {
        std::shared_ptr<unsigned int> eventId;
        // truth based
        std::shared_ptr<unsigned int> truthTrajectoryId;
        std::shared_ptr<int> pdgId;
        std::shared_ptr<unsigned int> uniqueId;
        std::shared_ptr<std::array<float,3> > beginMomentum;
        std::shared_ptr<std::array<float,3> > endMomentum;
        std::shared_ptr< std::array<float,3> > beginVtx;
        std::shared_ptr<std::array<float,3> > endVtx;
        // match stat
        std::shared_ptr<unsigned int> nExpectedMeasurements;
        std::shared_ptr<unsigned int> nSeeds;
        std::shared_ptr<unsigned int> maxSeedTruthCount;
        std::shared_ptr<unsigned int> nTracks;
        std::shared_ptr<unsigned int> nDroppedTracks;
        std::shared_ptr<unsigned int> maxTrackTruthCount;
        std::shared_ptr<unsigned int> bestTrackStatus;
        std::shared_ptr<std::uint8_t> nEdgeHits1000;
        std::shared_ptr<std::uint8_t> nEdgeHits0500;
        std::shared_ptr<std::uint8_t> nEdgeHits0250;
        // measurements
        std::shared_ptr<std::vector<std::array<float,3> > > measurementLocalPosAndTime;
        std::shared_ptr<std::vector<std::array<float,4> > > measurementLocalCovAndTimeCov;
        std::shared_ptr<std::vector<std::array<float,3> > > measurementGlobalPos;
        std::shared_ptr<std::vector<std::array<float,3> > > measurementBField;
        std::shared_ptr<std::vector<std::uint16_t> > measurementType;
        std::shared_ptr<std::vector<std::uint64_t> > measurementGeoId; //or identifier ?
        std::shared_ptr<std::vector<std::uint16_t> > measurementCoordinateDim;
        std::shared_ptr<std::vector<unsigned int> > measurementCoordinateOffset; //or identifier ?
        std::shared_ptr<std::vector<std::int16_t> > measurementCoordinates; //or identifier ?

        //        std::shared_ptr<unsigned int> truthCount;
        std::shared_ptr<std::vector<unsigned int> > measurementIndex;
        std::shared_ptr<std::vector<std::array<float,2> > > localPrediction;
        std::shared_ptr<std::vector<std::array<float,3> > > localPredictionCov;
        std::shared_ptr<std::vector<std::array<float,2> > > localFiltered;
        std::shared_ptr<std::vector<std::array<float,3> > > localFilteredCov;
        std::shared_ptr<std::vector<unsigned int> > calibratedDim;
        std::shared_ptr<std::vector<float> > calibrated;
        std::shared_ptr<std::vector<float> > calibratedCov;
        std::shared_ptr<std::vector<std::array<float,3> > > predictedMomentum;
        std::shared_ptr<std::vector<std::array<float,6> > > predictedMomentumCov;
        std::shared_ptr<std::vector<std::array<float,3> > > filteredMomentum;
        std::shared_ptr<std::vector<std::array<float,6> > > filteredMomentumCov;
        std::shared_ptr<std::vector<std::array<float,3> > > stateBField;
        std::shared_ptr<std::vector<std::size_t> > stateGeoId;
        enum EClassification {Other, TruthMatch, ConfusedHit, Hole, ConfusedHole};
        std::shared_ptr< std::vector<unsigned int> > classification;
        std::shared_ptr< std::vector<float> > chi2;
        std::shared_ptr<std::vector<unsigned int> > measurementIndexMissing;
        std::shared_ptr<std::vector<std::array<float,2> > > localPredictionMissing;
        std::shared_ptr<std::vector<std::array<float,3> > > localPredictionMissingCov;
        std::shared_ptr<std::vector<std::array<float,3> > > predictedMomentumMissing;
        std::shared_ptr<std::vector<std::array<float,6> > > predictedMomentumMissingCov;
        std::shared_ptr<std::vector<float> > predictedMissingChi2;
     };

     std::array<std::pair<Acts::BoundaryTolerance,MeasurementData::EHitType>,3 > m_sortedTolerances{
        std::make_pair(Acts::BoundaryTolerance::AbsoluteEuclidean(-1.),  MeasurementData::kEdgeHit100),
        std::make_pair(Acts::BoundaryTolerance::AbsoluteEuclidean(-.5),  MeasurementData::kEdgeHit050),
        std::make_pair(Acts::BoundaryTolerance::AbsoluteEuclidean(-.25), MeasurementData::kEdgeHit025)
     };

     mutable std::mutex m_tupleMutex;
     Tuple m_tuple  ATLAS_THREAD_SAFE;
     mutable std::unique_ptr<TFile> m_file ATLAS_THREAD_SAFE;
     mutable std::unique_ptr<RNTupleWriter> m_writer ATLAS_THREAD_SAFE;
     mutable std::mutex m_statMutex;
     mutable ActsUtils::Stat m_candidatesForPerfectMatch ATLAS_THREAD_SAFE;
     mutable ActsUtils::Stat m_droppedCandidatesForPerfectMatch ATLAS_THREAD_SAFE;
     mutable std::size_t m_nPerfectCandidates  ATLAS_THREAD_SAFE{};
     mutable std::size_t m_nTruthTrajectories  ATLAS_THREAD_SAFE{};
     mutable std::size_t m_nNoTrack  ATLAS_THREAD_SAFE{};

     static unsigned int getSeedType(const char *seedType);
     static std::mutex s_seedTypeMutex;
     static std::unordered_map<std::string, unsigned int> s_seedTypeMap ATLAS_THREAD_SAFE;
  };
}

#endif
