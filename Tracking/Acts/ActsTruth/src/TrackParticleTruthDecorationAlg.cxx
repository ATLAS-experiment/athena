/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include "TrackParticleTruthDecorationAlg.h"
#include "ActsEvent/TrackContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include <unordered_map>

#include "ActsEvent/Decoration.h"
#include "xAODMuonViews/ContainerDecorator.h"

namespace {
   using FloatDecorator_t = xAOD::ContainerDecorator<xAOD::TrackParticleContainer, float>;
   using IntDecorator_t = xAOD::ContainerDecorator<xAOD::TrackParticleContainer, int>;
   using UintDecorator_t = xAOD::ContainerDecorator<xAOD::TrackParticleContainer, unsigned>;
   using LinkDecorator_t = xAOD::ContainerDecorator<xAOD::TrackParticleContainer,
                                                   ElementLink<xAOD::TruthParticleContainer>>;
}

namespace ActsTrk
{

  StatusCode TrackParticleTruthDecorationAlg::initialize() {
     ATH_CHECK(TrackTruthMatchingBaseAlg::initialize());
     ATH_CHECK( m_trackToTruth.initialize() );
     ATH_CHECK( m_trkParticleName.initialize() );

     m_floatDecor.emplace_back(m_trkParticleName, "truthMatchProbability");
     m_floatDecor.emplace_back(m_trkParticleName, "truthHitPurity");
     m_floatDecor.emplace_back(m_trkParticleName, "truthHitEfficiency");
     ATH_CHECK(m_floatDecor.initialize());
    
     m_intDecor.emplace_back(m_trkParticleName, "truthType");
     m_intDecor.emplace_back(m_trkParticleName, "truthOrigin");
     ATH_CHECK(m_intDecor.initialize());
     ATH_CHECK(m_truthClassDecor.initialize());
     ATH_CHECK(m_linkDecor.initialize());
     ATH_CHECK(m_derivateTruthKey.initialize(!m_derivateTruthKey.empty()));

     ATH_CHECK(m_truthClassifier.retrieve());
     return StatusCode::SUCCESS;
  }


  StatusCode TrackParticleTruthDecorationAlg::execute(const EventContext &ctx) const
  {
    const TruthParticleHitCounts &truth_particle_hit_counts = getTruthParticleHitCounts(ctx);
    // @TODO or use simply a vector ?
    std::unordered_map<const ActsTrk::TrackContainerBase *, const ActsTrk::TrackToTruthParticleAssociation *> truth_association_map;
    truth_association_map.reserve( m_trackToTruth.size());
    for (const SG::ReadHandleKey<TrackToTruthParticleAssociation> &key : m_trackToTruth) {
      const TrackToTruthParticleAssociation* assocMap{};
      ATH_CHECK(SG::get(assocMap, key, ctx));
      truth_association_map.insert(std::make_pair( assocMap->sourceContainer(), assocMap));
    }

    const xAOD::TrackParticleContainer* track_particle_handle{};
    ATH_CHECK(SG::get(track_particle_handle, m_trkParticleName, ctx));

   const xAOD::TruthParticleContainer* derivateTruth{};
   ATH_CHECK(SG::get(derivateTruth, m_derivateTruthKey, ctx));
   std::vector<std::pair<const xAOD::TruthParticle*, const xAOD::TruthParticle*>> derivateTruthMap{};
   if (derivateTruth) {
      for (const xAOD::TruthParticle* linkMeTo : *derivateTruth) {
            const xAOD::TruthParticle* parentP = xAOD::TruthHelpers::getTruthParticle(*linkMeTo);
            if (!parentP){
               ATH_MSG_ERROR("Link to the original truth particle contianer not present "
                           <<m_derivateTruthKey.fullKey());
               return StatusCode::FAILURE;
            }
            derivateTruthMap.emplace_back(parentP, linkMeTo);
      }
   }
  
    /** @brief Muon tracks are linked to the MuonTruthParticle container instead of the TruthParticles
     *         insert bend the link to the MuonTruthParticle if available */
    auto truthParticleToLink =[&derivateTruthMap](const xAOD::TruthParticle* part) {
         if (!part) {
            return part;
         }
         if (auto itr = std::ranges::find_if(derivateTruthMap, [part](const auto& truthPair){
            return truthPair.first == part;
         }); itr != derivateTruthMap.end()) {
            return itr->second;
         }
         return part;
    };

    std::vector<FloatDecorator_t> float_decor = xAOD::createDecorators(m_floatDecor, ctx, 0.f);
    std::vector<IntDecorator_t> int_decor = xAOD::createDecorators(m_intDecor, ctx, 0);
    UintDecorator_t truthClass_decor{m_truthClassDecor, ctx};
    LinkDecorator_t link_decor{m_linkDecor, ctx};

    EventStat event_stat(*m_truthSelectionTool,
                         perEtaSize(),
                         perPdgIdSize(),
                         track_particle_handle->size());

    std::pair<const ActsTrk::TrackContainerBase *, const ActsTrk::TrackToTruthParticleAssociation *>
             the_track_truth_association{ nullptr, nullptr};
    for(const xAOD::TrackParticle *track_particle : *track_particle_handle) {
       TruthMatchResult truth_match{} ;
       MCTruthPartClassifier::ParticleType type = MCTruthPartClassifier::Unknown;
       MCTruthPartClassifier::ParticleOrigin origin = MCTruthPartClassifier::NonDefined;
       unsigned int classification = 0;
       auto optional_track = getActsTrack(*track_particle);

       if (!optional_track.has_value()) {
         continue;
       }
      const ActsTrk::TrackContainerBase *track_container = &(optional_track->container());
      if (track_container != the_track_truth_association.first) {
         auto truth_association_map_iter = truth_association_map.find( track_container );
         if (truth_association_map_iter != truth_association_map.end()) {
            the_track_truth_association = *truth_association_map_iter;
         } else {
            ATH_MSG_WARNING("No truth association available");
            continue;
         }
      }
      truth_match = analyseTrackTruth(truth_particle_hit_counts,
                                      (*the_track_truth_association.second).at(optional_track->index()),
                                     event_stat);
         
      const xAOD::TruthParticle* truth_particle = truthParticleToLink(truth_match.m_truthParticle);
      if (!truth_particle) {
         continue;
      }
      const auto* container = static_cast<const xAOD::TruthParticleContainer*>(truth_particle->container());

      link_decor(*track_particle) = ElementLink<xAOD::TruthParticleContainer>{*container, truth_particle->index(), ctx};

      auto truthClass = m_truthClassifier->particleTruthClassifier(truth_particle);
      type = truthClass.first;
      origin = truthClass.second;
      classification = std::get<0>(MCTruthPartClassifier::defOrigOfParticle(truth_particle));
      float_decor[kMatchingProbability](*track_particle) = truth_match.m_matchProbability;
      float_decor[kHitPurity](*track_particle) = truth_match.m_hitPurity;
      float_decor[kHitEfficiency](*track_particle) = truth_match.m_hitEfficiency;
      int_decor[kTruthType](*track_particle) = type;
      int_decor[kTruthOrigin](*track_particle) = origin;
      truthClass_decor(*track_particle) = classification;
    }
    postProcessEventStat(truth_particle_hit_counts,
                         track_particle_handle->size(),
                         event_stat);
    return StatusCode::SUCCESS;
  }

}
