/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include "TrackFindingValidationAlg.h"

namespace ActsTrk
{

  StatusCode TrackFindingValidationAlg::initialize() {
     ATH_CHECK( m_trackToTruth.initialize() );
     return TrackTruthMatchingBaseAlg::initialize();
  }

  StatusCode TrackFindingValidationAlg::execute(const EventContext &ctx) const {
    const TruthParticleHitCounts& truth_particle_hit_counts = getTruthParticleHitCounts(ctx);
    const TrackToTruthParticleAssociation* track_to_truth_handle{};
    ATH_CHECK(SG::get(track_to_truth_handle, m_trackToTruth, ctx));
    EventStat event_stat(*m_truthSelectionTool,
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
