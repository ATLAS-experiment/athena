/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1_RPCDIGITCUTS_H
#define VP1_RPCDIGITCUTS_H

#include "VP1Base/VP1Interval.h"
#include <cmath>

struct RpcDigitCuts {
  bool showEta = true;
  bool showPhi = true;
  bool showPrimarySide = true;
  bool showOppositeSide = true;
  bool keepMissingToT = true;
  VP1Interval timeRange{-VP1Interval::inf(), VP1Interval::inf()};
  VP1Interval totRange{-VP1Interval::inf(), VP1Interval::inf()};

  // Times are in ns; a negative ToT denotes an absent measurement.
  bool accepts(bool phi, double time, double tot, bool oppositeSide) const
  {
    if ((phi ? !showPhi : !showEta) ||
        (oppositeSide ? !showOppositeSide : !showPrimarySide))
      return false;
    if (!std::isfinite(time) || !std::isfinite(tot) ||
        timeRange.isEmpty() || totRange.isEmpty() || !timeRange.contains(time))
      return false;
    return tot < 0 ? keepMissingToT : totRange.contains(tot);
  }
};

#endif
