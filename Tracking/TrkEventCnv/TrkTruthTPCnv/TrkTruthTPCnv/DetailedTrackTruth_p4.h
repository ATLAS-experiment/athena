/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

// Andrei.Gaponenko@cern.ch, 2008
// Olivier.Arnaez@cern.ch, 2015


#ifndef TRKTRUTHTPCNV_DETAILEDTRACKTRUTH_P4_H
#define TRKTRUTHTPCNV_DETAILEDTRACKTRUTH_P4_H

#include "TrkTruthTPCnv/SubDetHitStatistics_p0.h"
#include "TrkTruthTPCnv/TruthTrajectory_p3.h"

#include "AthenaKernel/CLASS_DEF.h"

namespace Trk {

  class DetailedTrackTruth_p4 {
  public:
    SubDetHitStatistics_p0 m_hitsCommon;
    SubDetHitStatistics_p0 m_hitsTrack;
    SubDetHitStatistics_p0 m_hitsTruth;
    TruthTrajectory_p3 m_trajectory;
  };
}

CLASS_DEF( Trk::DetailedTrackTruth_p4 , 104029962 , 1 )

#endif/*TRKTRUTHTPCNV_DETAILEDTRACKTRUTH_P4_H*/
