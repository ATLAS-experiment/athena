/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_TRACKINGHELPERS_H
#define MUONTRACKEVENT_TRACKINGHELPERS_H

#include "MuonPatternEvent/Segment.h"
#include "xAODMuon/MuonSegment.h"

namespace MuonR4{
    /** @brief Helper function to navigate from the xAOD::MuonSegment to the MuonR4::Segment.
     *         The segment should be decorated with 'parentSegment' decoration.
     *  @param seg: Reference to the segment of interest. */
    const Segment* detailedSegment(const xAOD::MuonSegment& seg);
}

#endif