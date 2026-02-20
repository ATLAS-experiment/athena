
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "hexStrToBinStr.h"
#include <string>
#include <stdexcept>

namespace GlobalSim {
  
  std::string hexStrToBinStr(std::string s) {
    if (s[0] == '0' and std::tolower(s[1]) == 'x' and s.size()>2) {
      s.assign(std::begin(s)+2, std::end(s));
    }
    std::string alpha{"abcdef"};
    std::string result;
    for (auto  c : s){
      c = std::tolower(c);
      int num;
      if (std::isdigit(c)) {
	num = int(c) - int('0');
      } else if (alpha.find(c) != std::string::npos){
	num = int(c) - int('a') + 10;
      } else {
	throw std::out_of_range("not hex character");
      }
     
      short mask{8};
      for (int j = 3; j != -1; --j) {
	result +=  (mask & num) ?  '1' : '0';
	mask = mask >> 1;
      }
    }
  
    return result;
  }
}
