/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ITKSTRIPCALIBRATIONTOOL_H
#define ACTSTRACKRECONSTRUCTION_ITKSTRIPCALIBRATIONTOOL_H

#include "src/detail/StripCalibratorToolImpl.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {

  class ITkStripCalibrationTool :
    public detail::StripCalibratorToolImpl<detail::RecoTrackStateContainer> {
  public:
    using traj_t = detail::RecoTrackStateContainer;
    
    using detail::StripCalibratorToolImpl<traj_t>::StripCalibratorToolImpl;
  };
  
} // namespace ActsTrk


#endif
