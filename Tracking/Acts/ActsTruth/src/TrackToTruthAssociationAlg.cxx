/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackToTruthAssociationAlg.h"

#include "ActsGeometry/ATLASSourceLink.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
#include <iomanip>
#include <cmath>
#include <type_traits>
#include <typeinfo>
#include <numeric>

namespace {
   template <typename T_EnumClass >
   constexpr typename std::underlying_type<T_EnumClass>::type to_underlying(T_EnumClass an_enum) {
      return static_cast<typename std::underlying_type<T_EnumClass>::type>(an_enum);
   }
}
namespace ActsTrk
{
  // to dump
  inline MsgStream &operator<<(MsgStream &out, const ActsUtils::Stat &stat) {
     ActsUtils::dumpStat(out, stat);
     return out;
  }

  TrackToTruthAssociationAlg::TrackToTruthAssociationAlg(const std::string &name,
                                                         ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TrackToTruthAssociationAlg::initialize()
  {
     ATH_CHECK( m_trackingGeometryTool.retrieve() );
     ATH_CHECK( m_tracksContainerKey.initialize() );
     ATH_CHECK( m_pixelClustersToTruth.initialize() );
     ATH_CHECK( m_stripClustersToTruth.initialize() );

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
    std::unique_ptr<TrackToTruthParticleAssociation>
       track_association( std::make_unique<TrackToTruthParticleAssociation>() );

    SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> pixelClustersToTruthAssociation = SG::makeHandle(m_pixelClustersToTruth, ctx);
    if (!pixelClustersToTruthAssociation.isValid()) {
       ATH_MSG_ERROR("No pixel clusterss for key " << m_pixelClustersToTruth.key() );
       return StatusCode::FAILURE;
    }
    SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> stripClustersToTruthAssociation = SG::makeHandle(m_stripClustersToTruth, ctx);
    if (!stripClustersToTruthAssociation.isValid()) {
       ATH_MSG_ERROR("No strip clusterss for key " << m_stripClustersToTruth.key() );
       return StatusCode::FAILURE;
    }
    SG::ReadHandle<ActsTrk::TrackContainer> tracksContainer = SG::makeHandle( m_tracksContainerKey, ctx);
    if (!tracksContainer.isValid()) {
       ATH_MSG_ERROR("No tracks for key " << m_tracksContainerKey.key() );
       return StatusCode::FAILURE;
    }
    track_association->resize( tracksContainer->size() );
    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

    std::array<const ActsTrk::MeasurementToTruthParticleAssociation *,
               static_cast< std::underlying_type<xAOD::UncalibMeasType>::type >(xAOD::UncalibMeasType::sTgcStripType)+1u>
       measurement_to_truth_association_maps{};
    measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::PixelClusterType)]=pixelClustersToTruthAssociation.cptr();
    measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::StripClusterType)]=stripClustersToTruthAssociation.cptr();
    ATH_MSG_DEBUG("Measurement association entries: "  << measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::PixelClusterType)]->size()
                 << " + " << measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::StripClusterType)]->size()
                 );
    unsigned int track_i=0;
    std::array<unsigned int,s_NCounterForAssociatedTruth> tracks_with_associated_truth{};

    std::vector<unsigned int> counted_truth_particles;
    counted_truth_particles.reserve(10);
    --track_i; // to have track_i at the begining of the loop
    for (const typename ActsTrk::TrackContainer::ConstTrackProxy track : *tracksContainer) {
       ++track_i;
       const auto lastMeasurementIndex = track.tipIndex();

       unsigned int n_measurements=0u;

       HitCounterArray &reco_hits = track_association->at(track_i).totalCounts();
       HitCounterArray &noise_hits = track_association->at(track_i).noiseCounts();
       ActsTrk::HitCountsPerTrack::container  &truth_particle_counts = track_association->at(track_i).countsPerTruthParticle();
       tracksContainer->trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [this,
           &n_measurements,
           &measurement_to_truth_association_maps,
           &truth_particle_counts,
           &reco_hits,
           &noise_hits,
           &counted_truth_particles
           ](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {
            if (!state.typeFlags().test(Acts::TrackStateFlag::OutlierFlag) && state.hasUncalibratedSourceLink()) {
              auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
              assert( sl.isValid() && *sl);
              const xAOD::UncalibratedMeasurement &uncalibMeas = **sl;


              const ActsTrk::MeasurementToTruthParticleAssociation *association_map = measurement_to_truth_association_maps.at(to_underlying(uncalibMeas.type()));
              if (association_map) {
                 ++n_measurements;
                 counted_truth_particles.clear();
                 for (const xAOD::TruthParticle *truth_particle : association_map->at(uncalibMeas.index()) ) {
                    const xAOD::TruthParticle *mother_particle = m_elasticDecayUtil.getMother(*truth_particle, m_maxEnergyLoss.value());

                    // do not count hits of associated truth particles again if they are associated to the same elastic decay chain:
                    if (std::find(counted_truth_particles.begin(), counted_truth_particles.end(), mother_particle->index())
                        ==counted_truth_particles.end()) {
                       counted_truth_particles.push_back(mother_particle->index());

                       ActsTrk::HitCountsPerTrack::container::iterator
                          hit_count_iter = std::find_if(truth_particle_counts.begin(),
                                                        truth_particle_counts.end(),
                                                        [mother_particle](const std::pair<const xAOD::TruthParticle *, HitCounterArray > &a) {
                                                           return a.first == mother_particle;
                                                        });
                       if (hit_count_iter == truth_particle_counts.end()) {
                          truth_particle_counts.push_back( std::make_pair(mother_particle, HitCounterArray{}));
                          hit_count_iter = truth_particle_counts.end()-1;
                       }
                       ++(hit_count_iter->second.at( to_underlying(uncalibMeas.type())));
                    }
                 }
                 if (association_map->at(uncalibMeas.index()).empty()) {
                    ++noise_hits.at( to_underlying(uncalibMeas.type()));
                 }
              }
              ++reco_hits.at( to_underlying(uncalibMeas.type()));
            }

          });
       std::sort( truth_particle_counts.begin(),
                  truth_particle_counts.end(),
                  [](const std::pair<const xAOD::TruthParticle *, HitCounterArray > &a,
                     const std::pair<const xAOD::TruthParticle *, HitCounterArray > &b) {
                     return std::accumulate(a.second.begin(),a.second.end(),0u) > std::accumulate(b.second.begin(),b.second.end(),0u);
                  });
       m_associationCounter.fillStatistics(n_measurements, truth_particle_counts.size());
       ++(tracks_with_associated_truth[std::min(truth_particle_counts.size(),tracks_with_associated_truth.size()-1u)]);
    }
    unsigned int idx=0;
    for (unsigned int elm : tracks_with_associated_truth) {
       m_nTracksWithAssociatedTruth[idx] += elm;
       ++idx;
    }

    SG::WriteHandle<TrackToTruthParticleAssociation> associationOutHandle(m_trackToTruthOut, ctx);
    if (associationOutHandle.record( std::move(track_association)).isFailure()) {
       ATH_MSG_ERROR("Failed to record track to truth assocition with key " << m_trackToTruthOut.key() );
       return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

} // namespace
