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
  bool first = true;
  std::cout << "[";
  for (it = v.begin (); it != v.end (); ++it){
      if (first) first = false;
      else std::cout << ".";
      ExpandedIdentifier::element_type value = *it;
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


bool 
ExpandedIdentifier::prefix_less (const ExpandedIdentifier& other) const{
  const ExpandedIdentifier& me = *this;
  const size_type my_fields = fields ();
  const size_type other_fields = other.fields ();
  // Scan fields up to the less common field number
  size_type field = 0;
  while ((field < my_fields) && (field < other_fields)){
    element_type my_field = me[field];
    element_type other_field = other[field];
    if (my_field < other_field) return true;
    if (my_field > other_field) return false;
    field++;
  }
  return false;
}

ExpandedIdentifier::operator std::string () const{
  std::string result;
  char temp[20];
  size_type my_fields = m_fields.size ();
  if (my_fields == 0) return (result);
  // print fields one by one.
  for (size_type field_number = 0; field_number < my_fields; field_number++){
    element_type value = m_fields[field_number];
    if (field_number > 0) result += "/";
    sprintf (temp, "%d", value);
    result += temp;
  }
  return (result);
}

void 
ExpandedIdentifier::show () const{
  show_vector (m_fields);
}


