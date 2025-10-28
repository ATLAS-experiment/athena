//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


#include "trim.h"

namespace GlobalSim {
  std::string trim(std::string s){
    const char* t = " \t\n\r\f\v";
    
    // trim from right
    auto l_rtrim =  [&t](std::string& s){
      s.erase(s.find_last_not_of(t) + 1);
      return s;
    };
    
    // trim from left
    auto l_ltrim = [&t] (std::string& s){
      s.erase(0, s.find_first_not_of(t));
      return s;
    };
    
    auto rs = l_rtrim(s);
    return l_ltrim(rs);
  }
}
