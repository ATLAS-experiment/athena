/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include "TrackParticleTruthDecorationAlg.h"
#include "ActsEvent/TrackContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <unordered_map>
#include "decoratorUtils.h"
#include "ActsEvent/Decoration.h"



namespace ActsTrk
{

  StatusCode TrackParticleTruthDecorationAlg::initialize()
  {
     StatusCode sc = TrackTruthMatchingBaseAlg::initialize();
     ATH_CHECK( m_trackToTruth.initialize() );
     ATH_CHECK( m_trkParticleName.initialize() );
     std::vector<std::string> float_decor_names(kNFloatDecorators);
     float_decor_names[kMatchingProbability]="truthMatchProbability";
     float_decor_names[kHitPurity]="truthHitPurity";
     float_decor_names[kHitEfficiency]="truthHitEfficiency";
     createDecoratorKeys(*this,m_trkParticleName,"" /*prefix ? */, float_decor_names,m_floatDecor);
     assert( m_floatDecor.size() == kNFloatDecorators);

     std::vector<std::string> int_decor_names(kNIntDecorators);
     int_decor_names[kTruthType]="truthType";
     int_decor_names[kTruthOrigin]="truthOrigin";
     createDecoratorKeys(*this,m_trkParticleName,"" /*prefix ? */, int_decor_names,m_intDecor);
     assert( m_intDecor.size() == kNIntDecorators);

     ATH_CHECK(m_truthClassDecor.initialize());
     ATH_CHECK(m_linkDecor.initialize());

     ATH_CHECK(m_truthClassifier.retrieve());
     return sc;
  }

  StatusCode TrackParticleTruthDecorationAlg::finalize()
  {
     StatusCode sc = TrackTruthMatchingBaseAlg::finalize();
     return sc;
  }

  StatusCode TrackParticleTruthDecorationAlg::execute(const EventContext &ctx) const
  {
    const TruthParticleHitCounts &truth_particle_hit_counts = getTruthParticleHitCounts(ctx);
    // @TODO or use simply a vector ?
    std::unordered_map<const ActsTrk::TrackContainerBase *, const ActsTrk::TrackToTruthParticleAssociation *> truth_association_map;
    truth_association_map.reserve( m_trackToTruth.size());
    for (const SG::ReadHandleKey<TrackToTruthParticleAssociation> &truth_association_key : m_trackToTruth) {
       SG::ReadHandle<TrackToTruthParticleAssociation> track_to_truth_handle = SG::makeHandle(truth_association_key, ctx);
       if (!track_to_truth_handle.isValid()) {
          ATH_MSG_ERROR("No track to truth particle association for key " << truth_association_key.key() );
          return StatusCode::FAILURE;
       }
       truth_association_map.insert(std::make_pair( track_to_truth_handle->sourceContainer(), track_to_truth_handle.cptr() ));
    }

    SG::ReadHandle<xAOD::TrackParticleContainer> track_particle_handle = SG::makeHandle(m_trkParticleName, ctx);
    if (!track_particle_handle.isValid()) {
       ATH_MSG_ERROR("No track particle container for key " << track_particle_handle.key() );
       return StatusCode::FAILURE;
    }
    std::vector< SG::WriteDecorHandle<xAOD::TrackParticleContainer,float > >
       float_decor( createDecorators<xAOD::TrackParticleContainer, float >(m_floatDecor, ctx) );
    std::vector< SG::WriteDecorHandle<xAOD::TrackParticleContainer,int > >
       int_decor( createDecorators<xAOD::TrackParticleContainer, int >(m_intDecor, ctx) );
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, unsigned int> truthClass_decor(m_truthClassDecor, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, ElementLink<xAOD::TruthParticleContainer> > link_decor(m_linkDecor, ctx);

    EventStat event_stat(truthSelectionTool(),
                         perEtaSize(),
                         perPdgIdSize(),
                         track_particle_handle->size());

    std::pair<const ActsTrk::TrackContainerBase *, const ActsTrk::TrackToTruthParticleAssociation *>
       the_track_truth_association{ nullptr, nullptr};
    ElementLink<xAOD::TruthParticleContainer> ref_truth_link;
    for(const xAOD::TrackParticle *track_particle : *track_particle_handle) {
       TruthMatchResult truth_match{} ;
       MCTruthPartClassifier::ParticleType type = MCTruthPartClassifier::Unknown;
       MCTruthPartClassifier::ParticleOrigin origin = MCTruthPartClassifier::NonDefined;
       unsigned int classification = 0;

       {
          std::optional<ActsTrk::TrackContainer::ConstTrackProxy> optional_track = getActsTrack(*track_particle);
          if (optional_track.has_value()) {
             const ActsTrk::TrackContainerBase *track_container = &(optional_track.value().container());
             if (track_container != the_track_truth_association.first && track_container ) {
                std::unordered_map<const ActsTrk::TrackContainerBase *, const ActsTrk::TrackToTruthParticleAssociation *>::const_iterator
                   truth_association_map_iter = truth_association_map.find( track_container );
                if (truth_association_map_iter != truth_association_map.end()) {
                   the_track_truth_association = *truth_association_map_iter;
                }
             }
             if (the_track_truth_association.second) {
                truth_match =  analyseTrackTruth(truth_particle_hit_counts,
                                                 (*the_track_truth_association.second).at(optional_track.value().index()),
                                                 event_stat);

                const xAOD::TruthParticle *truth_particle = truth_match.m_truthParticle;

                // decorate track particle with link to truth particle, matching probability etc.
                if (truth_particle) {
                   if (!ref_truth_link.isValid()) {
                      const xAOD::TruthParticleContainer *truth_particle_container
                         = dynamic_cast<const xAOD::TruthParticleContainer *>(truth_particle->container());
                      if (!truth_particle_container) {
                         ATH_MSG_ERROR("Valid truth particle not part of a xAOD::TruthParticleContainer");
                      }
                      else {
                         ref_truth_link=  ElementLink<xAOD::TruthParticleContainer>(*truth_particle_container,0u,ctx);
                      }
                   }
                   assert( truth_particle->container() == ref_truth_link.getStorableObjectPointer() );
                   link_decor(*track_particle) = ElementLink<xAOD::TruthParticleContainer>(ref_truth_link, truth_particle->index());

		   auto truthClass = m_truthClassifier->particleTruthClassifier(truth_particle);
		   type = truthClass.first;
		   origin = truthClass.second;
		   classification = std::get<0>(MCTruthPartClassifier::defOrigOfParticle(truth_particle));
                }
                else {
                   link_decor(*track_particle) = ElementLink<xAOD::TruthParticleContainer>();
                }
             }
          }
       }
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
