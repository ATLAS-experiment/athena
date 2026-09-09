#include "GeoPrimitives/GeoPrimitives.h"
#include "TrackMonitorTool.h"

#include "HGTD_Identifier/HGTD_ID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/HGTDCluster.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
// extrapolation with direct navigation
#include "ActsInterop/Logger.h"

#include "ReadoutGeometryBase/SolidStateDetectorElementBase.h"
#include "ReadoutGeometryBase/DetectorDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

#include "Acts/EventData/TransformationHelpers.hpp"

// extrapolation with direct navigator
#include "Acts/Propagator/DirectNavigator.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/ActorList.hpp"

#include <cstdint>

#include "TROOT.h"
#include "TVirtualRWMutex.h"

namespace ActsTrk::detail {
  using RecoTrackContainer = Acts::TrackContainer<Acts::VectorTrackContainer,
                                                  Acts::VectorMultiTrajectory>;
}
namespace {
class GRestore {
public:
   GRestore() : m_globalFile(gFile), m_globalDirectory(gDirectory) {}
   ~GRestore() { gFile=m_globalFile; gDirectory=m_globalDirectory; if (gDirectory) gDirectory->cd(); }
private:
   TFile *m_globalFile;
   TDirectory *m_globalDirectory;
};
}

StatusCode ActsTrk::TrackMonitorTool::initialize() {
   ATH_CHECK(m_measurementToTruth.initialize());
   ATH_CHECK(m_measurementMask.initialize());

   ATH_CHECK(m_trackingGeometrySvc.retrieve());
   m_surfAcc = ActsTrk::detail::xAODUncalibMeasSurfAcc(m_trackingGeometrySvc.get());
   ATH_CHECK(m_magneticFieldKey.initialize());

   ATH_CHECK( m_extrapolationTool.retrieve( DisableTool{m_extrapolationTool.empty()} ));

   if (!m_pixelIDName.empty()) {
      ATH_CHECK( this->detStore()->retrieve(m_pixelID, m_pixelIDName) );
   }
   if (!m_stripIDName.empty()) {
      ATH_CHECK( this->detStore()->retrieve(m_stripID, m_stripIDName) );
   }
   if (!m_hgtdIDName.empty()) {
      ATH_CHECK( this->detStore()->retrieve(m_hgtdID, m_hgtdIDName) );
   }
   m_logger = makeActsAthenaLogger(this, name());

   ATH_CHECK(createNtuple());
   return StatusCode::SUCCESS;
}

StatusCode ActsTrk::TrackMonitorTool::finalize() {
   ATH_MSG_INFO("Perfect matches " << m_nPerfectCandidates << " / " << m_nTruthTrajectories
                << " without track " << m_nNoTrack
                << "\nKept candidates/truth " << m_candidatesForPerfectMatch
                << "\nDropped candites/truth " << m_droppedCandidatesForPerfectMatch);

   if (m_writer) {
      ROOT::TWriteLockGuard root_lock (ROOT::gCoreMutex);
      //TLockGuard root_lock(gROOTMutex);
      m_writer.reset();
      m_file.reset();
   }

   return StatusCode::SUCCESS;
}


std::vector<std::unique_ptr<ActsTrk::TrackMonitorTool::EventData> > ActsTrk::TrackMonitorTool::EventData::s_eventData;
std::mutex ActsTrk::TrackMonitorTool::EventData::s_mutex;

namespace {
   thread_local ActsTrk::TrackMonitorTool::EventData *g_threadEventData ATLAS_THREAD_SAFE=0;
}

std::mutex ActsTrk::TrackMonitorTool::s_seedTypeMutex;
std::unordered_map<std::string, unsigned int> ActsTrk::TrackMonitorTool::s_seedTypeMap;
unsigned int ActsTrk::TrackMonitorTool::getSeedType(const char *seedType) {
   std::lock_guard<std::mutex> lock(s_seedTypeMutex);
   std::pair<std::unordered_map<std::string, unsigned int>::iterator,bool>
      ret=s_seedTypeMap.insert(std::make_pair(std::string( seedType ? seedType : ""), s_seedTypeMap.size()));
   return ret.first->second;
}


inline std::pair<unsigned int, unsigned int> ActsTrk::TrackMonitorTool::findBestTruth(std::vector<std::pair<unsigned int, unsigned int> > &truth_counts) {
   std::pair<unsigned int, unsigned int> max_count_truth{std::numeric_limits<unsigned int>::max(), 0u };
   for (const std::pair<unsigned int, unsigned int> &elm : truth_counts ) {
      if (elm.second>max_count_truth.second) {
         max_count_truth = elm;
      }
   }
   return max_count_truth;
}

namespace {
   void throwNoEventData()  {
      throw std::logic_error("newEvent was not called for this thread.");
   }
}
inline ActsTrk::TrackMonitorTool::EventData &ActsTrk::TrackMonitorTool::EventData::currentEventData() {
   if (!g_threadEventData) {
      throwNoEventData();
   }
   return *g_threadEventData;
}

void ActsTrk::TrackMonitorTool::newEvent(const EventContext &ctx,
                                         const Acts::GeometryContext &tgContext) const {

   std::vector<const ActsTrk::MeasurementToTruthParticleAssociation *> measurement_to_truth
      = getMeasurementToTruthContainer(ctx,m_measurementToTruth);

   std::vector<std::vector<bool> > measurement_mask
      = getMeasurementMask(ctx,m_measurementMask, measurement_to_truth);

   TruthTrajectoryContainer truth_trajectories = getTruthTrajectories(measurement_to_truth,m_minHitsForTruthTrajectory,m_maxEnergyLossElasticDecay,
                                                                      measurement_mask);

   EventData *the_event_data=nullptr;
   {
      std::lock_guard<std::mutex> lock(EventData::s_mutex);
      unsigned int event_data_i=0;
      for (; event_data_i< EventData::s_eventData.size(); ++event_data_i) {
         if (EventData::s_eventData[event_data_i].get() == nullptr) {
            break;
         }
      }
      if (event_data_i>=EventData::s_eventData.size()) {
         EventData::s_eventData.resize(event_data_i+1);
      }
      const AtlasFieldCacheCondObj* field{nullptr};
      if (!SG::get(field, m_magneticFieldKey, ctx).isSuccess() || !field) {
         throw std::runtime_error(std::format("Failed to get magnetic field with key {}", m_magneticFieldKey.fullKey().fullKey().c_str()));
      }
      // @TODO could reuse event data ?
      EventData::s_eventData[event_data_i]=std::make_unique<EventData>(std::move(Acts::MagneticFieldContext(field)),
                                                                       std::move(measurement_to_truth),
                                                                       std::move(truth_trajectories));
      the_event_data=EventData::s_eventData[event_data_i].get();
      the_event_data->m_bestMatchCache.trackIndex.resize( the_event_data->m_truthTrajectories.size() * 2, EventData::BestMatchCache::g_invalidTrack);
      the_event_data->m_bestMatchCache.truthCount.resize( the_event_data->m_truthTrajectories.size() * 2,0u);
      the_event_data->m_bestMatchCache.status.resize( the_event_data->m_truthTrajectories.size() * 2,0u);
      the_event_data->m_bestMatchCache.candidateCount.resize( the_event_data->m_truthTrajectories.size() * 2,0u);
      the_event_data->m_bestMatchCache.stateStat.resize( the_event_data->m_truthTrajectories.size() * 2,std::array<std::uint8_t, EventData::kNStateStat>{} );
      the_event_data->m_seedCandidates.resize( the_event_data->m_truthTrajectories.size(), std::make_pair<unsigned int, unsigned int>(0u,0u));
      the_event_data->m_currentEventNumber=ctx.eventID().event_number();
      the_event_data->m_currentGeometryContext = &tgContext;

      {
         field->getInitializedCache(the_event_data->fieldCache);
      }

      g_threadEventData=the_event_data;
   }
}

void ActsTrk::TrackMonitorTool::measurements([[maybe_unused]] const EventContext &ctx,
                                             [[maybe_unused]] const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                                             [[maybe_unused]] const std::vector<size_t> &offsets) const {
}

void
ActsTrk::TrackMonitorTool::newSeed([[maybe_unused]] const Acts::GeometryContext &tgContext,
                                   const ActsTrk::Seed &seed,
                                   [[maybe_unused]] const Acts::BoundTrackParameters &initialParameters,
                                   [[maybe_unused]] const detail::MeasurementIndex &measurementIndexer,
                                   [[maybe_unused]] unsigned int iseed,
                                   [[maybe_unused]] bool isKF,
                                   [[maybe_unused]] const char *seedType,
                                   [[maybe_unused]] bool first_seed) const  {
   EventData &event_data=EventData::currentEventData();
   event_data.m_currentSeed=EventData::makeSeedId(getSeedType(seedType),iseed);
   event_data.m_seedTruthParticle=nullptr;
   event_data.m_seedAssociatedTruthCache.clear();
   event_data.m_seedAssociatedTruthCache.reserve(seed.sp().size());
   event_data.m_stateStat.clear();
   for (const auto *space_point : seed.sp()) {
      for (const auto *measurement : space_point->measurements()) {
         countTruthContribution(event_data,*measurement,event_data.m_seedAssociatedTruthCache);
      }
   }
   std::pair<unsigned int,unsigned int>  max_count_truth = findBestTruth(event_data.m_seedAssociatedTruthCache);
   ATH_MSG_DEBUG("newSeed ev " << event_data.m_currentEventNumber << " seed " << event_data.m_currentSeed
                << " truth matched hits " << max_count_truth.second
                << " all truth hits " << ( max_count_truth.first<event_data.m_truthTrajectories.size()
                                           ? event_data.m_truthTrajectories.measurements(max_count_truth.first).size()
                                           : 0u )
                );
   if (max_count_truth.first < event_data.m_truthTrajectories.m_truthTrajectoryIndex.size()
       && event_data.m_truthTrajectories.m_truthTrajectoryIndex[max_count_truth.first] < event_data.m_truthTrajectories.size()) {
      unsigned int truth_trajectory_index=event_data.m_truthTrajectories.m_truthTrajectoryIndex[max_count_truth.first];
      event_data.m_seedTruthParticle=event_data.m_truthTrajectories.m_truthParticle[truth_trajectory_index];
      assert(truth_trajectory_index < event_data.m_seedCandidates.size());
      ++event_data.m_seedCandidates[truth_trajectory_index].first;
      event_data.m_seedCandidates[truth_trajectory_index].second=std::max(event_data.m_seedCandidates[truth_trajectory_index].second,max_count_truth.second);
   }
}

