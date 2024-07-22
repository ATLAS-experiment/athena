/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
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
    
    double val{0};
    if(std::find(m_posinf.begin(), m_posinf.end(), s) != m_posinf.end()){
      val =  std::numeric_limits<std::size_t>::max();}
    else {
      val = std::stoull(s);
    }
    
    return val;
  }

  
 private:
  static constexpr std::array<std::string,4> m_posinf{"inf", "+inf", "pinf", "INF"};

};

#endif
