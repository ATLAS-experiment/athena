/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCalib_RtFromPointsH
#define MuonCalib_RtFromPointsH

// STL //
#include <vector>
#include <memory>
// MuonCalib //
#include "MuonCalibMath/SamplePoint.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/ITrRelation.h"
#include "MdtCalibData/IRtResolution.h"

namespace MuonCalib {

    /// \class RtFromPoints
    /// This class allows the user to retrieve an RtChebyshev or RtRelationLookUp
    /// object corresponding to a set of (r, t) points.

    class RtFromPoints {
    public:
        /** @brief Converts a list of r-t data points into a r(t) relation expressed as a series of 
         *         chebychev polynomials
         *  @param dataPoints: List of data points to be converted x1 -> drift time & x2 -> drift radius
         *  @param order: Number of chebychev polynomials to fit */
        static std::unique_ptr<IRtRelation> getRtChebyshev(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of r-t data points into a t(r) relation expressed as a 
         *        series of chebychev polynomials 
         *  @param dataPoints: List of data points to be converted x1 -> drift radius & x2 -> drift time
         *  @param order: Number of chebychev polynomials to fit */
        static std::unique_ptr<ITrRelation> getTrChebyshev(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of reso - t  into a reso(t) relation expressed as a series of 
         *         chebychev polynomials
         *  @param dataPoints: List of data points to be converted x1 -> drift time & x2 -> resolution
         *  @param order: Number of chebychev polynomials to fit */
        static std::unique_ptr<IRtResolution> getResoChebyshev(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of sample points into a drift radius resolution function. The resolution is 
         *         parametrized as a series of Chebychev polynomials.
         *  @param dataPoints: List of data points to convert. The points need to have a resolution attached
         *  @param rtRelPtr: Pointer to the rt-relation acting as mediator from t -> r
         *  @param relUnc: Relative uncertainty on each point.
         *  @param order: Order of the chebychev polynomial */
        static std::unique_ptr<IRtResolution> getResoChebyshev(const std::vector<SamplePoint>& dataPoints,
                                                               const IRtRelationPtr rtRelPtr, 
                                                               const double relUnc,
                                                               const unsigned order);
        
        /** @brief Converts a list of r-t data points into a r(t) relation expressed as a series
         *         of legendre polynomials
         *  @param dataPoints: List of data points to be converted x1 -> drift radius & x2 -> drift time
         *  @param order: Number of legendre polynomials to use in the fit */
        static std::unique_ptr<IRtRelation> getRtLegendre(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of t(r) data points into a t(r) relation expressed as a 
         *        series of legendre polynomials 
         *  @param dataPoints: List of data points to be converted x1 -> drift radius & x2 -> drift time
         *  @param order: Number of chebychev polynomials to fit */
        static std::unique_ptr<ITrRelation> getTrLegendre(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of r(t) data points into a r(t) relation expressed as a
         *         series of elementary monomonials
         * @param dataPoints: List of data points to be converted x1 -> drift time & x2 -> drift radius
         * @param order: Order of the maximum monomial in the fit */
        static std::unique_ptr<IRtRelation> getRtSimplePoly(const std::vector<SamplePoint>& dataPoints, const unsigned order);
        /** @brief Converts a list of t(r) data points into a t(r) relation expressed as a
         *         series of elementary monomonials
         * @param dataPoints: List of data points to be converted x1 -> drift radius & x2 -> drift time
         * @param order: Order of the maximum monomial in the fit */
        static std::unique_ptr<ITrRelation> getTrSimplePoly(const std::vector<SamplePoint>& dataPoints, const unsigned order);

        ///< get an RtRelationLookUp resembling the r(t) function as
        ///< described by the sample points in  the vector "sample_points";
        ///< x1 coordinate of the sample points contains the drift time,
        ///< x2 the corresponding radius; the method takes the minimum and
        ///< maximum x1 values in the set of sample points a lower and upper
        ///< limits in RtRelationLookUp
        static std::unique_ptr<IRtRelation> getRtRelationLookUp(const std::vector<SamplePoint>& sample_points);

        private:
            /** @brief Executes the fit of  chebychev polynomials to the data points
             *  @param dataPoints: Data points to fit. No normalization of the domain required
             *  @param order: Number of Chebychev polynomials to use in the fit */
            static CalibFunc::ParVec chebyFit(const std::vector<SamplePoint>& dataPoints,
                                              const unsigned order);

            /** @brief Executes the fit of Legendre polynomials to the data points
             *  @param dataPoints: Data points to fit. No normalization of the domain required
             *  @param order: Number of Legendre polynomials to use in the fit */
            static CalibFunc::ParVec legendreFit(const std::vector<SamplePoint>& dataPoints,
                                                 const unsigned order);
            /** @brief Exectues the fit of simple monomials to the data points
             *  @param dataPoints: Data points to fit.
             *  @param order: Number of Legendre polynomials to use in the fit */
            static CalibFunc::ParVec simplePolyFit(const std::vector<SamplePoint>& dataPoints,
                                                   const unsigned order);
            
    };

}  // namespace MuonCalib

#endif
