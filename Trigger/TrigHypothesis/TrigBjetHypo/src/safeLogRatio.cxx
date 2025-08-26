/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBjetHypo/safeLogRatio.h"

#include <cmath>
#include <limits>

float safeLogRatio(float num, float denom) {
  // ep(silon) is the smallest non-subnormal number
  // the recyprocal of this should not overflow
  float ep = std::numeric_limits<float>::min();
  float ratio = (std::abs(denom) < ep ? INFINITY : num / denom);
  return std::abs(ratio) < ep ? -INFINITY : std::log( ratio );
}
