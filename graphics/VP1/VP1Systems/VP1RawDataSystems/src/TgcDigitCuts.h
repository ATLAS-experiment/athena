/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1_TGCDIGITCUTS_H
#define VP1_TGCDIGITCUTS_H

#include "MuonDigitContainer/TgcDigit.h"

// Selection policy shared by collection filtering and its unit tests.
struct TgcDigitCuts {
  bool showStrips = true;
  bool showWires = true;
  bool showPrevious = true;
  bool showCurrent = true;
  bool showNext = true;
  bool showNextNext = true;
  bool showUndefined = true;
  bool showUnknown = true;

  bool accepts(bool strip, unsigned bcTag) const
  {
    if (strip ? !showStrips : !showWires)
      return false;
    switch (bcTag) {
    case TgcDigit::BC_UNDEFINED: return showUndefined;
    case TgcDigit::BC_PREVIOUS: return showPrevious;
    case TgcDigit::BC_CURRENT: return showCurrent;
    case TgcDigit::BC_NEXT: return showNext;
    case TgcDigit::BC_NEXTNEXT: return showNextNext;
    default: return showUnknown;
    }
  }
};

#endif
