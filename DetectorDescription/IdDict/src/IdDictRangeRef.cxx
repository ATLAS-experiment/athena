/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictRangeRef.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictFieldImplementation.h"


IdDictRangeRef::IdDictRangeRef (IdDictRange& range)
  : m_range (range)
{
}


void
IdDictRangeRef::resolve_references(IdDictMgr& idd,
                                   IdDictDictionary& dictionary,
                                   IdDictRegion& region) {
  m_range.resolve_references(idd, dictionary, region);
}

void
IdDictRangeRef::generate_implementation(const IdDictMgr& idd,
                                        IdDictDictionary& dictionary,
                                        IdDictRegion& region,
                                        const std::string& tag) {
  m_range.generate_implementation(idd, dictionary, region, tag);
}

void
IdDictRangeRef::reset_implementation() {
  m_range.reset_implementation();
}

bool
IdDictRangeRef::verify() const {
  return(m_range.verify());
}

Range
IdDictRangeRef::build_range() const {
  return m_range.build_range();
}