void
ActsTrk::TrackMonitorTool::newTrack([[maybe_unused]] const Acts::GeometryContext &tgContext,
                                    const ActsTrk::TrackMonitorTool::track_container_t &tracks,
                                    const typename track_container_t::TrackProxy &track,
                                    [[maybe_unused]] const detail::MeasurementIndex &measurementIndexer,
                                    bool rejected) const {
   EventData &event_data=EventData::currentEventData();
   std::vector<std::pair<unsigned int, unsigned int> >::const_iterator
      track_status_iter = std::find_if(event_data.m_trackStatus.begin(),event_data.m_trackStatus.end(),
                                       [trackIndex=track.index()](const std::pair<unsigned int, unsigned int> &elm) {
                                          return elm.first == trackIndex;
                                       });
   unsigned int status = ((track_status_iter != event_data.m_trackStatus.end()) ? track_status_iter->second : 0u );
   status |= static_cast<unsigned int>(kTrackIsFinal);
   // if (track_status_iter == event_data.m_trackStatus.end()) {
   //    throw std::logic_error("no status for track.");
   // }
   if (track_status_iter != event_data.m_trackStatus.end()) {
      event_data.m_trackStatus.erase(track_status_iter);
   }
   if (rejected) {
      status |= kTrackFailedTrackSelection;
   }
   addTrack(tracks, track, status);
}

using track_state_proxy_t = ActsTrk::detail::RecoTrackContainer::TrackStateProxy;
bool
ActsTrk::TrackMonitorTool::newTrackState([[maybe_unused]] const Acts::GeometryContext &tgContext,
                                         const ActsTrk::TrackMonitorTool::track_container_t &tracks,
                                         const typename ActsTrk::TrackMonitorTool::track_container_t::TrackProxy &track,
                                         [[maybe_unused]] const ActsTrk::TrackMonitorTool::track_state_proxy_t &state,
                                         [[maybe_unused]] const detail::MeasurementIndex &measurementIndexer,
                                         unsigned int status,
                                         [[maybe_unused]] bool useFiltered,
                                         [[maybe_unused]] bool newLine) const {
   EventData &event_data=EventData::currentEventData();
   unsigned int track_index=track.index();
   (void) track_index;
   std::vector<std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > >::iterator
      state_stat_iter=std::find_if(event_data.m_stateStat.begin(),event_data.m_stateStat.end(),
                                   [trackIndex=track.index()](const std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > &elm) {
                                      return elm.first == trackIndex;
                                   });
   if (state_stat_iter == event_data.m_stateStat.end()) {
      event_data.m_stateStat.emplace_back(std::make_pair(track.index(), std::array<std::uint8_t, EventData::kNStateStat>{}));
      state_stat_iter = event_data.m_stateStat.end();
      --state_stat_iter;
   }
   assert (state_stat_iter != event_data.m_stateStat.end() && state_stat_iter->first == track.index());
   const std::array<std::uint8_t, EventData::kNStateStat> &current_counts = state_stat_iter->second;
   (void) current_counts;
   if (state.typeFlags().isHole() || state.typeFlags().hasNoExpectedHit()) {
      ++(state_stat_iter->second[EventData::kNHoles]);
   }
   if (state.typeFlags().isMeasurement() || state.typeFlags().isOutlier()) {
      ++(state_stat_iter->second[EventData::kNMeasurements]);
   }
   if (state.typeFlags().isMaterial()) {
      ++(state_stat_iter->second[EventData::kNMaterial]);
   }
   ++(state_stat_iter->second[EventData::kNOther]);

   if (status & kTrackIsFinal) {
      if (status & gDroppedMask) {
         addTrack(tracks, track, status);
      }
      else {
         //      EventData &event_data=EventData::currentEventData();
         event_data.m_trackStatus.push_back( std::make_pair(track.index(),status));
      }
   }

   return true;
}

void ActsTrk::TrackMonitorTool::addTrack(const ActsTrk::TrackMonitorTool::track_container_t &tracks,
                                         const typename ActsTrk::TrackMonitorTool::track_container_t::TrackProxy &track,
                                         unsigned int status) const {
   if (status & kTrackIsFinal) {
      EventData &event_data=EventData::currentEventData();
      if (!event_data.m_dynamicColumnsInitialized) {
         event_data.m_actsTracksContainer.ensureDynamicColumns(tracks);
         event_data.m_dynamicColumnsInitialized=true;
      }
      event_data.m_trackTruthAssociationCache.clear();
      event_data.m_trackTruthAssociationCache.reserve(16);
      const auto tipIndex = track.tipIndex();
      //      using const_track_state_proxy_t = ActsTrk::TrackMonitorTool::track_container_t::ConstTrackStateProxy;
      tracks.trackStateContainer().visitBackwards(tipIndex,
                                                  [&event_data,this](const const_track_state_proxy_t &state) -> void
         {
            if (state.hasUncalibratedSourceLink()) {
               const xAOD::UncalibratedMeasurement *
                  measurement = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
               countTruthContribution(event_data,*measurement,event_data.m_trackTruthAssociationCache);
            }
         });
      std::pair<unsigned int,unsigned int>  max_count_truth = findBestTruth(event_data.m_trackTruthAssociationCache);
      // only consider tracks which are matched to truth i.e. fake tracks are dropped
      if (max_count_truth.first < event_data.m_truthTrajectories.m_truthTrajectoryIndex.size()
          && event_data.m_truthTrajectories.m_truthTrajectoryIndex[max_count_truth.first] < event_data.m_truthTrajectories.size()) {
         unsigned int truth_trajectory_index=event_data.m_truthTrajectories.m_truthTrajectoryIndex[max_count_truth.first];
         unsigned int best_match_idx = truth_trajectory_index*2;
         if (status & gDroppedMask) {
            ++best_match_idx;
         }
         if (event_data.m_bestMatchCache.truthCount.at(best_match_idx)<max_count_truth.second) {
            // if (event_data.m_bestMatchCache.truthCount.at(best_match_idx)>0u) {
            //    if ((status & gDroppedMask)==0u) {
            //       ++event_data.m_foundBetterMatch;
            //    }
            // }
            status |= matchType(tracks,track, event_data.m_truthTrajectories,truth_trajectory_index, event_data.m_matchCache);

            ATH_MSG_DEBUG("addTrack (better) ev " << event_data.m_currentEventNumber << " seed " << event_data.m_currentSeed
                         << " truth matched hits " << max_count_truth.second << " (was " << event_data.m_bestMatchCache.truthCount.at(best_match_idx)
                         << ", candidates " << event_data.m_bestMatchCache.candidateCount.at(best_match_idx) << ")"
                         << " all truth hits " << event_data.m_truthTrajectories.measurements(truth_trajectory_index).size()
                         << " status " << std::hex << status << std::dec
                         );

            if ((status & EventData::BestMatchCache::kPerfectMatch)==0) {
               auto actsDestProxy   = event_data.m_actsTracksContainer.makeTrack();
               actsDestProxy.copyFrom(track);  // make sure we copy track states!
               event_data.m_bestMatchCache.trackIndex.at(best_match_idx) = actsDestProxy.index();
            }
            else {
               event_data.m_bestMatchCache.trackIndex.at(best_match_idx) = EventData::BestMatchCache::g_invalidTrack;
            }

            event_data.m_bestMatchCache.truthCount.at(best_match_idx) = max_count_truth.second;
            event_data.m_bestMatchCache.status.at(best_match_idx) = status;

            std::vector<std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > >::const_iterator
               state_stat_iter=std::find_if(event_data.m_stateStat.begin(),event_data.m_stateStat.end(),
                                            [trackIndex=track.index()](const std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > &elm) {
                                               return elm.first == trackIndex;
                                            });
            if (state_stat_iter != event_data.m_stateStat.end()) {
               event_data.m_bestMatchCache.stateStat.at(best_match_idx) = state_stat_iter->second;
            }
            //            event_data.m_trackIndexMap.insert( std::make_pair(track.index(),best_match_idx));
         }
         else {
            std::vector<std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > >::const_iterator
               state_stat_iter=std::find_if(event_data.m_stateStat.begin(),event_data.m_stateStat.end(),
                                            [trackIndex=track.index()](const std::pair< unsigned int, std::array<std::uint8_t, EventData::kNStateStat> > &elm) {
                                               return elm.first == trackIndex;
                                            });
            static constexpr std::uint8_t zeroByte{};
            ATH_MSG_INFO("addTrack (worse) ev " << event_data.m_currentEventNumber << " seed " << event_data.m_currentSeed
                         << " truth matched hits " << max_count_truth.second
                         << " all truth hits " << event_data.m_truthTrajectories.measurements(truth_trajectory_index).size()
                         << " status " << std::hex << status << std::dec
                         << " stat : " << (state_stat_iter != event_data.m_stateStat.end() ?   state_stat_iter->second[EventData::kNOther] : zeroByte)
                         << " measurements " << (state_stat_iter != event_data.m_stateStat.end() ?   state_stat_iter->second[EventData::kNMeasurements] : zeroByte)
                         << " holes " << (state_stat_iter != event_data.m_stateStat.end() ?   state_stat_iter->second[EventData::kNHoles] : zeroByte)
                         << " material " << (state_stat_iter != event_data.m_stateStat.end() ?   state_stat_iter->second[EventData::kNMeasurements] : zeroByte)
                         );
            // if ((status & gDroppedMask)==0u) {
            //    ++event_data.m_foundBetterMatch;
            // }
         }
         ++event_data.m_bestMatchCache.candidateCount.at(best_match_idx);
      }
      else  {
         ++event_data.m_unmatched;
      }
   }
}

void ActsTrk::TrackMonitorTool::finalizeEvent(const EventContext &ctx) const {
   EventData &event_data=EventData::currentEventData();
   std::array<unsigned int,2> n_tracks{0u,0u};
   std::array<unsigned int,2> n_candidates{0u,0u};
   unsigned int idx=0;
   for (unsigned int count : event_data.m_bestMatchCache.candidateCount) {
      n_candidates[idx&1] += count;
      ++idx;
   }
   idx=0;
   for (unsigned int track_idx : event_data.m_bestMatchCache.trackIndex) {
      if (track_idx != EventData::BestMatchCache::g_invalidTrack) {
         ++n_tracks[idx&1];
      }
      ++idx;
   }
   ATH_MSG_DEBUG("finalize ev " << event_data.m_currentEventNumber << " seed " << event_data.m_currentSeed
                << " truth trajectories  " << event_data.m_truthTrajectories.size()
                << " candidate tracks " << n_candidates[0] << " dropped " << n_candidates[1]
                << " final " << n_tracks[0] << " dropped " << n_tracks[1]);
   storeTracks(ctx, event_data);
   cleanupEventData();
}

void ActsTrk::TrackMonitorTool::cleanupEventData() const {
   const EventData *current_event_data =g_threadEventData;
   std::lock_guard<std::mutex> lock(EventData::s_mutex);

   for (unsigned int event_data_i=0; event_data_i< EventData::s_eventData.size(); ++event_data_i) {
      if (EventData::s_eventData[event_data_i].get() == current_event_data) {
         EventData::s_eventData[event_data_i].reset();
         break;
      }
   }
   g_threadEventData=nullptr;
}

