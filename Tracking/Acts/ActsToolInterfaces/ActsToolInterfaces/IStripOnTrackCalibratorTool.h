/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPONTRACKCALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPONTRACKCALIBRATORTOOL_H

#include "IOnTrackCalibratorTool.h"
#include "IStripOnBoundStateCalibratorTool.h"

namespace ActsTrk {
  template<typename traj_t>
  using StripOnTrackCalibratorBase = OnTrackCalibratorBase<xAOD::StripCluster,1, traj_t>;

  template<typename traj_t>
  class IStripOnTrackCalibratorTool : virtual public IOnTrackCalibratorTool<xAOD::StripCluster,1,traj_t> {
  public:
      DeclareInterfaceID(IStripOnTrackCalibratorTool,1,0);
  };
} // namespace ActsTrk

#endif
