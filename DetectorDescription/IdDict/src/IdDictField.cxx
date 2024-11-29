/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "IdDict/IdDictField.h"
#include "IdDict/IdDictLabel.h"
#include <stdexcept>
#include <format>
#include <iostream>


void IdDictField::resolve_references (const IdDictMgr& /*idd*/) 
{ 
} 
 
void IdDictField::generate_implementation (const IdDictMgr& /*idd*/, 
					   const std::string& /*tag*/) 
{ 
} 
 
void IdDictField::reset_implementation () 
{ 
} 
 
bool IdDictField::verify () const 
{ 
  return (true); 
} 
 
IdDictLabel* IdDictField::find_label (const std::string& name) const 
{ 
  for (size_t i = 0; i < m_labels.size (); ++i) 
    { 
      IdDictLabel* label = m_labels[i]; 
      if ((label != 0) && (label->m_name == name)) return (label); 
    } 
 
  return (0); 
} 
 
void 
IdDictField::add_label (IdDictLabel* label) { 
  m_labels.push_back (label); 
} 
  
size_t 
IdDictField::get_label_number () const { 
  return m_labels.size (); 
} 
 
const std::string 
IdDictField::get_label (size_t index) const { 
  std::string result;
  try{
    result = m_labels.at(index)->m_name;
  } catch (std::out_of_range & e){
    throw std::out_of_range(std::format("IdDictField::get_label : Attempt to access index {} in vector of size {}", index, m_labels.size()));
  }
  return result;
} 
 
ExpandedIdentifier::element_type 
IdDictField::get_label_value (const std::string& name) const { 
  ExpandedIdentifier::element_type value = 0;
  try{
    value = std::stoi(name);
    return value; 
  } catch (std::invalid_argument & e){
    for (const auto *label:m_labels) { 
      if (label == nullptr) continue; 
      if (label->m_valued) value = label->m_value; 
      if (label->m_name == name)  { 
        return (value); 
      } 
      value++; 
    } 
  }
  std::cerr << "Warning : label " << name << " not found" << std::endl; 
  return (0); 
} 
 
void IdDictField::clear () { 
  for (size_t i = 0; i < m_labels.size (); ++i) { 
      IdDictLabel* label = m_labels[i]; 
      delete label; 
  } 
  m_labels.clear (); 
} 
