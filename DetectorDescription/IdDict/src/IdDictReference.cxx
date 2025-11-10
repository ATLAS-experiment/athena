/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictReference.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictSubRegion.h"
#include "src/Debugger.h"

#include <iostream>

IdDictReference::IdDictReference (const std::string& subregion_name)
  :
  m_subregion_name(subregion_name),
  m_subregion(nullptr),
  m_resolved_references(false) {
}

IdDictReference::IdDictReference (IdDictSubRegion* subregion)
  :
  m_subregion(subregion),
  m_resolved_references(true) {
}

IdDictReference::~IdDictReference () {
}

void IdDictReference::resolve_references(IdDictMgr& /*idd*/,
                                         IdDictDictionary& dictionary,
                                         IdDictRegion& /*region*/) {
  if (!m_resolved_references) {
    m_subregion = dictionary.find_subregion(m_subregion_name);
    m_resolved_references = true;
  }
}

void IdDictReference::generate_implementation(const IdDictMgr& idd,
                                              IdDictDictionary& dictionary,
                                              IdDictRegion& region,
                                              const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictReference::generate_implementation>" << std::endl;
  }

  if (m_subregion != 0) m_subregion->generate_implementation(idd, dictionary, region, tag);
}

void IdDictReference::reset_implementation() {
  if (m_subregion != 0) m_subregion->reset_implementation();
}

bool IdDictReference::verify() const {
  return(true);
}

Range IdDictReference::build_range() const {
  Range result;

  result = m_subregion->build_range();


  return(result);
}
