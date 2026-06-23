/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELONTRACKCALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IPIXELONTRACKCALIBRATORTOOL_H

#include "IOnTrackCalibratorTool.h"
#include "IPixelOnBoundStateCalibratorTool.h"

namespace ActsTrk {
  template<typename traj_t>
  using PixelOnTrackCalibratorBase = OnTrackCalibratorBase<xAOD::PixelCluster,2, traj_t>;

  template<typename traj_t>
  class IPixelOnTrackCalibratorTool : virtual public IOnTrackCalibratorTool<xAOD::PixelCluster,2, traj_t> {
  public:
      DeclareInterfaceID(IPixelOnTrackCalibratorTool,1,0);
  };
} // namespace ActsTrk

#endif
