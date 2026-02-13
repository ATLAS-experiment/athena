/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/Identifier.h"
#include "GaudiKernel/MsgStream.h"
#include <charconv>
#include <format>


void Identifier::set (const std::string& id){
  const auto start = id.data();
  const auto end = start + id.size();
  static constexpr int base = 16;
  //add 2 to start to get past the Ox prefix.
  const auto [p,ec] = std::from_chars(start+2, end, m_id, base);
  if (ec != std::errc()){
    throw std::runtime_error("Number was not parsed in Identifier::set");
  }
}


std::string 
Identifier::getString() const{
  return std::format("0x{:0x}", m_id);
}

void 
Identifier::show (std::ostream & out) const{
    out << getString();
}

void 
Identifier::show (MsgStream & out) const{
    out << getString();
}

MsgStream&
operator<<(MsgStream& f, const Identifier& id){
  f << id.getString();
  return f;
}

std::ostream&
operator<<(std::ostream& os, const Identifier& id){
  os << id.getString();
  return os;
}



