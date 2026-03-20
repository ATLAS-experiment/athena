/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EGAMMAANALYSISINTERFACES_GLOBALEVENTINFO_H
#define EGAMMAANALYSISINTERFACES_GLOBALEVENTINFO_H
#include "xAODEventInfo/EventInfo.h"

namespace egammaMVACalib {
  /// A structure holding some global event information
  struct GlobalEventInfo {
    int nPV;
    float acmu;
    const xAOD::EventInfo* eventInfo;
    std::array<float, 4> scaleEs; /// stores correction scale factors to each layer
    GlobalEventInfo(): nPV(0),acmu(0.),eventInfo(nullptr), scaleEs({1.,1.,1.,1.}){}
  };
};

#endif
