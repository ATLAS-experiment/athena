/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibMath/ChebyshevPolynomial.h"
#include "MuonCalibMath/ChebychevPoly.h"
namespace MuonCalib {
    double ChebyshevPolynomial::value(const int  order, const double x) const {
      return chebyshevPoly1st(order, x);
    }
}
