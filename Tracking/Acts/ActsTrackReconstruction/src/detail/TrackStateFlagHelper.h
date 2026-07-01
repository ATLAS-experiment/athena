/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_TRACKSTATEFLAGHELPER_H
#define ACTSTRK_TRACKSTATEFLAGHELPER_H
#include "Acts/EventData/TrackStateType.hpp"

namespace ActsTrk::detail {
   // convenience functions to test and set track state flags
   constexpr unsigned int setTrackStateFlag(Acts::TrackStateFlag flag) {
      return 1u<<static_cast<unsigned int>(flag);
   }
   constexpr bool testTrackStateFlag(Acts::TrackStateFlag flag, unsigned int flags) {
      return flags & setTrackStateFlag(flag);
   }
}
#endif
