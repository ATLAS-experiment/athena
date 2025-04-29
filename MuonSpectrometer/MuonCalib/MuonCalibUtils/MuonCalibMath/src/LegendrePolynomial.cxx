/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibMath/LegendrePolynomial.h"
#include "MuonCalibMath/LegendrePoly.h"
#include "cmath"

using namespace MuonCalib;

double LegendrePolynomial::value(const int  k, const double  x) const {
    return std::legendre(k, x);
}
