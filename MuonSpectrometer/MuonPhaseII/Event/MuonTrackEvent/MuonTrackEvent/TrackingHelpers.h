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
    /** @brief Helper function to extract the measurements from the segment
     *  @param seg: Reference to the segment of interest
     *  @param skipOutlier: Switch toggling whether outlier measurements or invalid calib state
     *                      measurements should be ignored */
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const Segment& seg,
                                                                          bool skipOutlier = true);
    /** @brief Helper function to extract the measurements from the segment
     *  @param seg: Reference to the segment of interest
     *  @param skipOutlier: Switch toggling whether outlier measurements or invalid calib state
     *                      measurements should be ignored */                                                                    
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const xAOD::MuonSegment& segment,
                                                                          bool skipOutlier = true);
    /** @brief Print the chamber ID of a segment, e.g. BMS1A12, meaning that the
     *         segment is in the first BMS eta station on the A-side in sector 12
     *  @param seg: Reference to the segment from which the id should be printed */
    std::string printID(const xAOD::MuonSegment& seg);


}

#endif