void ActsTrk::TrackMonitorTool::countTruthContribution(ActsTrk::TrackMonitorTool::EventData &event_data,
                                                       const xAOD::UncalibratedMeasurement &measurement,
                                                       std::vector<std::pair<unsigned int, unsigned int> > &truth_counts) const {
   const unsigned int measurement_type_index = static_cast<unsigned int>(measurement.type());
   assert( measurement_type_index < event_data.m_measurementToTruth.size() );
   if (event_data.m_measurementToTruth[measurement_type_index]) {
      for (const xAOD::TruthParticle *truth_particle_child :  event_data.m_measurementToTruth[measurement_type_index]->at(measurement.index())) {
         const xAOD::TruthParticle *truth_particle_parent = m_elasticDecay.getMother(*truth_particle_child, m_maxEnergyLossElasticDecay);
         std::vector<std::pair<unsigned int, unsigned int> >::iterator
            truth_iter = std::find_if(truth_counts.begin(),truth_counts.end(),
                                      [truth_particle_index=truth_particle_parent->index()](const std::pair<unsigned int, unsigned int> &elm) {
                                         return elm.first == truth_particle_index;
                                      });
         if (truth_iter != truth_counts.end()) {
            // @TODO weight differently depending on measurement degrees of freedom / type ?
            ++truth_iter->second;
         }
         else {
            truth_counts.push_back(std::make_pair(truth_particle_parent->index(), 1u));
         }
      }
   }
}

unsigned int ActsTrk::TrackMonitorTool::matchType(const track_container_t &tracks,
                                                  const typename track_container_t::TrackProxy &track,
                                                  const TruthTrajectoryContainer &truthTrajectories,
                                                  unsigned int truth_trajectory_i,
                                                  std::vector<bool> &match_cache) {
   std::span<const unsigned int> truth_trajectory_measurements  = truthTrajectories.measurements(truth_trajectory_i);
   match_cache.clear();

   match_cache.resize( truth_trajectory_measurements.size(),false);
   unsigned int confused_hits=0u;
   unsigned int matched_hits=0u;
   //   using const_track_state_proxy_t = ActsTrk::TrackMonitorTool::track_container_t::ConstTrackStateProxy;
   tracks.trackStateContainer().visitBackwards(track.tipIndex(),
                                               [&confused_hits,
                                                &matched_hits,
                                                &match_cache,
                                                &truth_trajectory_measurements](const const_track_state_proxy_t &state) {
      if (state.hasUncalibratedSourceLink()) {
         const xAOD::UncalibratedMeasurement *
            measurement = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
         unsigned int measurementId = ActsTrk::TruthTrajectoryContainer::makeMeasurementId(static_cast<unsigned int>(measurement->type()),
                                                                                           measurement->index());
         std::span<const unsigned int>::const_iterator
            truth_measurement_iter = std::find_if(truth_trajectory_measurements.begin(),truth_trajectory_measurements.end(),
                                                  [measurementId](unsigned int a_measurement_id) {
                                                     return (a_measurement_id == measurementId);
                                                  });
         if (truth_measurement_iter != truth_trajectory_measurements.end()) {
            if (match_cache.at( truth_measurement_iter - truth_trajectory_measurements.begin() )) {
               throw std::logic_error("Measurement appears twice");
            }
            match_cache.at( truth_measurement_iter - truth_trajectory_measurements.begin() )=true;
            ++matched_hits;
         }
         else {
            ++confused_hits;
         }
      }
   });
   unsigned denominator = confused_hits + truth_trajectory_measurements.size();
   unsigned numerator =  matched_hits;
   if (denominator == numerator) {
      return EventData::BestMatchCache::kPerfectMatch;
   }
   else if (denominator == numerator+1) {
      return EventData::BestMatchCache::kNearPerfectMatch;
   }
   else if (denominator <= numerator*2) {
      return EventData::BestMatchCache::kGoodMatch;
   }
   return 0u;
}

void ActsTrk::TrackMonitorTool::storeTracks(const EventContext &ctx, EventData &event_data) const {
   unsigned int n_perfect=0u;
   unsigned int n_noTrack=0u;

   ActsUtils::Stat candidatesForPerfectMatch;
   ActsUtils::Stat droppedCandidatesForPerfectMatch;
   assert(event_data.m_bestMatchCache.status.size() == event_data.m_truthTrajectories.size()*2);
   assert(event_data.m_bestMatchCache.candidateCount.size() == event_data.m_truthTrajectories.size()*2);
   assert(event_data.m_bestMatchCache.truthCount.size() == event_data.m_truthTrajectories.size()*2);
   assert(event_data.m_bestMatchCache.trackIndex.size() == event_data.m_truthTrajectories.size()*2);

   for (unsigned int truth_particle_i=0; truth_particle_i<event_data.m_truthTrajectories.size(); ++truth_particle_i) {
      unsigned int matched_track_idx=truth_particle_i*2;
      if (event_data.m_bestMatchCache.status[matched_track_idx] & EventData::BestMatchCache::kPerfectMatch) {
         ++n_perfect;
         candidatesForPerfectMatch.add(event_data.m_bestMatchCache.candidateCount[matched_track_idx]);
         droppedCandidatesForPerfectMatch.add(event_data.m_bestMatchCache.candidateCount[matched_track_idx+1]);
      }
      else {
         fillNTuple(ctx, event_data, truth_particle_i);
      }
   }
   {
      std::lock_guard<std::mutex> lock(m_statMutex);
      m_candidatesForPerfectMatch += candidatesForPerfectMatch;
      m_droppedCandidatesForPerfectMatch += droppedCandidatesForPerfectMatch;
      m_nPerfectCandidates += n_perfect;
      m_nTruthTrajectories += event_data.m_truthTrajectories.size();
      m_nNoTrack += n_noTrack;
   }
}

namespace {
const xAOD::TruthParticle *getLastChild(const xAOD::TruthParticle *parent) {
   const xAOD::TruthParticle *last_child=parent;
   while (parent->hasDecayVtx()) {
      const xAOD::TruthVertex* decay_vertex = parent->decayVtx();
      assert(decay_vertex);
      if (decay_vertex->nOutgoingParticles()==0u) break;
      unsigned int best_child_i=std::numeric_limits<unsigned int>::max();
      double best_angle=-std::numeric_limits<double>::max();
      Acts::Vector3 parent_dir{ parent->p4().X(), parent->p4().Y(), parent->p4().Z()};
      double parent_norm = parent_dir.norm();
      for (unsigned int child_i=0; child_i<decay_vertex->nOutgoingParticles(); ++child_i) {
         const xAOD::TruthParticle* child=decay_vertex->outgoingParticle(child_i);
         assert(child);
         if (child && child->pdgId() == parent->pdgId()) {
            Acts::Vector3 child_dir{ child->p4().X(), child->p4().Y(), child->p4().Z()};
            double child_norm=child_dir.norm();
            double angle = parent_dir.dot(child_dir)/(parent_norm*child_norm);
            if(angle>best_angle) {
               best_angle=angle;
               best_child_i=child_i;
            }
         }
      }
      parent = (best_child_i<decay_vertex->nOutgoingParticles()) ? decay_vertex->outgoingParticle(best_child_i) : nullptr;
      if (!parent) break;
      last_child=parent;
   }
   return last_child;
}
   inline std::array<float, 3> getFloatMomentum(const TLorentzVector &p4) {
      return std::array<float, 3> {
         static_cast<float>(p4.X()),
         static_cast<float>(p4.Y()),
         static_cast<float>(p4.Z())
      };
   }
   inline std::array<float, 3> getVertexPosition(const xAOD::TruthVertex *vertex) {
      assert(vertex);
      return std::array<float, 3> {vertex->x(),vertex->y(),vertex->z()};
   }
   inline std::array<float, 3> getScaledDirection(const TLorentzVector &p4, double scale) {
      Acts::Vector3 dir{p4.X(), p4.Y(),p4.Z() };
      dir *= (scale/dir.norm());
      return std::array<float, 3> {static_cast<float>(dir[0]),
                                   static_cast<float>(dir[1]),
                                   static_cast<float>(dir[2])};
   }


   void getUnmatchedMeasurements(ActsTrk::TrackMonitorTool::EventData &event_data,
                                 unsigned int matched_track_i,
                                 std::vector<const xAOD::UncalibratedMeasurement *> &unmatched_measurements_out,
                                 std::vector<bool> &matched,
                                 std::unordered_map<std::uint64_t, unsigned int> &hitsOnSurface,
                                 std::vector<unsigned int> &measurement_index,
                                 unsigned int &n_local_pos, unsigned int &n_local_cov) {
      unsigned int truth_trajectory_i=matched_track_i/2;
      std::span<const unsigned int> truth_trajectory_measurements=event_data.m_truthTrajectories.measurements(truth_trajectory_i);
      matched.clear();
      matched.resize( truth_trajectory_measurements.size(),false);
      hitsOnSurface.clear();
      hitsOnSurface.reserve( truth_trajectory_measurements.size());
      measurement_index.clear();
      measurement_index.reserve(truth_trajectory_measurements.size());
      n_local_pos=0u;
      n_local_cov=0u;


      using const_track_state_proxy_t = ActsTrk::TrackMonitorTool::track_container_t::ConstTrackStateProxy;
      auto track = event_data.m_actsTracksContainer.getTrack(event_data.m_bestMatchCache.trackIndex.at(matched_track_i));
      event_data.m_actsTracksContainer.trackStateContainer().visitBackwards(track.tipIndex(),
                                                  [&unmatched_measurements_out,
                                                   &match_cache=matched,
                                                   &hitsOnSurface,
                                                   &measurement_index,
                                                   &n_local_pos,
                                                   &n_local_cov,
                                                   &truth_trajectory_measurements](const const_track_state_proxy_t &state) {
        if (state.hasUncalibratedSourceLink()) {
           if (state.hasReferenceSurface()) {
              ++hitsOnSurface[state.referenceSurface().geometryId().value()];
           }
           n_local_pos += state.calibratedSize();
           for (unsigned int dim_i=state.calibratedSize(); dim_i>0; --dim_i) {
              n_local_cov += dim_i;
           }
           const xAOD::UncalibratedMeasurement *
              measurement = ActsTrk::detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
           unsigned int measurementId = ActsTrk::TruthTrajectoryContainer::makeMeasurementId(static_cast<unsigned int>(measurement->type()),
                                                                                             measurement->index());
           std::span<const unsigned int>::const_iterator
              truth_measurement_iter = std::find_if(truth_trajectory_measurements.begin(),truth_trajectory_measurements.end(),
                                                    [measurementId](unsigned int a_measurement_id) {
                                                       return (a_measurement_id == measurementId);
                                                    });
           if (truth_measurement_iter != truth_trajectory_measurements.end()) {
              // if (match_cache.at( truth_measurement_iter - truth_trajectory_measurements.begin() )) {
              //    throw std::logic_error("Measurement appears twice");
              // }
              match_cache.at( truth_measurement_iter - truth_trajectory_measurements.begin() )=true;
              measurement_index.push_back( truth_measurement_iter - truth_trajectory_measurements.begin() );
           }
           else {
              measurement_index.push_back( unmatched_measurements_out.size() + truth_trajectory_measurements.size() );
              unmatched_measurements_out.push_back(measurement);
           }
        }
        else {
           measurement_index.push_back(std::numeric_limits<unsigned int>::max());
        }
      });
   }

