/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef RtResolutionChebyshevH
#define RtResolutionChebyshevH

// standard C++ //
#include <cstdlib>
#include <iostream>

// STL //
#include <vector>

// MDT calibration //
#include "MdtCalibData/IRtResolution.h"

namespace MuonCalib {
    /**
    @class  RtResolutionChebyshev
    This class contains the implementation of a spatial resolution @f$ \sigma @f$
    parametrized by a linear combination of Chebyshev polyonomials.

    Convention:

    @f[ \sigma(t) = \sum_{k=0}^{K}
                         p_k*T_k(2*(t-0.5*(tupper+tlower))/(tupper-tlower) @f]
    where @f$ T_k @f$ is the Chebyshev polynomial of k-th order,
    tupper and tlower are upper and lower drift-time bounds.

    Units: [t] = ns, [r] = mm, [v] = mm/ns.

    */

    class RtResolutionChebyshev : public IRtResolution {
    public:
        // Constructors
        /** initialization constructor,

        size of ParVec - 2 = order of the r(t) polynomial,

        ParVec[0] = t_low (smallest allowed drift time),
        ParVec[1] = t_up (largest allowed drift time).
        ParVec[2...] = parameters of the Chebyshev polynomial

        */
        explicit RtResolutionChebyshev(const ParVec& vec);

        // Methods //
        // methods required by the base classes //
        std::string name() const;  //!< get the class name

        //!< get the resolution corresponding to the drift time t;
        //!< if t is not within [t_low, t_up] an unphysical radius of 99999 is
        //!< returned; the background rate is ignored in present implementation
        double resolution(double t, double bgRate = 0.0) const;

        // get-methods specific to the RtResolutionChebyshev class //

        //!< get the lower drift-time bound
        double tLower() const;

        //!< get the upper drift-time bound
        double tUpper() const;

        //!< get the number of parameters used to describe the resolution
        unsigned int numberOfResParameters() const;

        //!< get the coefficients of the r(t) polynomial
        std::vector<double> resParameters() const;

        //!< get the reduced time which is the argument of the Chebyshev polynomial
        double get_reduced_time(const double  t) const;
    };
}  // namespace MuonCalib

#endif
