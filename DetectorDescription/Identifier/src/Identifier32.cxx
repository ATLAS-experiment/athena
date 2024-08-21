/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/Identifier32.h"

#include <iostream>
#include <format>


std::string Identifier32::getString() const{
  std::string s = std::format("0x{:x}", m_id);
  return s;
}

void Identifier32::show () const{
  //ensure optimisations are maintained for this 
  static_assert(std::is_trivially_destructible<Identifier32>::value);
  static_assert(std::is_trivially_copy_constructible<Identifier32>::value);
  std::cout << *this;
}

std::ostream & operator << (std::ostream &out, const Identifier32 &c){
  out<<std::string(c);
  return out;
}



