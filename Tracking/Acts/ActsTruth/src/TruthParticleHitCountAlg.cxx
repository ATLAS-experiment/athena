/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthParticleHitCountAlg.h"

#include "ActsGeometry/ATLASSourceLink.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
#include <iomanip>
#include <cmath>
#include <type_traits>
#include <typeinfo>
#include <numeric>

// for erase_if detection i.e. >=c++20
#include <version>

namespace {
   template <typename T_EnumClass >
   constexpr typename std::underlying_type<T_EnumClass>::type to_underlying(T_EnumClass an_enum) {
      return static_cast<typename std::underlying_type<T_EnumClass>::type>(an_enum);
   }
}

#ifndef  __cpp_lib_erase_if
namespace std {
   template <class T_container, class T_Func>
   inline std::size_t erase_if(T_container &container, T_Func pred) {
      std::size_t orig_size = container->size();
      for ( typename T_container::iterator iter = container.begin();
            iter != container.end();
            /* increment only if nothing was erased! */ ) {
         if (pred(*iter)) {
            iter = erase(iter);
         }
         else {
            ++iter;
         }
      }
      return orig_size - container->size();
   }
}
#endif

namespace ActsTrk
{
  // to dump
  inline MsgStream &operator<<(MsgStream &out, const ActsUtils::Stat &stat) {
     ActsUtils::dumpStat(out, stat);
     return out;
  }

  TruthParticleHitCountAlg::TruthParticleHitCountAlg(const std::string &name,
                                                         ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator)
  {
  }

  StatusCode TruthParticleHitCountAlg::initialize()
  {
     ATH_CHECK( m_trackingGeometryTool.retrieve() );
     ATH_CHECK( m_pixelClustersToTruth.initialize() );
     ATH_CHECK( m_stripClustersToTruth.initialize() );
     ATH_CHECK( m_hgtdClustersToTruth.initialize() );

     ATH_CHECK( m_truthHitCountsOut.initialize() );

     m_elasticDecayUtil.setEnergyLossBinning(m_energyLossBinning.value());
     return StatusCode::SUCCESS;
  }

  template <bool IsDebug>
  template <class T_OutStream>
  inline void TruthParticleHitCountAlg::AssociationCounter<IsDebug>::dumpStatistics(T_OutStream &out) const {
     if constexpr(IsDebug) {
        out << "Measurements per truth particle :" << m_measPerTruthParticle << std::endl
            << m_measPerTruthParticle.histogramToString();
     }
  }
  template <bool IsDebug>
  void inline TruthParticleHitCountAlg::AssociationCounter<IsDebug>::fillStatistics(unsigned int n_measurements) const {
     if constexpr(IsDebug) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_measPerTruthParticle.add(n_measurements);
     }
  }

  StatusCode TruthParticleHitCountAlg::finalize()
  {
     ATH_MSG_INFO("Number of truth particles with hits : " << m_nTruthParticlesWithHits);
     if (msgLvl(MSG::INFO)) {
        if constexpr(TruthParticleHitCountDebugHists) {
           m_elasticDecayUtil.dumpStatistics(msg());
           msg() << std::endl;
           m_associationCounter.dumpStatistics(msg());
        }
        msg() << endmsg;
     }
     return StatusCode::SUCCESS;
  }

  StatusCode TruthParticleHitCountAlg::execute(const EventContext &ctx) const
  {
    std::unique_ptr<TruthParticleHitCounts>
       truth_particle_hit_counts( std::make_unique<TruthParticleHitCounts>() );

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
    SG::ReadHandle<ActsTrk::MeasurementToTruthParticleAssociation> hgtdClustersToTruthAssociation = SG::makeHandle(m_hgtdClustersToTruth, ctx);
    if(!hgtdClustersToTruthAssociation.isValid()) {
       ATH_MSG_DEBUG("No HGTD clusterss for key " << m_hgtdClustersToTruth.key() );
    }
    Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

    std::array<const ActsTrk::MeasurementToTruthParticleAssociation *,
               static_cast< std::underlying_type<xAOD::UncalibMeasType>::type >(xAOD::UncalibMeasType::nTypes)>
       measurement_to_truth_association_maps{};
    measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::PixelClusterType)]=pixelClustersToTruthAssociation.cptr();
    measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::StripClusterType)]=stripClustersToTruthAssociation.cptr();
    measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::HGTDClusterType)]=hgtdClustersToTruthAssociation.cptr();
    ATH_MSG_DEBUG("Measurement association entries: "
                  << measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::PixelClusterType)]->size()
                  << " + " << measurement_to_truth_association_maps[to_underlying(xAOD::UncalibMeasType::StripClusterType)]->size());

    unsigned int measurement_type_i=0;
    --measurement_type_i;
    for( const ActsTrk::MeasurementToTruthParticleAssociation *associated_truth_particles : measurement_to_truth_association_maps ) {
       ++measurement_type_i;
       if (!associated_truth_particles) continue;
       for (const ActsTrk::ParticleVector &truth_particles : *associated_truth_particles) {
          for (const xAOD::TruthParticle *truth_particle : truth_particles) {
             assert (truth_particle);
             const xAOD::TruthParticle *mother_particle = m_elasticDecayUtil.getMother(*truth_particle, m_maxEnergyLoss.value());
             if (mother_particle) {
                assert(measurement_type_i < (*truth_particle_hit_counts)[mother_particle].size());
                ++(*truth_particle_hit_counts)[mother_particle][measurement_type_i];
             }
          }
       }
    }
    unsigned int n_hits_min = m_nHitsMin.value();
    unsigned int truth_particles_without_enough_measurements
       = std::erase_if( *truth_particle_hit_counts,
                        [this, n_hits_min](const std::pair<const xAOD::TruthParticle * const,HitCounterArray> &elm) {
                           unsigned int n_measurements=std::accumulate(elm.second.begin(), elm.second.end(),0u);
                           this->m_associationCounter.fillStatistics(n_measurements);
                           return n_measurements<n_hits_min;
                        });

    m_nTruthParticlesWithHits += truth_particle_hit_counts->size();

    ATH_MSG_DEBUG("Truth particles with hits:" << truth_particle_hit_counts->size()
                  << ", without enough hits: " << truth_particles_without_enough_measurements);

    SG::WriteHandle<TruthParticleHitCounts> truth_particle_hit_counts_out_handle(m_truthHitCountsOut, ctx);
    if (truth_particle_hit_counts_out_handle.record( std::move(truth_particle_hit_counts)).isFailure()) {
       ATH_MSG_ERROR("Failed to record track to truth assocition with key " << m_truthHitCountsOut.key() );
       return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

} // namespace
