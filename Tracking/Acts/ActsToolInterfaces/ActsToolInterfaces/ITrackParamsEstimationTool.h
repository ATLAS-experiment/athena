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

    virtual 
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      bool reverseSearch,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      const Acts::CalibrationContext& calContext,
			      std::function<const Acts::Surface&(const ActsTrk::Seed& seed, bool useTopSp)> retrieveSurface) const = 0;

    virtual
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
            const ActsTrk::Seed& seed,
            bool reverseSearch,
            const Acts::GeometryContext& geoContext,
            const Acts::MagneticFieldContext& magFieldContext,
            const Acts::CalibrationContext& calContext,
            const Acts::Surface& surface,
            const Acts::Vector3& bField) const = 0;

    using SpacePointIndicesFun_t = std::function<std::array<std::size_t, 3>(std::size_t)>;
    virtual SpacePointIndicesFun_t spacePointIndicesFun() const = 0;
    virtual bool estimateFromTopSp(bool reverseSearch) const = 0;

  };
  
} // namespace 

#endif 

