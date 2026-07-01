/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ATLASMEASUREMENTSELECTOR_H
#define ACTSTRACKRECONSTRUCTION_ATLASMEASUREMENTSELECTOR_H

#include "src/IMeasurementSelector.h"
#include <vector>
#include <utility>
#include <memory>

#include "ActsToolInterfaces/IPixelOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IStripOnTrackCalibratorTool.h"
#include "ActsToolInterfaces/IHGTDOnTrackCalibratorTool.h"
#include "src/detail/AtlasUncalibSourceLinkAccessor.h"
#include "Definitions.h"

#include <vector>
#include <utility>

namespace ActsTrk::detail {
   class IOnBoundStateCalibratorTool;
   std::unique_ptr<ActsTrk::IMeasurementSelector>  getMeasurementSelector(
           const EventContext &ctx,
           const ActsTrk::IPixelOnTrackCalibratorTool<detail::RecoTrackStateContainer> *pixelOnTrackCalibratorTool,
           const ActsTrk::IStripOnTrackCalibratorTool<detail::RecoTrackStateContainer> *stripOnTrackCalibratorTool,
           const ActsTrk::IHGTDOnTrackCalibratorTool<detail::RecoTrackStateContainer> *hgtdOnTrackCalibratorTool,
           const ActsTrk::detail::MeasurementRangeList &measurementRanges,
           const std::vector<float> &etaBinsf,
           const std::vector<std::pair<float, float> > &chi2CutOffOutlier,
           const std::vector<size_t> &numMeasurementsCutOff,
           double edge_hole_border_width);

}

#endif
