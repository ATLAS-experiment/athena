/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_NNCLUSTERCALIBRATORTOOL_H
#define ACTSTRACKRECONSTRUCTION_NNCLUSTERCALIBRATORTOOL_H

#include "src/detail/NNClusterCalibratorToolImpl.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {
  /***
   * @brief Tool that allows to calibrate pixel clusters using NN
   * 
   * The implementation is actually in: @see NNClusterCalibratorToolImpl
   */
  class ITkNNClusterCalibratorTool :
    public detail::NNClusterCalibratorToolImpl<ITk::PixelOfflineCalibData, detail::RecoTrackStateContainer> {
  public:
    using calib_data_t = ITk::PixelOfflineCalibData;
    using traj_t = detail::RecoTrackStateContainer;
    
    using detail::NNClusterCalibratorToolImpl<calib_data_t, traj_t>::NNClusterCalibratorToolImpl;
  };
  
} // namespace ActsTrk


#endif
