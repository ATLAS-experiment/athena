/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthParticleHitCountAlg.h"

#include "Acts/Utilities/Enumerate.hpp"

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"
#include <iomanip>
#include <cmath>
#include <type_traits>
#include <typeinfo>
#include <numeric>
#include <ranges>

namespace ActsTrk
{
  // to dump
  inline MsgStream &operator<<(MsgStream &out, const ActsUtils::Stat &stat) {
     ActsUtils::dumpStat(out, stat);
     return out;
  }

  StatusCode TruthParticleHitCountAlg::initialize() {
     ATH_CHECK( m_ctxProvider.initialize() );
     ATH_CHECK( m_measAssocKeys.initialize());
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

  StatusCode TruthParticleHitCountAlg::execute(const EventContext &ctx) const {
    auto truth_particle_hit_counts = std::make_unique<TruthParticleHitCounts>();

    const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
    using AssocMapVec_t = std::vector<const ActsTrk::MeasurementToTruthParticleAssociation *>;
    std::array<AssocMapVec_t, Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)>
       measurement_to_truth_association_maps{};
    for (const auto& key : m_measAssocKeys) {
       const ActsTrk::MeasurementToTruthParticleAssociation* assocMap{};
       ATH_CHECK(SG::get(assocMap, key, ctx));
       if (assocMap->empty()) {
            continue;
       }
       const auto typeIdx = Acts::toUnderlying(assocMap->sourceContainer()->front()->type());
       measurement_to_truth_association_maps[typeIdx].push_back(assocMap);
    }

    for(const auto&[measurement_type_i, mapsPerType]: Acts::enumerate(measurement_to_truth_association_maps) ) {
      for (const ActsTrk::MeasurementToTruthParticleAssociation* associated_truth_particles : mapsPerType) {
         for (const ActsTrk::ParticleVector &truth_particles : *associated_truth_particles) {
            for (const xAOD::TruthParticle *truth_particle : truth_particles) {
               assert (truth_particle);
               const xAOD::TruthParticle *mother_particle = m_elasticDecayUtil.getMother(*truth_particle, m_maxEnergyLoss.value());
               if (mother_particle) {
                   ++(*truth_particle_hit_counts)[mother_particle][measurement_type_i];
               }
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
