//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "hexTOB2bitsetTOB.h"
#include <algorithm>

namespace GlobalSim {

  std::bitset<72> hexTOB2bitsetTOB(std::string s) {

    auto bit_tob = std::bitset<72>();
    auto bs = std::bitset<4>();

    std::size_t ind{s.size()*4};

    // ensure s is lower case
    std::transform(s.begin(), s.end(), s.begin(),
		   [](unsigned char c){return std::tolower(c);});

    for(const char& c : s) {
      bs = (c >= 'a') ? (c - 'a' + 10) : (c-'0');
      for (int j = 3; j != -1; --j) {
	bit_tob[--ind] = bs[j];
      }
      if (ind == 0) {break;}
    }
    return bit_tob;
  }

}
