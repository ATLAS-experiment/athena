/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_RTCHEBYSHEV_H
#define MUONCALIB_RTCHEBYSHEV_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <span>

// MDT calibration //
#include "MdtCalibData/IRtRelation.h"

namespace MuonCalib {
    /**
    @class  RtChebyshev
    This class contains the implementation of an r(t) relationship
    parametrized by a linear combination of Chebyshev polyonomials.

    Convention:

    @f[ r(t) = \sum_{k=0}^{K}
                         p_k*T_k(2*(t-0.5*(tupper+tlower))/(tupper-tlower) @f]
    where T_k is the Chebyshev polynomial of k-th order,
    tupper and tlower are upper and lower drift-time bounds.

    Units: [t] = ns, [r] = mm, [v] = mm/ns. */
    class RtChebyshev : public IRtRelation {

       public:
        // Constructors
        /** initialization constructor,

        size of ParVec - 2 = order of the r(t) polynomial,

        ParVec[0] = t_low (smallest allowed drift time),
        ParVec[1] = t_up (largest allowed drift time).
        ParVec[2...] = parameters of the Chebyshev polynomial

        */

        explicit RtChebyshev(const ParVec& vec) ;
        // Methods //
        // methods required by the base classes //
        virtual std::string name() const override final;  //!< get the class name


        //!< get the radius corresponding to the drift time t;
        //!< if t is not within [t_low, t_up] an unphysical radius of 99999 is returned
        virtual double radius(double t) const override final;
        //!< get the drift velocity
        virtual double driftVelocity(double t) const override final;
        //!< get the drift acceleration
        virtual double driftAcceleration(double t) const override final;
        // get-methods specific to the RtChebyshev class //
        //!< get the lower drift-time bound
        virtual double tLower() const override final;
        //!< get the upper drift-time bound
        virtual double tUpper() const override final;

        virtual double tBinWidth() const override final;

        virtual unsigned nDoF() const override final;

        //!< get the coefficients of the r(t) polynomial
        std::vector<double> rtParameters() const;

    };
}  // namespace MuonCalib

#endif
