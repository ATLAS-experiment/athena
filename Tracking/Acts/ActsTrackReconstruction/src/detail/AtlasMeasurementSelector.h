/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ATLASMEASUREMENTSELECTOR_H
#define ACTSTRACKRECONSTRUCTION_ATLASMEASUREMENTSELECTOR_H

#include "src/IMeasurementSelector.h"
#include <vector>
#include <utility>
#include <memory>

#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "src/detail/AtlasUncalibSourceLinkAccessor.h"

#include <vector>
#include <utility>

namespace ActsTrk::detail {
   class IOnBoundStateCalibratorTool;
   std::unique_ptr<ActsTrk::IMeasurementSelector>  getMeasurementSelector(const ActsTrk::IOnBoundStateCalibratorTool *pixelOnTrackCalibratorTool,
                                                                          const ActsTrk::IOnBoundStateCalibratorTool *stripOnTrackCalibratorTool,
                                                                          const ActsTrk::IOnBoundStateCalibratorTool *hgtdOnTrackCalibratorTool,
                                                                          const ActsTrk::detail::MeasurementRangeList &measurementRanges,
                                                                          const std::vector<float> &etaBinsf,
                                                                          const std::vector<std::pair<float, float> > &chi2CutOffOutlier,
                                                                          const std::vector<size_t> &numMeasurementsCutOff);

}

#endif
