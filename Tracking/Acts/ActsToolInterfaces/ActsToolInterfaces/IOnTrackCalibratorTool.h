/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IONTRACKCALIBRATORTOOL_H
#define ACTSTOOLINTERFACES_IONTRACKCALIBRATORTOOL_H

#include <GaudiKernel/IAlgTool.h>

namespace ActsTrk {

template <typename traj_t>
class OnTrackCalibrator;

template <typename traj_t>
class IOnTrackCalibratorTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IOnTrackCalibratorTool, 1, 0);

    virtual void connect(OnTrackCalibrator<traj_t>& calibrator) const = 0;
};

} // namespace ActsTrk

#endif
