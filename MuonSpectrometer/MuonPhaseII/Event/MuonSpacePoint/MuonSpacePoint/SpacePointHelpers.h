/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_MUONSPACEPOINTHELPERS_H
#define MUONSPACEPOINT_MUONSPACEPOINTHELPERS_H

#include "MuonSpacePoint/SpacePoint.h"
#include <MuonSpacePoint/CalibratedSpacePoint.h>

/** @brief Set of helper functions for uncalibrated / calibrated space points meant for local and global pattern recognition. */
namespace MuonR4 {

    /** @brief Returns whether the uncalibrated spacepoint is a precision hit (Mdt, micromegas, stgc strips)
     *  @param hit: Reference to the uncalibrated space point */
    bool isPrecisionHit(const SpacePoint& hit);

    /** @brief Returns whether the calibrated spacepoint is valid and therefore suitable to be used in the segment fit
     *  @param hit: Reference to the calibrated space point of interest */
    bool isGoodHit(const CalibratedSpacePoint& hit);
    /** @brief Returns whether the calibrated spacepoint is a precision hit (Mdt, micromegas, stgc strips)
     *  @param hit: Reference to the calibrated space point of interest */
    bool isPrecisionHit(const CalibratedSpacePoint& hit);
    
}
#endif
