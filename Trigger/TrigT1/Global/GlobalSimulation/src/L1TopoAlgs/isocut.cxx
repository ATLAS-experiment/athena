/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "isocut.h"

#include <stdexcept>

namespace GlobalSim {
  bool isocut(const std::string& threshold, const unsigned int bit) {
    if (threshold == "None") {return true;}
    if (threshold == "Loose") {return bit >= 1;}
    if (threshold == "Medium") {return bit >= 2;}
    if (threshold == "HadMedium") {return bit >= 2;}
    if (threshold == "Tight") {return bit>= 3;}

    throw std::runtime_error("GlobalSim isocut() unnown threshold " +
			     threshold);
  }

  
  bool isocut(const unsigned int threshold, const unsigned int bit) {
    if (bit >= threshold) {return true;}
    return false;
  }

}
