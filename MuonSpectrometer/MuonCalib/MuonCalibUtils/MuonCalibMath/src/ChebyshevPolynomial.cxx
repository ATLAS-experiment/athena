/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibMath/ChebyshevPolynomial.h"
#include "Acts/Utilities/detail/Polynomials.hpp"
namespace MuonCalib {
    double ChebyshevPolynomial::value(const int  order, const double x) const {
      return Acts::detail::chebychevPolyTn(x,order);
    }
}
