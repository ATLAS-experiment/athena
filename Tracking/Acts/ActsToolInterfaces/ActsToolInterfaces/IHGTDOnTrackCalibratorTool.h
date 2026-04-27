/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IHGTDONTRACKCALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IHGTDONTRACKCALIBRATORTOOL_H

#include "IOnTrackCalibratorTool.h"
#include "IHGTDOnBoundStateCalibratorTool.h"

namespace ActsTrk {
  template<typename traj_t>
  using HGTDOnTrackCalibratorBase = OnTrackCalibratorBase<xAOD::HGTDCluster,3, traj_t>;

  template<typename traj_t>
  class IHGTDOnTrackCalibratorTool : virtual public IOnTrackCalibratorTool<xAOD::HGTDCluster,3,traj_t> {
  public:
      DeclareInterfaceID(IHGTDOnTrackCalibratorTool,1,0);
  };
} // namespace ActsTrk

#endif
