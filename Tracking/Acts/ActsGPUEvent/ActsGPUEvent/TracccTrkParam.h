/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRKPARAM_H
#define ACTSGPUEVENT_TRACCCTRKPARAM_H

// Framework include(s).
#include "AthenaKernel/CLASS_DEF.h"

// Acts/traccc include(s).
#include <traccc/definitions/primitives.hpp>
#include <detray/geometry/identifier.hpp>

// VecMem include(s).
#include <vecmem/containers/data/vector_buffer.hpp>

// System include(s).
#include <array>

namespace ActsTrk {

  /// Everything the host CKF needs to know about one device seed, packed on
  /// device so that it can be copied to the host in one go: the initial track
  /// parameters estimated on device and the traccc measurement indices of the
  /// seed space points. The traccc seeds and space points are not copied, and
  /// of the traccc covariance only the 15 independent entries of the loc0..q/p
  /// block and the time variance are kept.
  struct TracccTrkParam {
    /// Bound parameters loc0, loc1, phi, theta, q/p, time (Acts and detray order)
    std::array<traccc::scalar, 6> parameters;
    /// Lower triangle of the loc0..q/p covariance, row by row:
    /// (0,0), (1,0), (1,1), (2,0), (2,1), (2,2), ...
    std::array<traccc::scalar, 15> covariance;
    /// Time variance. The initial covariance of traccc is diagonal, so the time
    /// has no correlations with the other parameters
    traccc::scalar timeVariance;
    /// Detray surface the parameters are given on
    detray::geometry::identifier surfaceLink;
    /// traccc measurement index of each seed space point, in the order bottom,
    /// middle, top. Only the first measurement of each space point is kept:
    /// this assumes pixel seeds, whose space points are made from a single
    /// measurement. Strip space points have two measurements, so strip seeds
    /// would need the second index (measurement_index_2) as well.
    std::array<unsigned int, 3> measurementIndices {};
    /// False if the estimate failed: all-zero parameters (traccc's degenerate
    /// states), non-finite parameters, or a non-finite or non-positive variance
    bool hasParameters = false;
  };

  /// One record per seed, index-aligned with the traccc seed collection
  using TracccTrkParamBuffer = vecmem::data::vector_buffer<TracccTrkParam>;

}  // namespace ActsTrk

// Declare identifiers for all the types.
CLASS_DEF(ActsTrk::TracccTrkParamBuffer, 40093323, 1)

#endif  // ACTSGPUEVENT_TRACCCTRKPARAM_H
