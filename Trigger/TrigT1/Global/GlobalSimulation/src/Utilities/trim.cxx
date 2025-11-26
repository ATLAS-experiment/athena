//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


#include "trim.h"

namespace GlobalSim {
  std::string trim(std::string s){
    const char* t = " \t\n\r\f\v";
    
    // trim from right
    auto l_rtrim =  [&t](std::string& str){
      str.erase(str.find_last_not_of(t) + 1);
      return str;
    };
    
    // trim from left
    auto l_ltrim = [&t] (std::string& str){
      str.erase(0, str.find_first_not_of(t));
      return str;
    };
    
    auto rs = l_rtrim(s);
    return l_ltrim(rs);
  }
}