   void findClosestTrackState(ActsTrk::TrackMonitorTool::EventData &event_data,
                              unsigned int matched_track_i,
                              const std::vector<std::pair<float, unsigned int> > &missingMeasurementCache,
                              std::vector<std::pair<float, ActsTrk::TrackMonitorTool::const_track_state_proxy_t> > &missingMeasurementClosestStateCache,
                              std::vector<std::pair<Acts::Vector3,Acts::Vector3> > &stateGlobalPosDirCache) {
      using const_track_state_proxy_t = ActsTrk::TrackMonitorTool::const_track_state_proxy_t;
      auto track = const_cast< ActsTrk::detail::RecoTrackContainer &>(event_data.m_actsTracksContainer).getTrack(event_data.m_bestMatchCache.trackIndex.at(matched_track_i));

      stateGlobalPosDirCache.clear();
      missingMeasurementClosestStateCache.clear();
      missingMeasurementClosestStateCache.resize(missingMeasurementCache.size(),
                                                 std::make_pair(std::numeric_limits<float>::max(),
                                                                ActsTrk::TrackMonitorTool::const_track_state_proxy_t(*track.innermostTrackState())));

      event_data.m_actsTracksContainer.trackStateContainer().visitBackwards(track.tipIndex(),
                                                  [&event_data,
                                                   &missingMeasurementClosestStateCache,
                                                   &missingMeasurementCache,
                                                   &stateGlobalPosDirCache](const const_track_state_proxy_t &state) {
            if (state.hasReferenceSurface()) {
               Acts::FreeVector
                  free_param = Acts::transformBoundToFreeParameters(state.referenceSurface(),
                                                                    *event_data.m_currentGeometryContext, state.parameters());
               stateGlobalPosDirCache.push_back(std::make_pair(Acts::Vector3{free_param[Acts::eFreePos0],
                                                                             free_param[Acts::eFreePos1],
                                                                             free_param[Acts::eFreePos2]},
                                                               Acts::Vector3{free_param[Acts::eFreeDir0],
                                                                             free_param[Acts::eFreeDir1],
                                                                             free_param[Acts::eFreeDir2]}));
               float r = static_cast<float>(stateGlobalPosDirCache.back().first.norm());
               for (unsigned int missing_i=0; missing_i<missingMeasurementCache.size(); ++missing_i) {
                  const float missing_r = missingMeasurementCache[missing_i].first;
                  if (std::abs(r-missing_r) < std::abs(missingMeasurementClosestStateCache[missing_i].first-missing_r)) {
                     missingMeasurementClosestStateCache[missing_i].first = r;
                     missingMeasurementClosestStateCache[missing_i].second = state;
                  }
               }
            }
         });
   }


   template <class T> concept isArray = requires(T t) { t.size(); };

   template <std::size_t NDIM, typename T_SubspaceIndex, typename T_SrcArr, typename T_DestVec>
   void pushBackProjection(const T_SubspaceIndex &indices, const T_SrcArr &src, T_DestVec &dest) {
      if constexpr( isArray<typename T_DestVec::value_type> ) {
         dest.emplace_back();
         for (unsigned int i=0; i<NDIM; ++i) {
            assert( i<dest.back().size());
            assert( i<indices.size());
            dest.back()[i]=src[indices[i]];
         }
      }
      else {
         for (unsigned int i=0; i<NDIM; ++i) {
            assert( i<indices.size());
            assert( indices[i]<src.size());
            dest.push_back(src[indices[i]]);
         }
      }
   }
   template <std::size_t NDIM, typename T_SubspaceIndex, typename T_SrcArr, typename T_DestVec>
   void pushBackProjectionCov(const T_SubspaceIndex &indices, const T_SrcArr &src, T_DestVec &dest) {
      if constexpr( isArray<typename T_DestVec::value_type> ) {
         dest.emplace_back();
         unsigned int dest_i=0;
         for (unsigned int i=0; i<NDIM; ++i) {
            assert( i<indices.size());
            for (unsigned int j=0; j<=i; ++j) {
               assert( dest_i<dest.back().size());
               dest.back()[dest_i++]=src( indices[i], indices[j] );
            }
         }
      }
      else {
      for (unsigned int i=0; i<NDIM; ++i) {
         assert( i<indices.size());
         for (unsigned int j=0; j<=i; ++j) {
            dest.push_back( src( indices[i], indices[j]) );
         }
      }
      }
   }

   template <typename T_Cov, typename T_DestVec, typename T_DestCovVec>
   void fillMomentum(const Acts::BoundVector &boundParams, T_Cov boundParamsCov,
                     T_DestVec &momentum_out,
                     T_DestCovVec &momentum_cov_out) {
      double p_over_q = boundParams[Acts::eBoundQOverP] != 0. ? 1./boundParams[Acts::eBoundQOverP] : 1e20;
      Acts::Vector3 momentum = Acts::makeDirectionFromPhiTheta(boundParams[Acts::eBoundPhi],
                                                               boundParams[Acts::eBoundTheta]) * p_over_q;
      if constexpr( isArray<typename T_DestVec::value_type> ) {
         momentum_out.emplace_back();
         assert( momentum_out.back().size() == momentum.rows() );
         for (unsigned int row_i=0; row_i<momentum.rows(); ++row_i) {
            momentum_out.back()[row_i]=momentum[row_i];
         }
      }
      else {
         for (unsigned int row_i=0; row_i<momentum.rows(); ++row_i) {
            momentum_out.push_back(momentum[row_i]);
         }
      }
   }

   template <class T_Matrix>
   static constexpr std::size_t matrixColumns() { return T_Matrix::ColsAtCompileTime;}
   template <class T_Matrix>
   static constexpr std::size_t matrixRows() { return T_Matrix::RowsAtCompileTime;}

   template <typename measurement_vector_t,
             typename measurement_cov_matrix_t,
             typename predicted_vector_t,
             typename predicted_cov_matrix_t>
double computeChi2(const measurement_vector_t &a,
                   const measurement_cov_matrix_t &a_cov,
                   const predicted_vector_t &b,
                   const predicted_cov_matrix_t &b_cov) {
      assert( a.rows() ==  b.rows() && a.cols() == 1 && b.cols()==1);
      assert( a_cov.rows() == a_cov.cols() && a_cov.rows() == b_cov.rows() && a_cov.cols() == b_cov.cols());
      auto inv_ab_cov( (a_cov+b_cov).inverse() );
      auto  diff( a-b);
      return (diff.transpose() * inv_ab_cov * diff)(0,0);
}

template <std::size_t N,class T_ResultType, typename T_SubspaceIndices, class T_Matrix>
T_ResultType project(T_SubspaceIndices parameter_map, const T_Matrix &matrix)
{
   using MatrixIndexMapType = unsigned char; // "char" to reduce the size of the map, and if not wide enough this entire
                                             //        concept is likely inefficient.
   using MatrixIndexType = unsigned int;     // @TODO or std::size_t ? does not matter

   // ensure that index types are wide enough
   static_assert( matrixRows<T_Matrix>() < std::numeric_limits<typename T_SubspaceIndices::value_type>::max());
   static_assert( N*matrixRows<T_Matrix>() < std::numeric_limits<typename T_SubspaceIndices::value_type>::max());

   T_ResultType ret;
   if constexpr(matrixColumns<T_Matrix>() == 1) {
      // handle projection of paramteter vector
      for (MatrixIndexType meas_i=0; meas_i<N; ++meas_i) {
         assert( meas_i < parameter_map.size() );
         ret(meas_i,0) = matrix( parameter_map[meas_i], 0);
      }
   }
   else {
      // handle projection of covariance matrix
      // "project" matrix
      for (MatrixIndexType meas_i=0; meas_i<N; ++meas_i) {
         assert( meas_i < parameter_map.size());
         MatrixIndexType param_i = parameter_map[meas_i];
         for (MatrixIndexType meas_j=0; meas_j<N; ++meas_j) {
            assert( meas_j < parameter_map.size());
            ret(meas_i,meas_j) = matrix(param_i, parameter_map[meas_j]);
         }
      }
   }
   return ret;
}

}

Acts::Result<Acts::BoundTrackParameters> ActsTrk::TrackMonitorTool::directExtrapolation(ActsTrk::TrackMonitorTool::EventData &event_data,
                                                                                        [[maybe_unused]] const EventContext &ctx,
                                                                                        const Acts::BoundTrackParameters& startParameters,
                                                                                        const Acts::Surface& targetSurface,
                                                                                        const Acts::Direction navDir,
                                                                                        const double pathLimit) const {

   // move to initialize
   using CurvedStepper_t = Acts::EigenStepper<Acts::EigenStepperDefaultExtension>;
   using CurvedPropagator_t = Acts::Propagator<CurvedStepper_t, Acts::DirectNavigator>;

   Acts::DirectNavigator navigator{m_logger->clone()};

   auto bField = std::make_shared<ATLASMagneticFieldWrapper>();
   CurvedStepper_t stepper{std::move(bField)};

   using SteppingLogger = Acts::detail::SteppingLogger;
   using EndOfWorld = Acts::EndOfWorldReached;
   using ActorList =
      Acts::ActorList<SteppingLogger, Acts::MaterialInteractor, EndOfWorld>;

   CurvedPropagator_t propagator{std::move(stepper), std::move(navigator),
                                 m_logger->clone()};

   using Options = typename CurvedPropagator_t::template Options<ActorList>;
   using namespace Acts::UnitLiterals;
   Options options(*event_data.m_currentGeometryContext, event_data.m_magneticFieldContext);

   options.pathLimit = pathLimit;
   options.loopProtection
      = (Acts::VectorHelpers::perp(startParameters.momentum())
         < m_ptLoopers * 1_MeV);
   options.maxSteps = m_maxStep;
   options.direction = navDir;
   options.stepping.maxStepSize = m_maxStepSize * 1_m;
   options.maxTargetSkipping = m_maxSurfSkip;
   options.surfaceTolerance = m_surfTolerance;

   options.navigation.appendExternalSurface(targetSurface);

   auto& mInteractor = options.actorList.template get<Acts::MaterialInteractor>();
   mInteractor.multipleScattering = false; // currently there is no material, since the direct navigator will only get the target surface
   mInteractor.energyLoss = false;  //
   mInteractor.recordInteractions = false;


   auto result = propagator.template propagate<Options, Acts::SurfaceReached, Acts::PathLimitReached>(startParameters, targetSurface, options);

   if (!result.ok()) {
      return result.error();
   }
   if (!result.value().endParameters.has_value()) {
      return Acts::PropagatorError::Failure;
   }
   return result.value().endParameters.value();
}


   void ActsTrk::TrackMonitorTool::extrapolateToMissing(const EventContext &ctx,
                             const ActsTrk::IExtrapolationTool &extrapolation_tool,
                             ActsTrk::TrackMonitorTool::EventData &event_data,
                             unsigned int matched_track_i,
                             std::vector<ActsTrk::TrackMonitorTool::MeasurementData> &measurementDataOfTrajectory,
                             const std::vector<std::pair<float, unsigned int> > &missingMeasurement,
                             const std::vector<std::pair<float, ActsTrk::TrackMonitorTool::const_track_state_proxy_t> > &missingMeasurementClosestState,
                             std::vector<std::pair<unsigned int, Acts::BoundTrackParameters> > &missingMeasurementExtrapolation) const
   {
      static constexpr double r_scale = 4*M_PI;
      static constexpr double min_path_length = 100.;
      auto track = const_cast< ActsTrk::detail::RecoTrackContainer &>(event_data.m_actsTracksContainer).getTrack(event_data.m_bestMatchCache.trackIndex.at(matched_track_i));
      for (unsigned int missing_i=0; missing_i<missingMeasurement.size(); ++missing_i) {
         unsigned int measurement_i = missingMeasurement.at(missing_i).second;
         ActsTrk::TrackMonitorTool::MeasurementData &measurement_data = measurementDataOfTrajectory.at(measurement_i);
         if (measurement_data.surface) {
            auto state_param = track.createParametersFromState(missingMeasurementClosestState.at(missing_i).second);
            for (unsigned int dir_i=0; dir_i<2; ++dir_i) {
               Acts::Result<Acts::BoundTrackParameters> extrapolation_result
               // = extrapolation_tool.propagate(
                  = directExtrapolation(event_data,
                                        ctx,
                                        state_param,
                                        *measurement_data.surface,
                                        ((missingMeasurement[missing_i].first>missingMeasurementClosestState[missing_i].first) ^ dir_i
                                         ? Acts::Direction::Forward()
                                         : Acts::Direction::Backward()),
                                        std::abs(missingMeasurement[missing_i].first-missingMeasurementClosestState[missing_i].first)*r_scale
                                        + min_path_length
                                        );
               Acts::FreeVector
                  free_param = (extrapolation_result.ok()
                                ? Acts::transformBoundToFreeParameters(*measurement_data.surface,
                                                                       *event_data.m_currentGeometryContext, extrapolation_result->parameters())
                                : Acts::FreeVector::Zero());
            ATH_MSG_INFO( "Try to reach " << measurement_data.surface->geometryId()
                          << " dR = " << (missingMeasurement[missing_i].first-missingMeasurementClosestState[missing_i].first)
                          << ((missingMeasurement[missing_i].first>missingMeasurementClosestState[missing_i].first) ^ dir_i ? " forward" :  " backward" )
                          << " measurement pos " << measurement_data.globalPos[0] << " " << measurement_data.globalPos[1]
                          << " " << measurement_data.globalPos[2]
                          << " extrapol " << free_param[0] << " " << free_param[1] << " " << free_param[2]
                          << " start surface " << missingMeasurementClosestState.at(missing_i).second.referenceSurface().geometryId().value()
                          << " target surface " << measurement_data.surface->geometryId().value()
                          << (extrapolation_result.ok() ? " success" : " failed")
                          );



            measurement_data.type = MeasurementData::setExtrapolationResult(measurement_data.type,
                                                                            extrapolation_result.ok(),
                                                                            (extrapolation_result.ok()
                                                                             ? measurement_data.surface->bounds().inside(
                                                                                  extrapolation_result->localPosition()
                                                                               )
                                                                             : false)
                                                                            );
            if (extrapolation_result.ok()) {
               missingMeasurementExtrapolation.push_back(std::make_pair(measurement_i, *extrapolation_result));
               break;
            }
            }
         }
         else {
            throw std::runtime_error("Missing hit without reference surface.");
         }
      }
   }


