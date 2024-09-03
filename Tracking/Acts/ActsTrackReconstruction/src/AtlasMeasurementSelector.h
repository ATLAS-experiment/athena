/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#pragma once
#include "IMeasurementSelector.h"

namespace ActsTrk {
   std::unique_ptr<ActsTrk::IMeasurementSelector>  getMeasurementSelector(const ActsTrk::IOnBoundStateCalibratorTool *onTrackCalibratorTool,
                                                                          const std::vector<float> &etaBinsf,
                                                                          const std::vector<std::pair<float, float> > &chi2CutOffOutlier,
                                                                          const std::vector<size_t> &numMeasurementsCutOff);
}
