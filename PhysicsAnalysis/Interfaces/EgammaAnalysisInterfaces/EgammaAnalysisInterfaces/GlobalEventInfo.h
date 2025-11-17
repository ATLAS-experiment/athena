/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EGAMMAANALYSISINTERFACES_GLOBALEVENTINFO_H
#define EGAMMAANALYSISINTERFACES_GLOBALEVENTINFO_H

namespace egammaMVACalib {
  /// A structure holding some global event information
  struct GlobalEventInfo {
    int nPV;
    float acmu;
    GlobalEventInfo(): nPV(0),acmu(0.){}
  };
};

#endif
