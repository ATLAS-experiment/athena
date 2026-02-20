/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictField.h"
#include "IdDict/IdDictLabel.h"
#include <stdexcept>
#include <format>
#include <iostream>


IdDictField::IdDictField (const std::string& name)
  : m_name(name)
{
}

// Define these out-of-line so header doesn't need to include IdDictLabel.h.
IdDictField::IdDictField(IdDictField&&) = default;
IdDictField::~IdDictField() = default;
IdDictField& IdDictField::operator= (IdDictField&&) = default;



bool IdDictField::verify() const {
  return(true);
}

const IdDictLabel* IdDictField::find_label(const std::string& name) const {
  for (const auto& p : m_labels) {
    if (p && p->name() == name) return p.get();
  }
  return nullptr;
}

void
IdDictField::add_label(std::unique_ptr<const IdDictLabel> label) {
  m_labels.push_back(std::move(label));
}

void
IdDictField::set_index (size_t index){
  m_index = index;
}

size_t
IdDictField::get_label_number() const {
  return m_labels.size();
}

const std::string&
IdDictField::get_label(size_t index) const {
  try{
    return m_labels.at(index)->name();
  } catch (std::out_of_range& e) {
    throw std::out_of_range(std::format("IdDictField::get_label : Attempt to access index {} in vector of size {}",
                                        index, m_labels.size()));
  }
}

ExpandedIdentifier::element_type
IdDictField::get_label_value(const std::string& name) const {
  ExpandedIdentifier::element_type value{0};
  if (std::ranges::find_if(name,[](const char c){ return !std::isdigit(c); }) != name.end()) {
    for (const auto& label: m_labels) {
      if (!label) continue;
      if (label->valued()) value = label->value();
      if (label->name() == name) {
        return(value);
      }
      value++;
    }
  }
  try{
    value = std::stoi(name);
    return value;
  } catch (const std::invalid_argument& e) {
    std::cerr << "Warning : label " << name << " not found: "<<e.what() << std::endl;
  }
 
  return(0);
}

void IdDictField::clear() {
  m_labels.clear();
}
