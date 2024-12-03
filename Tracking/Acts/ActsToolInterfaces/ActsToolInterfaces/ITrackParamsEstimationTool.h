/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ITRACKPARAMESTIMATIONTOOL_H
#define ACTSTOOLINTERFACES_ITRACKPARAMESTIMATIONTOOL_H

// Athena 
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "ActsEvent/Seed.h"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"

// Others
#include <functional>

namespace ActsTrk {
  class ITrackParamsEstimationTool
    : virtual public IAlgTool {
  public:
    DeclareInterfaceID(ITrackParamsEstimationTool, 1, 0);
    
    virtual 
      std::optional<Acts::BoundTrackParameters>
      estimateTrackParameters(const EventContext& ctx,
			      const ActsTrk::Seed& seed,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      std::function<const Acts::Surface&(const ActsTrk::Seed&)> retrieveSurface) const = 0;

    virtual
      std::optional<Acts::BoundTrackParameters>
      estimateTrackParameters(const EventContext& ctx,
                              const ActsTrk::Seed& seed,
                              const Acts::GeometryContext& geoContext,
                              const Acts::Surface& surface,
                              const Acts::Vector3& bField) const = 0;
  };
  
} // namespace 

#endif 

