/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibR4/TrSqrt.h"

#include <cmath>
#include <iostream>

using namespace MuonCalibR4;

double TrSqrt::tFromR(const double  r, bool& /*out_of_bound_flag*/) const {
    return r*r;
}

double TrSqrt::rFromT(const double  t, bool& /*out_of_bound_flag*/) const {
    return std::sqrt(t);
}

