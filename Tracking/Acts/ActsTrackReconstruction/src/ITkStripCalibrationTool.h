/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ITKSTRIPCALIBRATIONTOOL_H
#define ACTSTRACKRECONSTRUCTION_ITKSTRIPCALIBRATIONTOOL_H

#include "src/detail/StripCalibratorImpl.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {

  class ITkStripCalibrationTool :
    public detail::StripCalibratorImpl<detail::RecoTrackStateContainer> {
  public:
    using traj_t = detail::RecoTrackStateContainer;
    
    using detail::StripCalibratorImpl<traj_t>::StripCalibratorImpl;
  };
  
} // namespace ActsTrk


#endif
