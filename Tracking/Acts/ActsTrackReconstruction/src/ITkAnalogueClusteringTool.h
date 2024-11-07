/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ITKANALOGUECLUSTERINGTOOL_H
#define ACTSTRACKRECONSTRUCTION_ITKANALOGUECLUSTERINGTOOL_H

#include "src/detail/AnalogueClusteringToolImpl.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {

  class ITkAnalogueClusteringTool :
    public detail::AnalogueClusteringToolImpl<ITk::PixelOfflineCalibData, detail::RecoTrackStateContainer> {
  public:
    using calib_data_t = ITk::PixelOfflineCalibData;
    using traj_t = detail::RecoTrackStateContainer;
    
    using detail::AnalogueClusteringToolImpl<calib_data_t, traj_t>::AnalogueClusteringToolImpl;
  };
  
} // namespace ActsTrk


#endif
