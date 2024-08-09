/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_SEGMENTFITEVENTDATA__H
#define MUONR4_MUONPATTERNEVENT_SEGMENTFITEVENTDATA__H

#include <GeoPrimitives/GeoPrimitives.h>
#include <MuonPatternEvent/MuonHoughDefs.h>


class ActsGeometryContext;
namespace MuonR4{
    class CalibratedSpacePoint;
    namespace SegmentFit {
        /**  @brief Returns the parsed parameters into an Eigen line parametrization.
         *          The first operand is the position. The other is the direction. */
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine(const Parameters& pars);

        std::string makeLabel(const Parameters& pars);
        std::string toString(const Parameters& pars);
    }


    struct SegmentFitResult {
        SegmentFitResult() = default;
        using AxisDefs = SegmentFit::AxisDefs;
        using Parameters = SegmentFit::Parameters;
        using Covariance = SegmentFit::Covariance;

        using HitType = std::unique_ptr<CalibratedSpacePoint>;
        using HitVec = std::vector<HitType>;
        
        /** @brief Does the candidate have phi measurements */
        bool hasPhi{false};
        /** @brief Final segment parameters */
        Parameters segmentPars{};
        /** @brief Uncertainties on the segment parameters */
        Covariance segmentParErrs{};
        /** @brief Calibrated measurements used in the fit */
        HitVec calibMeasurements{};
        /** @brief Chis per measurement to identify outliers */
        std::vector<double> chi2PerMeasurement{};
        /** @brief chi2 of the fit */
        double chi2{0.};
        /** @brief degrees of freedom */
        int nDoF{0};
        /** @brief Is the fit converged */
        bool converged{false};
        /** @brief Number of iterations called to reach the minimum */
        unsigned int nIter{0};

        /** @brief Returns the defining parameters as a pair of Amg::Vector3D
         *         The first part is the position expressed at the chamber centre
         *         The second part is the direction expressed at the chamber centre */
        std::pair<Amg::Vector3D, Amg::Vector3D> makeLine() const {
            return SegmentFit::makeLine(segmentPars);
        }       
   };

}

#endif 
