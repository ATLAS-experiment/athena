/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNHELPERS_UTILS_H
#define MUONPATTERNHELPERS_UTILS_H
#include <CxxUtils/ArrayHelper.h>
#include <array>

namespace MuonR4{
    /** @brief Returns the sign of a number */
    constexpr double sign(const double x) {
        return  x > 0. ? 1. : x < 0. ? -1. : 0.;
    }
}
#endif