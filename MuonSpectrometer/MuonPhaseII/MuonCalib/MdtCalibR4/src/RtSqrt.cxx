/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/


#include "MdtCalibR4/RtSqrt.h"

#include <string>
#include <cmath>
#include <iostream>

using namespace MuonCalibR4;

std::string RtSqrt::name(void) const {
    return "RtSqrt";
}

double RtSqrt::radius(double t) const {
    if(t < 0) {
        return std::sqrt(std::abs(t));
    }
    return std::sqrt(t);
}

double RtSqrt::drdt(double t) const {
    return 0.5 / std::sqrt(std::abs(t));
}

double RtSqrt::driftvelocity(double t) const {
    return 0.5 / std::sqrt(t);
}

double RtSqrt::tLower(void) const {
    return parameters()[0];
}

double RtSqrt::tUpper(void) const {
    return parameters()[1];
}

