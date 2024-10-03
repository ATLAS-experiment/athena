/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimMatchInfo.h"
#include <iostream>


std::ostream& operator<<(std::ostream& s, const FPGATrackSimMatchInfo& h) {

  s << "barcode: " << h.barcode()
    << ", event index: " << h.evtindex() << std::endl;

  return s;
}
