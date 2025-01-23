/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_RTSIMPLEPOLYNOMIAL_H
#define MUONCALIB_RTSIMPLEPOLYNOMIAL_H

#include <cstdlib>
#include <iostream>
#include <vector>

// MDT calibration //
#include "MdtCalibData/IRtRelation.h"

namespace MuonCalib {
    class RtSimplePolynomial : public IRtRelation {

       public:

        explicit RtSimplePolynomial(const ParVec& vec) ;
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
