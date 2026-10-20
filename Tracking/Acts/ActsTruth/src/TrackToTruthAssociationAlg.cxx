/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackToTruthAssociationAlg.h"

#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"

#include <iomanip>
#include <cmath>
#include <type_traits>
#include <typeinfo>
#include <numeric>


namespace ActsTrk
{
  // to dump
  inline MsgStream &operator<<(MsgStream &out, const ActsUtils::Stat &stat) {
     ActsUtils::dumpStat(out, stat);
     return out;
  }

  StatusCode TrackToTruthAssociationAlg::initialize()
  {
     ATH_CHECK( m_ctxProvider.initialize() );
     ATH_CHECK( m_tracksContainerKey.initialize() );
     ATH_CHECK( m_measAssocKeys.initialize() );
     ATH_CHECK( m_trackToTruthOut.initialize() );

     m_elasticDecayUtil.setEnergyLossBinning(m_energyLossBinning.value());
     return StatusCode::SUCCESS;
  }

  template <bool IsDebug>
  template <class T_OutStream>
  inline void TrackToTruthAssociationAlg::AssociationCounter<IsDebug>::dumpStatistics(T_OutStream &out) const {
     if constexpr(IsDebug) {
        out << "Measurements per track :" << m_measPerTrack << std::endl
            << m_measPerTrack.histogramToString() << std::endl
            << "TruthParticles per track :" << m_truthParticlesPerTrack << std::endl
            << m_truthParticlesPerTrack.histogramToString();
     }
  }
  template <bool IsDebug>
  void inline TrackToTruthAssociationAlg::AssociationCounter<IsDebug>::fillStatistics(unsigned int n_measurements, unsigned int n_particles) const {
     if constexpr(IsDebug) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_measPerTrack.add(n_measurements);
        m_truthParticlesPerTrack.add(n_particles);
     }
  }

  namespace {
     std::string makeLabel(const std::string &header, unsigned int i, const std::string &tail) {
        std::stringstream label;
        label << header << " " << i << " " << tail;
        return label.str();
     }
  }

  StatusCode TrackToTruthAssociationAlg::finalize()
  {
     if (m_nIncompatibleMeasurementContainer>0) {
        ATH_MSG_WARNING( "Encountered measurements not compaible with provided association maps in "
                         << m_nIncompatibleMeasurementContainer << " of " << (m_nCcompatibleMeasurementContainer+m_nIncompatibleMeasurementContainer) << " cases.");
     }
     if (msgLvl(MSG::INFO)) {
        msg(MSG::INFO) << "-- Statistics:" << std::endl;
        unsigned int idx=0;
        for (const std::atomic<std::size_t> &elm : m_nTracksWithAssociatedTruth) {
           msg() << std::setw(20) << elm << " "
                 << (idx==0
                     ? std::string(" tracks without associated truth particles.")
                     : (idx==m_nTracksWithAssociatedTruth.size()-1
                        ? makeLabel(" tracks with more than",
                                    m_nTracksWithAssociatedTruth.size()-1,
                                    "associated truth particles.")
                        : makeLabel(" tracks with",
                                    idx,
                                    "associated truth particle(s).")));
           if (idx!=m_nTracksWithAssociatedTruth.size()-1 || TrackToTruthParticleAssociationDebugHists) {
              msg() << std::endl;
           }
           ++idx;
        }
        if constexpr(TrackToTruthParticleAssociationDebugHists) {
           m_elasticDecayUtil.dumpStatistics(msg());
           msg() << std::endl;
           m_associationCounter.dumpStatistics(msg());
        }
        msg() << endmsg;
     }
     return StatusCode::SUCCESS;
  }

  StatusCode TrackToTruthAssociationAlg::execute(const EventContext &ctx) const
  {
    auto track_association = std::make_unique<TrackToTruthParticleAssociation>();

    const ActsTrk::TrackContainer* tracksContainer{};
    ATH_CHECK(SG::get(tracksContainer, m_tracksContainerKey, ctx));
   
    track_association->resize( tracksContainer->size() );
    track_association->setSourceContainer(DataLink<ActsTrk::TrackContainer>(*tracksContainer,ctx));
    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

   std::array<std::vector<const ActsTrk::MeasurementToTruthParticleAssociation*>,
              Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> prdToTruthMaps{};
   
   for (const auto& key : m_measAssocKeys) {
       const MeasurementToTruthParticleAssociation* measAssocMap{};
       ATH_CHECK(SG::get(measAssocMap, key, ctx));
       if (measAssocMap->sourceContainer()->empty()) {
            continue;
       }
       const xAOD::UncalibMeasType measType = measAssocMap->sourceContainer()->front()->type();
       prdToTruthMaps[Acts::toUnderlying(measType)].push_back(measAssocMap);
   }
   std::array<unsigned int,s_NCounterForAssociatedTruth> tracks_with_associated_truth{};
   std::pair<unsigned int, unsigned int> compatible_assoc_container_counts{};
   if (m_nIncompatibleMeasurementContainer>0) {
      // used to suppress reoccurring error messages
      --m_nIncompatibleMeasurementContainer;
      ++compatible_assoc_container_counts.second;
   }

    std::vector<unsigned int> counted_truth_particles;
    counted_truth_particles.reserve(10);

    for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : *tracksContainer) {
       const unsigned track_i = track.index();
       unsigned int n_measurements=0u;
     
       HitCounterArray &reco_hits = track_association->at(track_i).totalCounts();
       HitCounterArray &noise_hits = track_association->at(track_i).noiseCounts();
       ActsTrk::HitCountsPerTrack::container  &truth_particle_counts = track_association->at(track_i).countsPerTruthParticle();
       for (const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state : track.trackStatesReversed()) {
         if (state.typeFlags().isOutlier() || !state.hasUncalibratedSourceLink()) {
            continue;
         }
         const auto* uncalibMeas = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
         const auto typeIdx = Acts::toUnderlying(uncalibMeas->type());

         ++n_measurements;

         const ActsTrk::MeasurementToTruthParticleAssociation* association_map{};
         if (auto map_itr = std::ranges::find_if(prdToTruthMaps.at(typeIdx), 
                           [uncalibMeas](const auto* map){
                               return uncalibMeas->container() == map->sourceContainer();
                           }); map_itr != prdToTruthMaps.at(typeIdx).end()) {
            association_map = (*map_itr);
         } else {
            continue;
         }
         ++compatible_assoc_container_counts.second;
         /** If no truth particle is associated mark it as noise */
         if (association_map->at(uncalibMeas->index()).empty()) {
            ++noise_hits.at(typeIdx);
         }
         counted_truth_particles.clear();
         /** Loop over the associated truth particles */
         for (const xAOD::TruthParticle *truth_particle : association_map->at(uncalibMeas->index()) ) {
            const xAOD::TruthParticle *mother_particle = m_elasticDecayUtil.getMother(*truth_particle, m_maxEnergyLoss.value());
            ATH_MSG_VERBOSE("Fill truth counts for "<<uncalibMeas->type()<<" hit with particle "<<(mother_particle->pt()*1.e-3)<<" [GeV], eta:"
               <<mother_particle->eta()<<", " <<", phi: "<<mother_particle->phi()<<", pdgId: "<<mother_particle->pdgId()<<", index: "<<mother_particle->index()
               <<", "<<mother_particle->container());
            // do not count hits of associated truth particles again if they are associated to the same elastic decay chain:
            if (!Acts::rangeContainsValue(counted_truth_particles, mother_particle->index())) {
               counted_truth_particles.push_back(mother_particle->index());

               ActsTrk::HitCountsPerTrack::container::iterator
                  hit_count_iter = std::ranges::find_if(truth_particle_counts,
                        [mother_particle](const std::pair<const xAOD::TruthParticle *, HitCounterArray > &a) {
                           return a.first == mother_particle;
                        });
               if (hit_count_iter == truth_particle_counts.end()) {
                  truth_particle_counts.push_back( std::make_pair(mother_particle, HitCounterArray{}));
                  hit_count_iter = truth_particle_counts.end()-1;
               }
               ++(hit_count_iter->second.at(typeIdx));
            }
         }
         ++reco_hits.at(typeIdx);
      }
      std::ranges::stable_sort(truth_particle_counts,
               [](const std::pair<const xAOD::TruthParticle*, HitCounterArray>& a,
                  const std::pair<const xAOD::TruthParticle*, HitCounterArray>& b) {
                  return std::accumulate(a.second.begin(),a.second.end(),0u) > 
                         std::accumulate(b.second.begin(),b.second.end(),0u);
               });
      m_associationCounter.fillStatistics(n_measurements, truth_particle_counts.size());
      ++(tracks_with_associated_truth[std::min(truth_particle_counts.size(),
                                               tracks_with_associated_truth.size()-1u)]);
    }
    for (const auto[idx, elem] : Acts::enumerate(tracks_with_associated_truth)) {
       m_nTracksWithAssociatedTruth[idx] += elem;

    }
    m_nIncompatibleMeasurementContainer += compatible_assoc_container_counts.second;
    m_nCcompatibleMeasurementContainer += compatible_assoc_container_counts.first;

    SG::WriteHandle associationOutHandle{m_trackToTruthOut, ctx};
    ATH_CHECK(associationOutHandle.record( std::move(track_association)));
    return StatusCode::SUCCESS;
  }

} // namespace
