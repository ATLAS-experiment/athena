/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibMath/LegendrePolynomial.h"
#include "Acts/Utilities/detail/Polynomials.hpp"
#include "cmath"

using namespace MuonCalib;

double LegendrePolynomial::value(const int  k, const double  x) const {
    return Acts::detail::legendrePoly(x,k);
}
