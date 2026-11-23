/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_TRACCCTRKPARAMCONVERSIONHELPER_H
#define ACTSGPUEVENT_TRACCCTRKPARAMCONVERSIONHELPER_H

#include "ActsGPUEvent/TracccTrkParam.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "Acts/EventData/BoundTrackParameters.hpp"

#include <optional>
#include <span>

namespace ActsPlugins {
  class DetraySurfaceIdMap;
}

namespace ActsTrk::detail {

  /// Convert the track parameters packed on device into a TracccTrkParam to Acts bound track parameters.
  /// The Acts surface of the detray surface the parameters are given on is found with the
  /// Detray -> ACTS surface map built by the ACTS Detray plugin.
  /// Returns std::nullopt for seeds without parameters (degenerate traccc states).
  /// Throws std::domain_error if there is no Acts surface for the detray surface.
  std::optional<Acts::BoundTrackParameters> toActsParameters(
      const TracccTrkParam& seed,
      const ActsPlugins::DetraySurfaceIdMap& surfaces);

  /// Find the xAOD cluster of a traccc measurement, using the maps from traccc measurement index
  /// to pixel and strip cluster index made by the TracccMeasurementConverterAlg.
  /// Returns nullptr if the measurement is in neither map.
  const xAOD::UncalibratedMeasurement* findMeasurement(
      unsigned int measIndex,
      std::span<const unsigned int> pixelMap,
      std::span<const unsigned int> stripMap,
      const xAOD::PixelClusterContainer& pixelClusters,
      const xAOD::StripClusterContainer& stripClusters);

}  // namespace ActsTrk::detail

#endif  // ACTSGPUEVENT_TRACCCTRKPARAMCONVERSIONHELPER_H
