/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TRACKSORT_H
#define TAURECTOOLS_TRACKSORT_H

#include "AthLinks/ElementLink.h"
#include "xAODTracking/TrackParticle.h"

/**
 * @brief Helper method to sort tracks
 *
 * Usage:
 * sort(track_begin, track_end, tauTrackSort);
 * We want pt0 > pt1 > ..., so the test on ptInvVert is < .
 *
 */

class TrackSort {

public:
  TrackSort() = default;

  bool operator() ( const xAOD::TrackParticle *t1, const xAOD::TrackParticle *t2 ) const
  {
    return std::abs( t1->pt() ) > std::abs( t2->pt() );
  };

};

#endif // TAURECTOOLS_TRACKSORT_H
