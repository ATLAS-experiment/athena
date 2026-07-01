/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/ExpandedIdentifier.h"
#include "GaudiKernel/MsgStream.h"
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <charconv>
#include <stdexcept>
#include <ranges>

template <typename Stream, typename CHAR>
static void stream_vector(Stream& out, const ExpandedIdentifier::element_vector& v, CHAR sep) {
    if (v.empty()) return;
    
    out << v.front();
    for (auto value : v | std::views::drop(1)) {
        out << sep << value;
    }
}

ExpandedIdentifier::ExpandedIdentifier (const std::string& text){
  set (text);
}


void 
ExpandedIdentifier::set (const std::string& text){
  clear ();
  if (text.empty()) return;
  const char *start = text.c_str();
  const char *last = start+text.size();
  static constexpr auto ok=std::errc{};
  int v{};
  bool foundNumber{};
  for (const char * p=start;p<last;++p){
    auto [ptr,ec] = std::from_chars(p, last,v);
    p=ptr;
    if (ec !=  ok) continue;
    add ((element_type) v);
    foundNumber = true;
  }
  if (not foundNumber){
    const std::string msg = "ExpandedIdentifier::set: '"+ text + "' is not a valid input string.";
    throw std::invalid_argument(msg);
  }
}



ExpandedIdentifier::operator std::string () const {
  // If a string is explicitly requested, we use stringstream to generate it
  std::ostringstream oss;
  stream_vector(oss, m_fields, '/');
  return oss.str();
}

void 
ExpandedIdentifier::show (std::ostream & out) const {
  out << '[';
  stream_vector(out, m_fields, '.');
  out << ']';
}

void 
ExpandedIdentifier::show (MsgStream & out) const {
  out << '[';
  stream_vector(out, m_fields, '.');
  out << ']';
}

std::ostream & operator << (std::ostream &out, const ExpandedIdentifier& id) {
  stream_vector(out, id.m_fields, '/');
  return out;
}