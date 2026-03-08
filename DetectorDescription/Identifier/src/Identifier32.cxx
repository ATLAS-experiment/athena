/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/Identifier32.h"

#include "GaudiKernel/MsgStream.h"
#include <format>


std::string Identifier32::getString() const{
  std::string s = std::format("0x{:x}", m_id);
  return s;
}

void 
Identifier32::show (std::ostream & out) const{
  out << *this;
}

void 
Identifier32::show (MsgStream & out) const{
  out << getString();
}


std::ostream & 
operator << (std::ostream &out, const Identifier32 &c){
  out<<std::string(c);
  return out;
}



