/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_TRACKINGHELPERS_H
#define MUONTRACKEVENT_TRACKINGHELPERS_H

#include "MuonPatternEvent/Segment.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODTracking/TrackParticle.h"

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
    /** @brief Print the details of a segment
     *  @param seg: Reference to the segment from which the details should be printed */
    std::string printSegment(const xAOD::MuonSegment& seg);
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
    
    
    
    /** @brief Expresses the segment position on the surface of the first
     *         measurement 
     *  @param gctx: Geometry context to align the measurment surfaces
     *  @param segment: Reference to the segment to be expressed
     *  @param skipOutlier: Allow the first surface to be an outlier*/
    Amg::Vector3D atFirstSurface(const Acts::GeometryContext& gctx,
                                 const xAOD::MuonSegment& segment,
                                 const bool skipOutlier = true);
    /** @brief Retrieves the first measurement associated with the segment
     *  @param segment: Refernece to the segment for which the measurement
     *                  shall be returned
     * @param skipOutlier: If true, it is ensured that the first measurement is
     *                     not an outlier */
    const xAOD::UncalibratedMeasurement* firstMeasurement(const xAOD::MuonSegment& segment,
                                                          const bool skipOutlier =true);
    /** @brief Returns the identifier of the volume in which the surface is embedded
     *  @param surface: Reference to the surface of interest */
    Acts::GeometryIdentifier volumeId(const Acts::Surface& surface);
    /** @brief Auxiliary class to sort the particles by momentum
     *         First sorting is by pt, then eta and finally by phi */
    struct ParticleSorter{
        bool operator()(const xAOD::IParticle* a,
                        const xAOD::IParticle* b) const;
    }; 

}

#endif