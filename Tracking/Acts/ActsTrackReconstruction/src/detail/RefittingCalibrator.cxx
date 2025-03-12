/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/RefittingCalibrator.h"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

namespace ActsTrk::detail {

void RefittingCalibrator::calibrate(const Acts::GeometryContext& /*gctx*/,
                                    const Acts::CalibrationContext& /*cctx*/,
                                    const Acts::SourceLink& sourceLink,
                                    MutableTrackStateProxy trackState) const {
  const auto& sl = sourceLink.get<RefittingSourceLink>();

  // Reset the original uncalibrated source link on this track state
  trackState.setUncalibratedSourceLink(sl.state.getUncalibratedSourceLink());

  // Here we construct a measurement by extracting the information available
  // in the state
  Acts::visit_measurement(sl.state.calibratedSize(), [&](auto N) {
    constexpr int Size = decltype(N)::value;

    trackState.allocateCalibrated(
        sl.state.template calibrated<Size>().eval(),
        sl.state.template calibratedCovariance<Size>().eval());
  });

  if (!sl.state.projectorSubspaceIndices().empty()) {
    trackState.setProjectorSubspaceIndices(sl.state.projectorSubspaceIndices());
  }
}

}  // namespace ActsTrk::detail
