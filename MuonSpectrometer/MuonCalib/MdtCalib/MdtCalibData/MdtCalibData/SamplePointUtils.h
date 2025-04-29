/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDTCALIBDATA_SAMPLEPOINTUTILS_H
#define MDTCALIBDATA_SAMPLEPOINTUTILS_H

#include "MuonCalibMath/SamplePoint.h"

#include <vector>
#include <utility>

namespace MuonCalib{
    class IRtRelation;
    class ITrRelation;
    class IRtResolution;

    /** @brief Constructs a list of sample points from the rt-relation. 
     *         The step-wdith between two points is taken from the rtRel bin width
     *         method and each point has the same uncertainty of 1 assigned
     *  @param rtRel: Reference of the rtRelation to transform to points. */
    std::vector<SamplePoint> fetchDataPoints(const IRtRelation& rtRel,
                                             const double relUnc);
    /** @brief Constructs a list of sample points from the rt-relation. 
     *         The step-wdith between two points is taken from the rtRel bin width
     *         method and each point is weighted with the relative resolution
     *  @param rtRel: Reference of the rtRelation to transform to points.
     *  @param reo: Reference to the r-t resolution function */
    std::vector<SamplePoint> fetchDataPoints(const IRtRelation& rtRel, 
                                             const IRtResolution& resoFunc);
    /** @brief Creates a new vector of samples points with x1 exchanged by x2 and 
     *         vice-versa. Each point has the same uncertainty assigned.
     *  @param points: List of rt- sample points of interest
     *  @param rtRel: Reference to the rt-relation to calculate each point's uncertainty. */
    std::vector<SamplePoint> swapCoordinates(const std::vector<SamplePoint>& points,
                                             const IRtRelation& rtRel);
    /** @brief Creates a new vector of sample points where the x2 coordinate becomes
     *         the x1 coordinate and the resolution becomes x2. Each point has the 
     *         same uncertainty assigned
     *  @param points: List of rt- sample points of interest
     *  @param uncert: Uncertainty oon each point */
    std::vector<SamplePoint> resoFromRadius(const std::vector<SamplePoint>& points,
                                            const double uncert);
    /** @brief Creates a new vector of sample points where the x2 is assigned to the 
     *         uncertainty and the uncertainty is replaced by a constant value
     *  @param: points: List of data points to transform
     *  @param uncert: Uncertainty to assign instead */
    std::vector<SamplePoint> fetchResolution(const std::vector<SamplePoint>& points,
                                             const double uncert);
    /** @brief Normalizes the domain of the samples points to the interval -1 to 1 */
    std::vector<SamplePoint> normalizeDomain(const std::vector<SamplePoint>& dataPoints);
    /** @brief Returns the chi2 of the rt-relation w.r.t. the data points */
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const IRtRelation& rtRel);
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const ITrRelation& trRel);
    double calculateChi2(const std::vector<SamplePoint>& dataPoints,
                         const IRtResolution& rtReso);
                         

    /** @brief Returns the minimum & maximum values covered by the sample points */
    std::pair<double, double> minMax(const std::vector<SamplePoint>& points);
    /** @brief Returns the interval covered by the sample points */
    std::pair<double, double> interval(const std::vector<SamplePoint>& points); 


}
#endif