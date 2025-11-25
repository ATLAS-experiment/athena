/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPTRACKER_COLLSET_H
#define FPTRACKER_COLLSET_H

#include "Collimator.h" // for Collimator::Container_t
#include "FPTrackerConstants.h" //for Side

namespace FPTracker{
  class CollimatorData;
  Collimator::Container_t collSet(const CollimatorData&, Side);
}
#endif
