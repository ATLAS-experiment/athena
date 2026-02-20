/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_DIGITIZER_H
#define GLOBALSIM_DIGITIZER_H

#include "ap_int.h"
#include <vector>
#include <algorithm>

namespace GlobalSim {

  //Scale factor to digitize cell energies to the hardware scale
  //Current best estimate
  static auto sf = [](double v) {
    if (v < 0) {return 0.;}
    if (v < 8000) {return v / 31.25;}
    if (v < 40000) {return 192. + v / 125;}
    if (v < 168000) {return 432. + v / 500;}
    if (v < 678000) {return 686. + v / 2000;}
    return 1023.;
  };
  
  struct digitizer{      
    //Outupt scaled cell energies to 10bit ints
    static std::vector<ap_int<10>>
    digitize10(const std::vector<double>& v) {
      auto result = std::vector<ap_int<10>>();
      result.reserve(v.size());
      
      std::transform(std::cbegin(v),
		     std::cend(v),
		     std::back_inserter(result),
		     sf);
      
      return result;
    }
    
    //Outupt scaled cell energies to 16bit ints
    static std::vector<ap_int<16>>
    digitize16(const std::vector<double>& v) {
      auto result = std::vector<ap_int<16>>();
      result.reserve(v.size());
      
      std::transform(std::cbegin(v),
		     std::cend(v),
		     std::back_inserter(result),
		     sf);
      
      return result;
    }
  };
}

#endif
