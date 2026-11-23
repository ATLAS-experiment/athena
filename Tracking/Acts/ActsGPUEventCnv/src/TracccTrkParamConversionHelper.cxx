/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccTrkParamConversionHelper.h"

#include "ActsPlugins/Detray/DetraySurfaceIdMap.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

namespace ActsTrk::detail {

  std::optional<Acts::BoundTrackParameters> toActsParameters(
      const TracccTrkParam& seed,
      const ActsPlugins::DetraySurfaceIdMap& surfaces) {
    if (!seed.hasParameters) {
      return std::nullopt;
    }

    const Acts::Surface* surface = surfaces.surface(seed.surfaceLink);
    if (surface == nullptr) {
      std::stringstream msg;
      msg << "No Acts surface known for Detray surface " << seed.surfaceLink;
      throw std::domain_error(msg.str());
    }

    // Same parameter order in detray and Acts: loc0, loc1, phi, theta, q/p, time
    Acts::BoundVector params;
    for (unsigned int i = 0; i < seed.parameters.size(); ++i) {
      params[i] = seed.parameters[i];
    }

    // No correlations with the time: the initial traccc covariance is diagonal
    Acts::BoundMatrix cov = Acts::BoundMatrix::Zero();
    cov(5, 5) = seed.timeVariance;
    unsigned int k = 0;
    for (unsigned int i = 0; i < 5u; ++i) {
      for (unsigned int j = 0; j <= i; ++j) {
        cov(i, j) = seed.covariance[k];
        cov(j, i) = seed.covariance[k];
        ++k;
      }
    }

    return Acts::BoundTrackParameters(surface->getSharedPtr(), params, cov,
                                      Acts::ParticleHypothesis::pion());
  }

  const xAOD::UncalibratedMeasurement* findMeasurement(
      unsigned int measIndex,
      std::span<const unsigned int> pixelMap,
      std::span<const unsigned int> stripMap,
      const xAOD::PixelClusterContainer& pixelClusters,
      const xAOD::StripClusterContainer& stripClusters) {
    constexpr unsigned int invalid = std::numeric_limits<unsigned int>::max();

    const unsigned int pixelIndex = measIndex < pixelMap.size() ? pixelMap[measIndex] : invalid;
    if (pixelIndex != invalid) {
      return pixelClusters.at(pixelIndex);
    }

    const unsigned int stripIndex = measIndex < stripMap.size() ? stripMap[measIndex] : invalid;
    if (stripIndex != invalid) {
      return stripClusters.at(stripIndex);
    }

    return nullptr;
  }

}  // namespace ActsTrk::detail