void ActsTrk::TrackMonitorTool::setMeasurementData(const Acts::GeometryContext &tgContext,
                                                   const xAOD::UncalibratedMeasurement *measurement,
                                                   ActsTrk::TrackMonitorTool::MeasurementData &measurement_out,
                                                   bool edge_check) const {
   assert(measurement);
   const Acts::Surface *surface = m_surfAcc.get(measurement);
   Acts::Vector2 local_pos;
   measurement_out.surface = surface;
   measurement_out.type = static_cast<unsigned short>(measurement->type());
   measurement_out.dim = static_cast<unsigned short>(measurement->numDimensions());
   switch (measurement->type()) {
   case xAOD::UncalibMeasType::StripClusterType: {
      auto temp_local_pos = measurement->localPosition<1>();
      auto temp_local_cov = measurement->localCovariance<1>();

      const ActsDetectorElement *actsElement = getActsDetectorElement(surface);
      const InDetDD::SolidStateDetectorElementBase *si_det_ele
         = dynamic_cast<const InDetDD::SolidStateDetectorElementBase *>(actsElement
                                                                        ? actsElement->upstreamDetectorElement()
                                                                        : nullptr);
      Amg::Vector3D sensor_center(si_det_ele->design().sensorCenter());

      const InDetDD::SCT_ModuleSideDesign *strip_design = dynamic_cast<const InDetDD::SCT_ModuleSideDesign *>(&si_det_ele->design());
      double length=0.;

      if (surface->bounds().type() == Acts::SurfaceBounds::eAnnulus) {
         measurement_out.subspaceIndices=std::array<std::uint8_t,3>{1,0,5};
         if (strip_design) {
            InDetDD::SiLocalPosition pos_phi_r(Amg::Vector2D{temp_local_pos[0], sensor_center[0]});
            std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition>
               ends = strip_design->endsOfStrip(pos_phi_r);
            length=(Acts::Vector2(ends.first)-Acts::Vector2(ends.second)).norm();
         }
         local_pos=Acts::Vector2{sensor_center[0],temp_local_pos[0]};
         measurement_out.localPosAndTime=std::array<float,3>{static_cast<float>(sensor_center[0]),temp_local_pos[0],0.};
         measurement_out.localCovAndTimeCov=std::array<float,4>{static_cast<float>(length),
                                                                0.f, temp_local_cov[0],0.f};
      }
      else {
         measurement_out.subspaceIndices=std::array<std::uint8_t,3>{0,1,5};
         if (strip_design) {
            InDetDD::SiLocalPosition pos_phi_r(Amg::Vector2D{temp_local_pos[0], sensor_center[1]});
            std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition>
               ends = strip_design->endsOfStrip(pos_phi_r);
            length=(Acts::Vector2(ends.first)-Acts::Vector2(ends.second)).norm();
         }
         local_pos=Acts::Vector2{temp_local_pos[0],sensor_center[1]};
         measurement_out.localPosAndTime=std::array<float,3>{temp_local_pos[0],static_cast<float>(sensor_center[1]),0.f};
         measurement_out.localCovAndTimeCov=std::array<float,4>{temp_local_cov[0],0.f,static_cast<float>(length),0.f};
      }
      if (!m_stripID) { throw std::runtime_error("No stripID."); }
      auto rdo_list = static_cast<const xAOD::StripCluster *>(measurement)->rdoList();
      measurement_out.coordinates.reserve( rdo_list.size() );
      for (Identifier::value_type identifier_value : rdo_list) {
         Identifier identifier(identifier_value);
         assert( std::in_range<std::int16_t>(m_stripID->strip(identifier)) );
         measurement_out.coordinates.push_back( static_cast<std::int16_t>(m_stripID->strip(identifier)));
      }
      break;
   }
   case xAOD::UncalibMeasType::PixelClusterType: {
      measurement_out.subspaceIndices=std::array<std::uint8_t,3>{0,1,5};
      auto temp_local_pos=measurement->localPosition<2>();
      auto temp_local_cov = measurement->localCovariance<2>();
      local_pos=Acts::Vector2{temp_local_pos[0],temp_local_pos[1]};
      measurement_out.localPosAndTime=std::array<float,3>{temp_local_pos[0],temp_local_pos[1],0.};
      measurement_out.localCovAndTimeCov=std::array<float,4>{temp_local_cov(0,0),temp_local_cov(0,1),temp_local_cov(1,1),0.};
      if (!m_pixelID) { throw std::runtime_error("No pixelID."); }
      auto rdo_list = static_cast<const xAOD::PixelCluster *>(measurement)->rdoList();
      measurement_out.coordinates.reserve( rdo_list.size()*2 );
      for (Identifier::value_type identifier_value : rdo_list) {
         Identifier identifier(identifier_value);
         assert( std::in_range<std::int16_t>(m_pixelID->eta_index(identifier)) );
         assert( std::in_range<std::int16_t>(m_pixelID->phi_index(identifier)) );
         measurement_out.coordinates.push_back( static_cast<std::int16_t>(m_pixelID->phi_index(identifier)));
         measurement_out.coordinates.push_back( static_cast<std::int16_t>(m_pixelID->eta_index(identifier)));
      }
      break;
   }
   case xAOD::UncalibMeasType::HGTDClusterType:{
      measurement_out.subspaceIndices=std::array<std::uint8_t,3>{0,1,5};
      auto temp_local_pos = measurement->localPosition<3>();
      auto temp_local_cov = measurement->localCovariance<3>();
      local_pos=Acts::Vector2{temp_local_pos[0],temp_local_pos[1]};
      measurement_out.localPosAndTime=std::array<float,3>{temp_local_pos[0],temp_local_pos[1],temp_local_pos[2]};
      measurement_out.localCovAndTimeCov=std::array<float,4>{temp_local_cov(0,0),temp_local_cov(0,1),temp_local_cov(1,1),temp_local_cov(2,2)};
      if (!m_hgtdID) { throw std::runtime_error("No hgtdID."); }
      auto rdo_list = static_cast<const xAOD::HGTDCluster *>(measurement)->rdoList();
      measurement_out.coordinates.reserve( rdo_list.size()*2 );
      measurement_out.dim = 2;
      for (Identifier::value_type identifier_value : rdo_list) {
         Identifier identifier(identifier_value);
         assert( std::in_range<std::int16_t>(m_hgtdID->eta_index(identifier)) );
         assert( std::in_range<std::int16_t>(m_hgtdID->phi_index(identifier)) );
         measurement_out.coordinates.push_back( static_cast<std::int16_t>(m_hgtdID->phi_index(identifier)));
         measurement_out.coordinates.push_back( static_cast<std::int16_t>(m_hgtdID->eta_index(identifier)));
      }
      break;
   }
   default:
      throw std::runtime_error("Unsupported measurement type.");
   }
   if (edge_check) {
      const Acts::SurfaceBounds &surface_bounds = surface->bounds();
      MeasurementData::EHitType the_hit_type=MeasurementData::kCentralHit;
      for (auto [/*Acts::BoundaryTolerance*/ tolerance, /*EventData::EdgeType */ hit_type] : m_sortedTolerances) {
         if (surface_bounds.inside(local_pos, tolerance)) {
            break;
         }
         else {
            the_hit_type=hit_type;
         }
      }
      measurement_out.type = MeasurementData::makeType(measurement_out.type, the_hit_type);
   }
   measurement_out.globalPos = surface->localToGlobal(tgContext,
                                                     local_pos,
                                                     Acts::Vector3{});
}

