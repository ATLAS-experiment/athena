/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/ExpandedIdentifier.h"
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <charconv>
#include <stdexcept>

static void 
show_vector (const ExpandedIdentifier::element_vector& v){
  ExpandedIdentifier::element_vector::const_iterator it;
  std::cout << "[";
  for (bool first{true}; auto value:v){
      std::cout << (first?"":".");
      first = false;
      std::cout << value;
    }
  std::cout << "]";
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
  // print fields one by one.
  std::string result;
  for (bool first{true}; auto value :m_fields){
    result += (first? "":"/") + std::to_string(value);
    first = false;
  }
  return result;
}

void 
ExpandedIdentifier::show () const{
  show_vector (m_fields);
}

std::ostream & operator << (std::ostream &out, const ExpandedIdentifier & x){
  out<<std::string(x);
  return out;
}


