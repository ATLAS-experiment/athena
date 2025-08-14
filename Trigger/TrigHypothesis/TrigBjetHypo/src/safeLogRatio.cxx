/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBjetHypo/safeLogRatio.h"

#include <cmath>

float safeLogRatio(float num, float denom) {
  float ratio = (denom == 0 ? INFINITY : num / denom);
  return ratio == 0 ? -INFINITY : std::log( ratio );
}