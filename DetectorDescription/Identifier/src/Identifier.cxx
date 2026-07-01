/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/Identifier.h"
#include "GaudiKernel/MsgStream.h"
#include <charconv>
#include <iostream>


template <typename Stream>
void streamIdentifier(Stream& os, unsigned long long id) {
      //This is faster than std::format and uses no allocations.
      //Please don't switch to std::format without performance checks
      char buf[32];
      buf[0] = '0';
      buf[1] = 'x';
      auto [ptr, ec] = std::to_chars(buf + 2, buf + sizeof(buf), id, 16);
      
      os << std::string_view(buf, ptr);
}


void Identifier::set (std::string_view id){
  const auto start = id.data();
  const auto end = start + id.size();
  static constexpr int base = 16;
  //add 2 to start to get past the Ox prefix.
  const auto [p,ec] = std::from_chars(start+2, end, m_id, base);
  if (ec != std::errc()){
    throw std::runtime_error("Number was not parsed in Identifier::set");
  }
}


std::string Identifier::getString() const {
    //This is faster than std::format when checked
    //Please don't switch to std::format without performance checks
    char buf[32];
    buf[0] = '0';
    buf[1] = 'x';
    auto [ptr, ec] = std::to_chars(buf + 2, buf + sizeof(buf), m_id, 16);
    return std::string(buf, ptr);
}

void Identifier::show(std::ostream& out) const {
    streamIdentifier(out, m_id);
}

void Identifier::show(MsgStream& out) const {
    streamIdentifier(out, m_id);
}

MsgStream& operator<<(MsgStream& f, const Identifier& id) {
    streamIdentifier(f, id.get_compact());
    return f;
}

std::ostream& operator<<(std::ostream& os, const Identifier& id) {
    streamIdentifier(os, id.get_compact());
    return os;
}

