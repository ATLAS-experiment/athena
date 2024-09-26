/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ChebyshevPolynomialH
#define ChebyshevPolynomialH

#include "math.h"
#include "MuonCalibMath/BaseFunction.h"

namespace MuonCalib {

/**
  @class ChebyshevPolynomial
          This class class provides a Chebyshev polynomial of order k.  */

  class ChebyshevPolynomial : public BaseFunction {
      public:
        // Constructor
        ChebyshevPolynomial() = default;  //!< default constructor
        // Methods
        /** get the value of the Chebyshev polynomial of k-th order at x, (-1 <= x <= 1) */
        double value(const int k, const double x) const;
    };
}

#endif
