/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "cmath"
#include "MuonCalibMath/SimplePolynomial.h"

using namespace MuonCalib;

double SimplePolynomial::value(const int k, const double x) const {
	  return std::pow(x, k);
}
