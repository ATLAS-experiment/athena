/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ITRACKPARAMESTIMATIONTOOL_H
#define ACTSTOOLINTERFACES_ITRACKPARAMESTIMATIONTOOL_H

// Athena 
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "ActsEvent/SeedContainer.h"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/BoundTrackParameters.hpp"

// Others
#include <array>
#include <functional>

namespace ActsTrk {
  class ITrackParamsEstimationTool
    : virtual public IAlgTool {
  public:
    DeclareInterfaceID(ITrackParamsEstimationTool, 1, 0);

    enum EstimationStatus
    {
      kNoSeedRefit,
      kSeedRefitFailed,
      kSeedRefitSuccess
    };

    /// Surface of the measurement measIndex of a space point
    using RetrieveSurfaceFun_t = std::function<const Acts::Surface&(const xAOD::SpacePoint& sp, std::size_t measIndex)>;

    /// Estimate the track parameters at the seed space point spIndex
    virtual
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      std::size_t spIndex,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      const Acts::CalibrationContext& calContext,
			      RetrieveSurfaceFun_t retrieveSurface) const = 0;

    /// Same, with the surface of the estimate and the B-field given
    virtual
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
            const ActsTrk::Seed& seed,
            std::size_t spIndex,
            const Acts::GeometryContext& geoContext,
            const Acts::MagneticFieldContext& magFieldContext,
            const Acts::CalibrationContext& calContext,
            const Acts::Surface& surface,
            const Acts::Vector3& bField) const = 0;

    /// Seed indices of the 3 space points of the estimate at space point spIndex
    using SpacePointIndicesFun_t = std::function<std::array<std::size_t, 3>(const ActsTrk::SpacePointRange&, std::size_t spIndex)>;
    virtual SpacePointIndicesFun_t spacePointIndicesFun() const = 0;

  };
  
} // namespace 

#endif 

