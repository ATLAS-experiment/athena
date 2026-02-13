/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/ExpandedIdentifier.h"
#include "GaudiKernel/MsgStream.h"
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <charconv>
#include <stdexcept>
#include <ranges>

static std::string 
show_vector (const ExpandedIdentifier::element_vector& v, const std::string & sep="/"){
  if (v.empty()) return {};
  std::string result = std::to_string(v.front());
  for (auto value : v | std::views::drop(1)){
      result+= sep + std::to_string(value);
  }
  return result;
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



ExpandedIdentifier::operator std::string () const{
  return show_vector(m_fields);
}

void 
ExpandedIdentifier::show (std::ostream & out) const{
  out<<  "["<< show_vector (m_fields,".") <<"]";
}

void 
ExpandedIdentifier::show (MsgStream & out) const{
  out<<  "["<< show_vector (m_fields,".") <<"]";
}

std::ostream & operator << (std::ostream &out, const ExpandedIdentifier & x){
  out<<std::string(x);
  return out;
}