void ActsTrk::TrackMonitorTool::setMeasurementData(const std::vector<const ActsTrk::MeasurementToTruthParticleAssociation *> &measurementToTruth,
                                                   const Acts::GeometryContext &tgContext,
                                                   const ActsTrk::TruthTrajectoryContainer &truth_trajectories,
                                                   const std::vector<bool> &matched,
                                                   unsigned int trajectory_i,
                                                   std::vector<ActsTrk::TrackMonitorTool::MeasurementData> &measurement_out) const {
   std::span<const unsigned int> measurements = truth_trajectories.measurements(trajectory_i);
   measurement_out.reserve(measurement_out.size()+measurements.size());
   for (auto [/*const MeasurementData &*/ measurement_id, measurement_i]
           : Acts::zip(measurements,
                       std::ranges::views::iota(0u,static_cast<unsigned int>(measurements.size())))) {

      unsigned int measurement_type = ActsTrk::TruthTrajectoryContainer::getMeasurementType(measurement_id);
      unsigned int measurement_index = ActsTrk::TruthTrajectoryContainer::getMeasurementIndex(measurement_id);
      const MeasurementToTruthParticleAssociation *measurement_to_truth = measurementToTruth.at(measurement_type);
      assert(measurement_to_truth);
      assert(measurement_to_truth->sourceContainer() );
      const xAOD::UncalibratedMeasurement *measurement = measurement_to_truth->sourceContainer()->at(measurement_index);
      measurement_out.emplace_back();
      setMeasurementData(tgContext, measurement, measurement_out.back(), !matched.empty() && !matched.at(measurement_i));
   }
}

StatusCode ActsTrk::TrackMonitorTool::createNtuple() {
   if (!m_ntupleName.empty()) {
   ROOT::TWriteLockGuard root_lock (ROOT::gCoreMutex);
      //   TLockGuard root_lock(gROOTMutex);
   GRestore restore;
   m_file.reset(TFile::Open(m_ntupleName.value().c_str(),"RECREATE"));
   using ROOT::RNTupleWriter;
   using ROOT::RNTupleModel;
   auto model=RNTupleModel::Create();

   m_tuple.eventId=model->MakeField<unsigned int>("eventId");
   // truth based
   m_tuple.truthTrajectoryId=model->MakeField<unsigned int>("truthTrajectoryId");
   m_tuple.pdgId=model->MakeField<int>("pdgId");
   m_tuple.uniqueId=model->MakeField<unsigned int>("uniqueId");
   m_tuple.beginMomentum=model->MakeField<std::array<float,3> >("beginMomentum");
   m_tuple.endMomentum=model->MakeField<std::array<float,3> >("endMomentum");
   m_tuple.beginVtx=model->MakeField< std::array<float,3> >("beginVtx");
   m_tuple.endVtx=model->MakeField<std::array<float,3> >("endVtx");
   // match stat
   m_tuple.nExpectedMeasurements=model->MakeField<unsigned int>("nExpectedMeasurements");
   m_tuple.nSeeds=model->MakeField<unsigned int>("nSeeds");
   m_tuple.maxSeedTruthCount=model->MakeField<unsigned int>("maxSeedTruthCount");
   m_tuple.nTracks=model->MakeField<unsigned int>("nTracks");
   m_tuple.nDroppedTracks=model->MakeField<unsigned int>("nDroppedTracks");
   m_tuple.maxTrackTruthCount=model->MakeField<unsigned int>("maxTrackTruthCount");
   m_tuple.bestTrackStatus=model->MakeField<unsigned int>("bestTrackStatus");
   m_tuple.nEdgeHits1000=model->MakeField<std::uint8_t>("nEdgeHits1000");
   m_tuple.nEdgeHits0500=model->MakeField<std::uint8_t>("nEdgeHits0500");
   m_tuple.nEdgeHits0250=model->MakeField<std::uint8_t>("nEdgeHits0250");

   // measurements
   m_tuple.measurementLocalPosAndTime=model->MakeField<std::vector<std::array<float,3> > >("measurementLocalPosAndTime");
   m_tuple.measurementLocalCovAndTimeCov=model->MakeField<std::vector<std::array<float,4> > >("measurementLocalCovAndTimeCov");
   m_tuple.measurementGlobalPos=model->MakeField<std::vector<std::array<float,3> > >("measurementGlobalPos");
   m_tuple.measurementBField=model->MakeField<std::vector<std::array<float,3> > >("measurementBfield");
   m_tuple.measurementType=model->MakeField<std::vector<std::uint16_t> >("measurementType");
   m_tuple.measurementGeoId=model->MakeField<std::vector<std::uint64_t> >("measurementGeoId"); //or identifier ?
   m_tuple.measurementCoordinateDim=model->MakeField<std::vector<std::uint16_t> >("measurementCoordinateDim");
   m_tuple.measurementCoordinateOffset=model->MakeField<std::vector<unsigned int> >("measurementCoordinateOffset"); //or identifier ?
   m_tuple.measurementCoordinates=model->MakeField<std::vector<std::int16_t> >("measurementCoordinates"); //or identifier ?

   // track state
   m_tuple.measurementIndex=model->MakeField<std::vector<unsigned int> >("measurementIndex");
   m_tuple.localPrediction=model->MakeField<std::vector<std::array<float,2> > >("localPrediction");
   m_tuple.localPredictionCov=model->MakeField<std::vector<std::array<float,3> > >("localPredictionCov");
   m_tuple.localFiltered=model->MakeField<std::vector<std::array<float,2> > >("localFiltered");
   m_tuple.localFilteredCov=model->MakeField<std::vector<std::array<float,3> > >("localFilteredCov");
   m_tuple.calibratedDim=model->MakeField<std::vector<unsigned int> >("calibratedDim");
   m_tuple.calibrated=model->MakeField<std::vector<float> >("calibrated");
   m_tuple.calibratedCov=model->MakeField<std::vector<float> >("calibratedCov");
   m_tuple.predictedMomentum=model->MakeField<std::vector<std::array<float,3> > >("predictedMomentum");
   m_tuple.predictedMomentumCov=model->MakeField<std::vector<std::array<float,6> > >("predictedMomentumCov");
   m_tuple.filteredMomentum=model->MakeField<std::vector<std::array<float,3> > >("filteredMomentum");
   m_tuple.filteredMomentumCov=model->MakeField<std::vector<std::array<float,6> > >("filteredMomentumCov");
   m_tuple.stateBField=model->MakeField<std::vector<std::array<float,3> > >("stateBField");
   m_tuple.stateGeoId=model->MakeField<std::vector<std::size_t> >("stateGeoId");
   m_tuple.classification=model->MakeField< std::vector<unsigned int> >("classification");
   m_tuple.chi2=model->MakeField< std::vector<float> >("chi2");

   if (m_extrapolationTool.isEnabled()) {
      m_tuple.measurementIndexMissing =model->MakeField< std::vector<unsigned int> >("measurementIndexMissing");
      m_tuple.localPredictionMissing=model->MakeField< std::vector<std::array<float,2> > >("localPredictionMissing");
      m_tuple.localPredictionMissingCov=model->MakeField< std::vector<std::array<float,3> > >("localPredictionMissingCov");
      m_tuple.predictedMomentumMissing=model->MakeField< std::vector<std::array<float,3> > >("predictedMomentumMissing") ;
      m_tuple.predictedMomentumMissingCov=model->MakeField< std::vector<std::array<float,6> > >("predictedMomentumMissingCov") ;
      m_tuple.predictedMissingChi2=model->MakeField< std::vector<float > >("predictedMissingChi2") ;
   }

   m_writer = RNTupleWriter::Append(std::move(model), "MatchedTracks", *m_file);
   }
   return StatusCode::SUCCESS;
}

