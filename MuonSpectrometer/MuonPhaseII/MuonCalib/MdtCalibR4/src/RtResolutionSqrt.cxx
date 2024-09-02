/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibR4/RtResolutionSqrt.h"
#include <string>

using namespace MuonCalibR4;

std::string RtResolutionSqrt::name(void) const {return "RtResolutionSqrt";}

double RtResolutionSqrt::resolution(double /*t*/, double /*bgRate*/) const {
    return 0.5;
}
