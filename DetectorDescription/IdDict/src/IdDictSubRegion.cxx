/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */


#include "IdDict/IdDictSubRegion.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictRegionEntry.h"
#include "src/Debugger.h"
#include <iostream>


IdDictSubRegion::IdDictSubRegion (const std::string& name,
                                  const std::string& group,
                                  const std::string& tag)
  : IdDictRegion (name, group, tag)
{
}

IdDictSubRegion::~IdDictSubRegion () = default;

IdDictSubRegion::IdDictSubRegion (IdDictSubRegion&&) = default;
IdDictSubRegion& IdDictSubRegion::operator= (IdDictSubRegion&&) = default;

void
IdDictSubRegion::generate_implementation(const IdDictMgr& /*idd*/,
                                         IdDictDictionary& /*dictionary*/,
                                         const std::string& /*tag*/) {
  std::cout << "IdDictSubRegion::generate_implementation - SHOULD NEVER BE CALLED " << std::endl;
}

void
IdDictSubRegion::generate_implementation(const IdDictMgr& idd,
                                         IdDictDictionary& dictionary,
                                         IdDictRegion& region,
                                         const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictSubRegion::generate_implementation>" << std::endl;
  }

  // NOTE: we DO NOT protect this method with
  // m_generated_implementation because a subregion is a "reference"
  // and must be looped over to fully implement a region.

  for (auto& entry : m_entries) {
    entry->generate_implementation(idd, dictionary, region, tag);
  }
}

void
IdDictSubRegion::reset_implementation() {
  for (auto& entry : m_entries) {
    entry->reset_implementation();
  }
}
