/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_ITKANALOGUECLUSTERINGTOOL_H
#define ACTS_ITKANALOGUECLUSTERINGTOOL_H

#include "AnalogueClusteringToolImpl.h"

namespace ActsTrk {

class ITkAnalogueClusteringTool :
	public AnalogueClusteringToolImpl<ITk::PixelOfflineCalibData, ActsTrk::MutableTrackStateBackend> {
public:
    using calib_data_t = ITk::PixelOfflineCalibData;
    using traj_t = ActsTrk::MutableTrackStateBackend;
    using AnalogueClusteringToolImpl<calib_data_t, traj_t>::AnalogueClusteringToolImpl;
};

} // namespace ActsTrk


#endif
