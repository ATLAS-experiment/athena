/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef LegendrePolynomialH
#define LegendrePolynomialH

// standard C++ //
#include "math.h"

// MuonCalib //
#include "MuonCalibMath/BaseFunction.h"


namespace MuonCalib {

    /// \class LegendrePolynomial
    /// This class provides a legendre polynomial of order k. It is derived from
    /// BaseFunction.
    class LegendrePolynomial : public BaseFunction {

        public:
            LegendrePolynomial() = default;

            // Methods //
            double value(const int k, const double x) const override final;
    };
}
#endif
