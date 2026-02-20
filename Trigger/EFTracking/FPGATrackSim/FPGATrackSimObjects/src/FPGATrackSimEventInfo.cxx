/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include <iostream>


std::ostream& operator<<(std::ostream& s, const FPGATrackSimEventInfo& h) {
  s << "Event " << h.eventNumber()
    << " \tRun " << h.runNumber();

  return s;
}