void ActsTrk::TrackMonitorTool::fillNTuple(const EventContext &ctx, EventData &event_data, unsigned int truth_trajectory_i) const {
   if (!m_writer) return;
   *m_tuple.eventId = event_data.m_currentEventNumber;
   // prepare truth
   *(m_tuple.truthTrajectoryId) = truth_trajectory_i;
   assert( truth_trajectory_i < event_data.m_truthTrajectories.size());
   const xAOD::TruthParticle *truth_particle = event_data.m_truthTrajectories.truthParticle(truth_trajectory_i);
   assert(truth_particle);

   const xAOD::TruthParticle *last_child = getLastChild(truth_particle);

   // preoare measurement data
   assert(event_data.m_currentGeometryContext);
   event_data.m_measurementDataCache.clear();
   // use best matching kept track (or dropped track)
   unsigned int matched_track_i=truth_trajectory_i*2;
   assert(    matched_track_i+1 < event_data.m_bestMatchCache.trackIndex.size()
           && matched_track_i+1 < event_data.m_bestMatchCache.truthCount.size());
   if (   event_data.m_bestMatchCache.trackIndex[matched_track_i]!=EventData::BestMatchCache::g_invalidTrack
       && event_data.m_bestMatchCache.trackIndex[matched_track_i+1]!=EventData::BestMatchCache::g_invalidTrack
       && event_data.m_bestMatchCache.truthCount[matched_track_i]<event_data.m_bestMatchCache.truthCount[matched_track_i+1]) {
      ATH_MSG_WARNING("Ignored dropped track has higher truth count: " <<
                      event_data.m_bestMatchCache.truthCount[matched_track_i] << " < " << event_data.m_bestMatchCache.truthCount[matched_track_i+1]);
   }
   if (event_data.m_bestMatchCache.trackIndex[matched_track_i]==EventData::BestMatchCache::g_invalidTrack) {
      ++matched_track_i;
   }
   // collected unmatched measurements on track
   event_data.m_measurementCache.clear();
   unsigned int n_calibrated_pos=0;
   unsigned int n_calibrated_cov=0;
   event_data.m_measurementIndexCache.clear();
   event_data.m_matchCache.clear();
   event_data.m_hitsOnSurfaceCache.clear();
   if (event_data.m_bestMatchCache.trackIndex[matched_track_i]!=EventData::BestMatchCache::g_invalidTrack) {
      // event_data.m_measurementCache will container pointers to all unmatched measurements
      // event_data.m_matchCache will be true for each truth measurement which is matched.
      getUnmatchedMeasurements(event_data,matched_track_i,
                               event_data.m_measurementCache,
                               event_data.m_matchCache,
                               event_data.m_hitsOnSurfaceCache,
                               event_data.m_measurementIndexCache,
                               n_calibrated_pos,
                               n_calibrated_cov);
   }
   std::span<const unsigned int> measurements=event_data.m_truthTrajectories.measurements(truth_trajectory_i);
   event_data.m_measurementDataCache.reserve( measurements.size() + event_data.m_measurementCache.size());

   assert(event_data.m_currentGeometryContext);
   event_data.m_measurementDataCache.clear();
   // gather measurement data of truth measurements
   setMeasurementData(event_data.m_measurementToTruth,
                      *event_data.m_currentGeometryContext,
                      event_data.m_truthTrajectories,
                      event_data.m_matchCache,
                      truth_trajectory_i,
                      event_data.m_measurementDataCache);
   // gather measurement data of unmatched measurements
   for (const xAOD::UncalibratedMeasurement *measurement : event_data.m_measurementCache) {
      event_data.m_measurementDataCache.emplace_back();
      setMeasurementData(*event_data.m_currentGeometryContext, measurement, event_data.m_measurementDataCache.back(),false);
   }

   // sort measurement data by trueh/unmatched, then r
   // first compute r
   event_data.m_measurementOrderCache.clear();
   event_data.m_measurementOrderCache.reserve( event_data.m_measurementDataCache.size());
   std::array<std::uint8_t, MeasurementData::kNHitTypes> edgeHits{};
   for (auto [/*const MeasurementData &*/ measurement_data, measurement_data_i]
           : Acts::zip(event_data.m_measurementDataCache,
                       std::ranges::views::iota(0u,static_cast<unsigned int>(event_data.m_measurementDataCache.size())))) {
      event_data.m_measurementOrderCache.push_back(std::make_pair( static_cast<float>(measurement_data.globalPos.norm()),measurement_data_i) );
      ++edgeHits.at(measurement_data.hitType());
   }
   // find maximum r for matched tracks
   event_data.m_missingMeasurementCache.clear();
   event_data.m_missingMeasurementClosestStateCache.clear();
   event_data.m_stateGlobalPosDirCache.clear();
   event_data.m_missingMeasurementExtrapolation.clear();
   float max_matched_r=0.f;
   for (unsigned int i=0; i<measurements.size(); ++i ) {
      max_matched_r=std::max(max_matched_r,event_data.m_measurementOrderCache[i].first);
      if (!event_data.m_matchCache.empty() &&  !event_data.m_matchCache.at(i)) {
         assert( i < event_data.m_measurementDataCache.size());
         MeasurementData &measurement_data = event_data.m_measurementDataCache[i];
         bool has_hit_on_surface=false;
         if (measurement_data.surface) {
            std::unordered_map<std::uint64_t, unsigned int>::const_iterator
               hit_on_surface_iter = event_data.m_hitsOnSurfaceCache.find(measurement_data.surface->geometryId().value());
            has_hit_on_surface = (hit_on_surface_iter != event_data.m_hitsOnSurfaceCache.end() && hit_on_surface_iter->second>0u);
         }

         event_data.m_measurementDataCache[i].type = MeasurementData::setMissing(measurement_data.type,true,has_hit_on_surface);
         event_data.m_missingMeasurementCache.push_back(std::make_pair(event_data.m_measurementOrderCache[i].first,i));
      }
   }
   // add max_r of matched tracks as offset to sort unmatched tracks after matched tracks
   max_matched_r *=1.1;
   for (unsigned int i=measurements.size(); i<event_data.m_measurementOrderCache.size(); ++i ) {
      event_data.m_measurementOrderCache[i].first+=max_matched_r;
   }

   std::sort(event_data.m_measurementOrderCache.begin(), event_data.m_measurementOrderCache.end(),[](const std::pair<float,unsigned int> &a,
                                                                                                     const std::pair<float,unsigned int> &b) {
      return a.first < b.first;
   });
   {
      // create reverse lookup table from measurement_data_i to the ordered index
      unsigned int order_i=0;
      event_data.m_measurementReverseIndexCache.clear();
      event_data.m_measurementReverseIndexCache.resize(event_data.m_measurementOrderCache.size());
      for (auto [r,measurement_data_i] : event_data.m_measurementOrderCache) {
         event_data.m_measurementReverseIndexCache[measurement_data_i]=order_i;
         ++order_i;
      }
   }
   if (event_data.m_bestMatchCache.trackIndex[matched_track_i]!=EventData::BestMatchCache::g_invalidTrack
       && m_extrapolationTool.isEnabled()) {
      findClosestTrackState(event_data,
                            matched_track_i,
                            event_data.m_missingMeasurementCache,
                            event_data.m_missingMeasurementClosestStateCache,
                            event_data.m_stateGlobalPosDirCache);
      extrapolateToMissing(ctx,
                           *m_extrapolationTool,
                           event_data,
                           matched_track_i,
                           event_data.m_measurementDataCache,
                           event_data.m_missingMeasurementCache,
                           event_data.m_missingMeasurementClosestStateCache,
                           event_data.m_missingMeasurementExtrapolation);
   }

   {
   std::lock_guard<std::mutex> lock(m_tupleMutex);
   // fill truth
   *(m_tuple.pdgId) = truth_particle->pdgId();
   *(m_tuple.uniqueId) = truth_particle->uid();
   *(m_tuple.beginMomentum) = getFloatMomentum(truth_particle->p4());
   *(m_tuple.endMomentum) = getFloatMomentum(last_child->p4());
   *(m_tuple.beginVtx) = truth_particle->hasProdVtx() ? getVertexPosition(truth_particle->prodVtx()) : std::array<float,3>{0.f,0.f,0.f};
   *(m_tuple.endVtx) = last_child->hasDecayVtx() ? getVertexPosition(last_child->decayVtx()) : getScaledDirection(last_child->p4(), 40e3);

   // fill match statistics
   *(m_tuple.nExpectedMeasurements) = measurements.size();
   *(m_tuple.nSeeds) = event_data.m_seedCandidates.at(truth_trajectory_i).first;
   *(m_tuple.maxSeedTruthCount) = event_data.m_seedCandidates.at(truth_trajectory_i).second;
   *(m_tuple.nTracks) = event_data.m_bestMatchCache.candidateCount.at(truth_trajectory_i*2);
   *(m_tuple.nDroppedTracks) = event_data.m_bestMatchCache.candidateCount.at(truth_trajectory_i*2+1);
   *(m_tuple.maxTrackTruthCount) = event_data.m_bestMatchCache.truthCount.at(matched_track_i); // kept or dropped
   *(m_tuple.bestTrackStatus) = event_data.m_bestMatchCache.status.at(matched_track_i); // kept or dropped,
   *(m_tuple.nEdgeHits1000)=edgeHits[MeasurementData::kEdgeHit100]+edgeHits[MeasurementData::kEdgeHit050]+edgeHits[MeasurementData::kEdgeHit025];
   *(m_tuple.nEdgeHits0500)=edgeHits[MeasurementData::kEdgeHit050]+edgeHits[MeasurementData::kEdgeHit025];
   *(m_tuple.nEdgeHits0250)=edgeHits[MeasurementData::kEdgeHit025];

   // fill measurement data
   m_tuple.measurementLocalPosAndTime->clear();
   m_tuple.measurementLocalPosAndTime->reserve( event_data.m_measurementOrderCache.size() * 3);
   m_tuple.measurementLocalCovAndTimeCov->clear();
   m_tuple.measurementLocalCovAndTimeCov->reserve( event_data.m_measurementOrderCache.size() * 4);
   m_tuple.measurementGlobalPos->clear();
   m_tuple.measurementGlobalPos->reserve( event_data.m_measurementOrderCache.size() * 3);
   m_tuple.measurementGeoId->clear();
   m_tuple.measurementGeoId->reserve( event_data.m_measurementOrderCache.size());
   m_tuple.measurementType->clear();
   m_tuple.measurementType->reserve( event_data.m_measurementOrderCache.size());
   m_tuple.measurementCoordinateDim->clear();
   m_tuple.measurementCoordinateDim->reserve( event_data.m_measurementOrderCache.size());
   m_tuple.measurementCoordinateOffset->clear();
   m_tuple.measurementCoordinateOffset->reserve( event_data.m_measurementOrderCache.size()+1);
   m_tuple.measurementBField->clear();
   m_tuple.measurementBField->reserve( event_data.m_measurementOrderCache.size() * 3);
   unsigned int n_coordinates=0;
   for (auto [r, measurement_data_i ] : event_data.m_measurementOrderCache) {
      const MeasurementData &measurement_data  = event_data.m_measurementDataCache.at(measurement_data_i);
      m_tuple.measurementLocalPosAndTime->push_back(measurement_data.localPosAndTime);
      m_tuple.measurementLocalCovAndTimeCov->push_back(measurement_data.localCovAndTimeCov);
      m_tuple.measurementGlobalPos->push_back(std::array<float,3>{static_cast<float>(measurement_data.globalPos[0]),
                                                                  static_cast<float>(measurement_data.globalPos[1]),
                                                                  static_cast<float>(measurement_data.globalPos[2])});

      m_tuple.measurementGeoId->push_back(measurement_data.surface->geometryId().value());
      m_tuple.measurementType->push_back(measurement_data.type);
      m_tuple.measurementCoordinateDim->push_back(measurement_data.dim);
      m_tuple.measurementCoordinateOffset->push_back(n_coordinates);
      n_coordinates+=measurement_data.coordinates.size();

      std::array<double,3> bfield{};
      event_data.fieldCache.getField(measurement_data.globalPos.data(),
                                bfield.data(),
                                nullptr);

      m_tuple.measurementBField->push_back(std::array<float,3>{static_cast<float>(bfield[0]),
                                                         static_cast<float>(bfield[1]),
                                                         static_cast<float>(bfield[2])});
   }
   m_tuple.measurementCoordinateOffset->push_back(n_coordinates);
   m_tuple.measurementCoordinates->clear();
   m_tuple.measurementCoordinates->reserve(n_coordinates);
   {
   unsigned int order_i=0;
   for (auto [r, measurement_data_i ] : event_data.m_measurementOrderCache) {
      const MeasurementData &measurement_data  = event_data.m_measurementDataCache.at(measurement_data_i);
      assert( m_tuple.measurementCoordinates->size() == m_tuple.measurementCoordinateOffset->at(order_i));
      for (std::int16_t a_coordinate: measurement_data.coordinates) {
         m_tuple.measurementCoordinates->push_back(a_coordinate);
      }
      ++order_i;
      assert( m_tuple.measurementCoordinates->size() == m_tuple.measurementCoordinateOffset->at(order_i));
   }
   }

   // fill track states
   m_tuple.measurementIndex->clear();
   m_tuple.measurementIndex->reserve(event_data.m_measurementIndexCache.size());
   m_tuple.localPrediction->clear();
   m_tuple.localPrediction->reserve(event_data.m_measurementIndexCache.size()*2);
   m_tuple.localPredictionCov->clear();
   m_tuple.localPredictionCov->reserve(event_data.m_measurementIndexCache.size()*3);
   m_tuple.localFiltered->clear();
   m_tuple.localFiltered->reserve(event_data.m_measurementIndexCache.size()*2);
   m_tuple.localFilteredCov->clear();
   m_tuple.localFilteredCov->reserve(event_data.m_measurementIndexCache.size()*3);

   m_tuple.calibratedDim->clear();
   m_tuple.calibratedDim->reserve(event_data.m_measurementIndexCache.size());
   m_tuple.calibrated->clear();
   m_tuple.calibrated->reserve(n_calibrated_pos);
   m_tuple.calibratedCov->clear();
   m_tuple.calibratedCov->reserve(n_calibrated_cov);
   m_tuple.predictedMomentum->clear();
   m_tuple.predictedMomentum->reserve(event_data.m_measurementIndexCache.size()*3);
   m_tuple.predictedMomentumCov->clear();
   m_tuple.predictedMomentumCov->reserve(event_data.m_measurementIndexCache.size()*6);
   m_tuple.filteredMomentum->clear();
   m_tuple.filteredMomentum->reserve(event_data.m_measurementIndexCache.size()*3);
   m_tuple.filteredMomentumCov->clear();
   m_tuple.filteredMomentumCov->reserve(event_data.m_measurementIndexCache.size()*6);

   m_tuple.stateBField->clear();
   m_tuple.stateBField->reserve(event_data.m_measurementIndexCache.size());
   m_tuple.stateGeoId->clear();
   m_tuple.stateGeoId->reserve(event_data.m_measurementIndexCache.size());
   m_tuple.chi2->clear();
   m_tuple.chi2->reserve(event_data.m_measurementIndexCache.size());
   enum EClassification {TruthMatch, ConfusedHit, Hole, ConfusedHole};
   m_tuple.classification->clear();
   m_tuple.classification->reserve(event_data.m_measurementIndexCache.size());

   if (m_extrapolationTool.isEnabled()) {
      m_tuple.measurementIndexMissing->clear();
      m_tuple.localPredictionMissing->clear();
      m_tuple.localPredictionMissingCov->clear();
      m_tuple.predictedMomentumMissing->clear();
      m_tuple.predictedMomentumMissingCov->clear();
      m_tuple.predictedMissingChi2->clear();
   }

   if (event_data.m_bestMatchCache.trackIndex[matched_track_i]!=EventData::BestMatchCache::g_invalidTrack) {
      unsigned int state_i=0u;
      using const_track_state_proxy_t = ActsTrk::TrackMonitorTool::track_container_t::ConstTrackStateProxy;
      auto track = event_data.m_actsTracksContainer.getTrack(event_data.m_bestMatchCache.trackIndex.at(matched_track_i));
      event_data.m_actsTracksContainer
         .trackStateContainer().visitBackwards(track.tipIndex(),
                                               [&m_tuple=this->m_tuple,
                                                &event_data,
                                                &state_i,
                                                truth_trajectory_i](const const_track_state_proxy_t &state) {
            unsigned int measurement_idx = event_data.m_measurementIndexCache.at(state_i);
            if (measurement_idx != std::numeric_limits<unsigned int>::max()) {
               unsigned int ordered_measurement_idx = event_data.m_measurementReverseIndexCache.at(measurement_idx);
               m_tuple.measurementIndex->push_back(ordered_measurement_idx);
            }
            else {
               m_tuple.measurementIndex->push_back(measurement_idx);
            }
            auto subspace_idx = state.hasProjector() ? state.projectorSubspaceIndices() : std::array<Acts::SubspaceIndex,6>{0,1,2,3,4,5};
            if (state.hasPredicted()) {
               pushBackProjection<2>( subspace_idx, state.predicted(), *m_tuple.localPrediction);
               pushBackProjectionCov<2>( subspace_idx, state.predictedCovariance(), *(m_tuple.localPredictionCov));
               fillMomentum(state.predicted(),state.predictedCovariance(), *(m_tuple.predictedMomentum),*(m_tuple.predictedMomentumCov) );
            }
            else {
               m_tuple.localPrediction->emplace_back(std::array<float,2>{});
               m_tuple.localPredictionCov->emplace_back(std::array<float,3>{});
               m_tuple.predictedMomentum->emplace_back(std::array<float,3>{});
               m_tuple.predictedMomentumCov->emplace_back(std::array<float,6>{});
            }

            if (state.hasSmoothed()) {
               pushBackProjection<2>( subspace_idx, state.smoothed(), *(m_tuple.localFiltered));
               pushBackProjectionCov<2>( subspace_idx, state.smoothedCovariance(), *(m_tuple.localFilteredCov));
               fillMomentum(state.smoothed(),state.smoothedCovariance(), *(m_tuple.filteredMomentum),*(m_tuple.filteredMomentumCov) );
            }
            else if (state.hasFiltered()) {
               pushBackProjection<2>( subspace_idx, state.filtered(), *(m_tuple.localFiltered) );
               pushBackProjectionCov<2>( subspace_idx, state.filteredCovariance(), *(m_tuple.localFilteredCov));
               fillMomentum(state.filtered(),state.filteredCovariance(), *(m_tuple.filteredMomentum),*(m_tuple.filteredMomentumCov) );
            }
            else {
               m_tuple.localFiltered->emplace_back(std::array<float,2>{});
               m_tuple.localFilteredCov->emplace_back(std::array<float,3>{});
               m_tuple.filteredMomentum->emplace_back(std::array<float,3>{});
               m_tuple.filteredMomentumCov->emplace_back(std::array<float,6>{});
            }

            if (state.hasCalibrated()) {
               m_tuple.calibratedDim->push_back(state.calibratedSize());
               switch (state.calibratedSize()) {
               case 1: {
                  pushBackProjection<1>( std::array<unsigned int,1>{0}, state.calibrated<1>(), *(m_tuple.calibrated));
                  pushBackProjectionCov<1>( std::array<unsigned int,1>{0}, state.calibratedCovariance<1>(), *(m_tuple.calibratedCov));
                  break;
               }
               case 2: {
                  pushBackProjection<2>( std::array<unsigned int,2>{0,1}, state.calibrated<2>(), *(m_tuple.calibrated));
                  pushBackProjectionCov<2>( std::array<unsigned int,2>{0,1}, state.calibratedCovariance<2>(), *(m_tuple.calibratedCov));
                  break;
               }
               case 3: {
                  pushBackProjection<3>( std::array<unsigned int,3>{0,1,2}, state.calibrated<3>(), *(m_tuple.calibrated));
                  pushBackProjectionCov<3>( std::array<unsigned int,3>{0,1,2}, state.calibratedCovariance<3>(), *(m_tuple.calibratedCov));
                  break;
               }
               default:
                  throw std::runtime_error("Unhandled dimension.");
               }
            }
            else {
               m_tuple.calibratedDim->push_back(0u);
            }

            if (state.hasReferenceSurface()) {
               m_tuple.stateGeoId->push_back(state.referenceSurface().geometryId().value());
               Acts::FreeVector
                  free_param = Acts::transformBoundToFreeParameters(state.referenceSurface(),
                                                                    *event_data.m_currentGeometryContext, state.parameters());

               std::array<double,3> bfield{};
               event_data.fieldCache.getField(free_param.data(),
                                         bfield.data(),
                                         nullptr);

               m_tuple.stateBField->push_back(std::array<float,3>{static_cast<float>(bfield[0]),
                                                                  static_cast<float>(bfield[1]),
                                                                  static_cast<float>(bfield[2])});
            }
            else {
               m_tuple.stateGeoId->push_back(Acts::GeometryIdentifier().value());
               m_tuple.stateBField->push_back(std::array<float,3>{});
            }
            m_tuple.chi2->push_back(state.chi2());
            m_tuple.classification->push_back(static_cast<unsigned int>(Tuple::Other));

            if (event_data.m_measurementIndexCache.at(state_i)!=std::numeric_limits<unsigned int>::max()) {
               unsigned int measurement_data_i = event_data.m_measurementIndexCache.at(state_i);
               assert( event_data.m_measurementDataCache.size() >= event_data.m_measurementCache.size());
               if (measurement_data_i + event_data.m_measurementCache.size() < event_data.m_measurementDataCache.size() ) {
                  m_tuple.classification->back() = static_cast<unsigned int>(Tuple::TruthMatch);
               }
               else {
                  m_tuple.classification->back() = static_cast<unsigned int>(Tuple::ConfusedHit);
               }
               unsigned int ordered_measurement_data_i = event_data.m_measurementReverseIndexCache.at(measurement_data_i);

               m_tuple.measurementIndex->push_back(ordered_measurement_data_i);
            }
            else {
               m_tuple.measurementIndex->push_back(std::numeric_limits<unsigned int>::max());
            }
            if (state.typeFlags().isHole()) {
               assert(event_data.m_measurementIndexCache.at(state_i)==std::numeric_limits<unsigned int>::max());
               m_tuple.classification->back() = static_cast<unsigned int>(Tuple::ConfusedHole);
               if (state.hasReferenceSurface()) {
                  if (event_data.m_measurementDataCache.size() > event_data.m_measurementCache.size() ) {
                     auto this_surface_id_value = state.referenceSurface().geometryId().value();
                     for (unsigned int measurement_data_i=0;
                          measurement_data_i<  event_data.m_measurementDataCache.size() - event_data.m_measurementCache.size();
                          ++measurement_data_i) {
                        if (event_data.m_measurementDataCache[measurement_data_i].surface->geometryId().value() == this_surface_id_value) {
                           m_tuple.classification->back() = static_cast<unsigned int>(Tuple::Hole);
                           break;
                        }
                     }
                  }
               }
            }
            ++state_i;
       });
   }

   if (m_extrapolationTool.isEnabled()) {
   for (const std::pair<unsigned int, Acts::BoundTrackParameters> &prediction_missing : event_data.m_missingMeasurementExtrapolation) {
      const MeasurementData &measurement_data = event_data.m_measurementDataCache.at(prediction_missing.first);
      m_tuple.measurementIndexMissing->push_back(prediction_missing.first);
      pushBackProjection<2>( measurement_data.subspaceIndices, prediction_missing.second.parameters(), *m_tuple.localPredictionMissing);
      if (prediction_missing.second.covariance().has_value()) {
         pushBackProjectionCov<2>( measurement_data.subspaceIndices, *prediction_missing.second.covariance(), *(m_tuple.localPredictionMissingCov));
      }
      static const Acts::BoundMatrix zero_cov(Acts::BoundMatrix::Zero());

      float chi2=std::numeric_limits<float>::max();
      if (prediction_missing.second.covariance().has_value()) {
       if (measurement_data.dim==1) {
         chi2 = static_cast<float>(computeChi2(Eigen::Matrix<double,1,1>{measurement_data.localPosAndTime[0]},
                                               Eigen::Matrix<double,1,1>{measurement_data.localCovAndTimeCov[0]},
                                               project<1,Eigen::Matrix<double,1,1> >(std::array<std::uint32_t,1>{measurement_data.subspaceIndices[0]},
                                                                                     prediction_missing.second.parameters()),
                                               project<1,Eigen::Matrix<double,1,1> >(std::array<std::uint32_t,1>{measurement_data.subspaceIndices[0]},
                                                                                     *( prediction_missing.second.covariance()))));
       }
       else {
          Eigen::Matrix<double,2,2> measurement_local_cov;
          measurement_local_cov << measurement_data.localCovAndTimeCov[0], measurement_data.localCovAndTimeCov[1],
                                   measurement_data.localCovAndTimeCov[1], measurement_data.localCovAndTimeCov[2];
         chi2 = static_cast<float>(computeChi2(Eigen::Matrix<double,2,1>{measurement_data.localPosAndTime[0],measurement_data.localPosAndTime[1]},
                                               measurement_local_cov,
                                               project<2,Eigen::Matrix<double, 2, 1> >(std::array<std::uint32_t,2>{measurement_data.subspaceIndices[0],
                                                                                                                   measurement_data.subspaceIndices[1]},
                                                                                       prediction_missing.second.parameters()),
                                               project<2,Eigen::Matrix<double, 2, 2> >(std::array<std::uint32_t,2>{measurement_data.subspaceIndices[0],
                                                                                                                   measurement_data.subspaceIndices[1]},
                                                  *(prediction_missing.second.covariance()))));
       }
      }
      m_tuple.predictedMissingChi2->push_back(chi2);


      fillMomentum(prediction_missing.second.parameters(),
                   (prediction_missing.second.covariance().has_value()
                    ? *prediction_missing.second.covariance()
                    : zero_cov),
                   *(m_tuple.predictedMomentumMissing),
                   *(m_tuple.predictedMomentumMissingCov) );
   }
   }
   ROOT::TWriteLockGuard root_lock (ROOT::gCoreMutex);
   //TLockGuard root_lock_global(gROOTMutex);
   m_writer->Fill();
   }
}
