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
     *  @param segment: Reference to the segment of interest
     *  @param skipOutlier: Switch toggling whether outlier measurements or invalid calib state
     *                      measurements should be ignored */                                                                    
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const xAOD::MuonSegment& segment,
                                                                          bool skipOutlier = true);
    /** @brief Print the chamber ID of a segment, e.g. BMS1A12, meaning that the
     *         segment is in the first BMS eta station on the A-side in sector 12
     *  @param seg: Reference to the segment from which the id should be printed */
    std::string printID(const xAOD::MuonSegment& seg);

    /** @brief Returns the number of associated Uncalibrated measurements
     *  @param segment: Reference to the segment of interest */
    std::size_t nMeasurements(const xAOD::MuonSegment& segment);
    /** @brief Returns the n-th uncalibrated measurement
     *  @param segment: Reference to the segment of interest
     *  @param n: Index of the measurement to retrieve */
    const xAOD::UncalibratedMeasurement* getMeasurement(const xAOD::MuonSegment& segment,
                                                        const std::size_t n);
    /** @brief Returns whether the n-the uncalibrated measurement is an outlier
     *  @param segment: Reference to the segment of interest
     *  @param n: Index of the measurement to retrieve */
    bool isOutlierMeasurement(const xAOD::MuonSegment& segment,
                              const std::size_t n);

}

#endif