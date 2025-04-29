/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_RtSpline_H
#define MUONCALIB_RtSpline_H

//////////////////
// HEADER FILES //
//////////////////

// standard C++ //
#include <cstdlib>
#include <iostream>

// STL //
#include <vector>
#include <memory>
// MDT calibration //
#include "MdtCalibData/IRtRelation.h"

// root
class TSpline3;

namespace MuonCalib {
    /**
    @class  RtSpline
    This class contains the implementation of an r(t) relationship
    parameterized as support points to a cubic spline

    @author Felix.Rauscher@cern.ch
    @date 07.08.2007
    */

    class RtSpline : public IRtRelation {
    private:
        // Spline Class//
        std::unique_ptr<TSpline3> m_sp3{};

    public:
        // Constructors
        /** initialization constructor,

        2 * (size of ParVec)  = number of points

        ParVec[    2n] = x coordinate of n_th support point
        ParVec[1 + 2n] = y coordinate of n_th support point

        */
        explicit RtSpline(const ParVec &vec);

        virtual ~RtSpline();

        // Methods //
        // methods required by the base classes //
        inline std::string name() const override final{ return "RtSpline"; }

        //!< get the class name
        virtual double radius(double t) const override final;
        //!< get the radius corresponding to the
        //!< drift time t;
        //!< 0 or 14.6 is returned if t is outside
        //!< the range
        virtual double driftVelocity(double t) const override final;
        //!< get the drift velocity
       virtual double driftAcceleration(double t) const override final;
       

        // get-methods specific to the RtSpline class //
        virtual double tLower() const override final;
        //!< get the lower drift-time bound
        virtual double tUpper() const override final;
        //!< get the upper drift-time bound
        virtual double tBinWidth() const override final;

        virtual unsigned nDoF() const override final;
    };
}  // namespace MuonCalib

#endif
