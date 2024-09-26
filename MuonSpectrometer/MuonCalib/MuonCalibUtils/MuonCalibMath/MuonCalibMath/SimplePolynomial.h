/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCalib_SimplePolynomialH
#define MuonCalib_SimplePolynomialH

#include "MuonCalibMath/BaseFunction.h"

namespace MuonCalib {
    /// \class SimplePolynomial
    /// This class provides the simple polynomials x^k (k=0, 1, ...) as base
    /// functions for fits.
    class SimplePolynomial : public BaseFunction {
        public:
            // Constructor //
            SimplePolynomial() = default;
            // Method  get x^k
            double value(const int k, const double x) const;
            
    };
}

#endif
