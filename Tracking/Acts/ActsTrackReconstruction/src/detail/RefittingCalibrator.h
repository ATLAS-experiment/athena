/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "ActsEvent/TrackContainer.h"

namespace ActsTrk::detail {

class RefittingCalibrator {
 public:
  using MutableTrackStateProxy =
      ActsTrk::MutableTrackStateBackend::TrackStateProxy;
  using ConstTrackStateProxy =
      ActsTrk::MutableTrackStateBackend::ConstTrackStateProxy;

  struct RefittingSourceLink {

    ActsTrk::TrackContainer::ConstTrackStateProxy state;

    RefittingSourceLink() = delete;

    RefittingSourceLink(
        const ActsTrk::TrackContainer::ConstTrackStateProxy& inputState)
        : state(inputState) {}
  };

  static const Acts::Surface* accessSurface(
      const Acts::SourceLink& sourceLink) {
    const auto& refittingSl = sourceLink.get<RefittingSourceLink>();
    return &refittingSl.state.referenceSurface();
  }

  void calibrate(const Acts::GeometryContext& gctx,
                 const Acts::CalibrationContext& cctx,
                 const Acts::SourceLink& sourceLink,
                 MutableTrackStateProxy trackState) const;
};

}  // namespace ActsTrk::detail

#endif  // ACTSTRACKRECONSTRUCTION_REFITTINGCALIBRATOR_H
