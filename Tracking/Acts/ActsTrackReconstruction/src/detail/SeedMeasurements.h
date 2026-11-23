/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_SEEDMEASUREMENTS_H
#define ACTSTRACKRECONSTRUCTION_SEEDMEASUREMENTS_H

#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include <boost/container/small_vector.hpp>

namespace ActsTrk::detail {

  /// Measurements attached to one seed, e.g. the measurements of its space points
  using SeedMeasurements = boost::container::small_vector<const xAOD::UncalibratedMeasurement*, 6>;

}  // namespace ActsTrk::detail

#endif
