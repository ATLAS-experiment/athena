/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PdgConditional.h"
#include <cmath>//for std::abs


namespace DerivationFramework {
  PdgConditional::PdgConditional():m_condition(0){
    //nop
  }
  PdgConditional::PdgConditional(int equalInt):m_condition(equalInt){
    //nop
  }
  PdgConditional::PdgConditional(unsigned equalInt):m_condition(equalInt){
    //nop
  }
  
  PdgConditional::PdgConditional(std::function<bool(int)> b):m_condition(b){
    //nop
  }
  
  bool 
  PdgConditional::operator==(int v) const{
    //compare with the held integer
    if (std::holds_alternative<int>(m_condition)){
      auto rawValue = std::get<0>(m_condition);
      return v == rawValue;
    }
    //use abs(value) if the held integer is unsigned
    if (std::holds_alternative<unsigned>(m_condition)){
      auto rawValue = std::get<1>(m_condition);
      return unsigned(std::abs(v)) == rawValue;
    }
    //otherwise use user-defined binary predicate
    return std::get<2>(m_condition)(v);
  }
  
}
