/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_ARGSTRINGTOSIZET_H
#define TRIGHLTJETHYPO_ARGSTRINGTOSIZET_H

#include <string>
#include <vector>
#include <algorithm>
#include <limits>

class ArgStrToSizeT {
 public:

  double operator() (const std::string& s){
    
    static const std::array<std::string,4> posinf{"inf", "+inf", "pinf", "INF"};
    double val{0};
    if(std::find(posinf.begin(), posinf.end(), s) != posinf.end()){
      val =  std::numeric_limits<std::size_t>::max();}
    else {
      val = std::stoull(s);
    }
    
    return val;
  }
};

#endif
