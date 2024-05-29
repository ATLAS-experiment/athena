/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include "TrackFindingValidationAlg.h"

namespace ActsTrk
{

  StatusCode TrackFindingValidationAlg::initialize()
  {
     StatusCode sc = TrackTruthMatchingBaseAlg::initialize();
     ATH_CHECK( m_trackToTruth.initialize() );
     return sc;
  }

  StatusCode TrackFindingValidationAlg::finalize()
  {
     StatusCode sc = TrackTruthMatchingBaseAlg::finalize();
     return sc;
  }

  StatusCode TrackFindingValidationAlg::execute(const EventContext &ctx) const
  {
    const TruthParticleHitCounts &truth_particle_hit_counts = getTruthParticleHitCounts(ctx);
    SG::ReadHandle<TrackToTruthParticleAssociation> track_to_truth_handle = SG::makeHandle(m_trackToTruth, ctx);
    if (!track_to_truth_handle.isValid()) {
       ATH_MSG_ERROR("No track to truth particle association for key " << m_trackToTruth.key() );
       return StatusCode::FAILURE;
    }

    EventStat event_stat(truthSelectionTool(),
                         perEtaSize(),
                         perPdgIdSize(),
                         track_to_truth_handle->size());

    for(const HitCountsPerTrack &track_hit_counts : *track_to_truth_handle) {
       analyseTrackTruth(truth_particle_hit_counts,
                         track_hit_counts,
                         event_stat);
    }
    postProcessEventStat(truth_particle_hit_counts,
                         track_to_truth_handle->size(),
                         event_stat);
    return StatusCode::SUCCESS;
  }

}
