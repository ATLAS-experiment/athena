/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictRegionEntry.h"

IdDictRegionEntry::IdDictRegionEntry () = default;

IdDictRegionEntry::~IdDictRegionEntry () = default;

void IdDictRegionEntry::resolve_references(IdDictMgr& /*idd*/,
                                           IdDictDictionary& /*dictionary*/,
                                           IdDictRegion& /*region*/) {
}

void IdDictRegionEntry::generate_implementation(const IdDictMgr& /*idd*/,
                                                IdDictDictionary& /*dictionary*/,
                                                IdDictRegion& /*region*/,
                                                std::string_view /*tag*/) {
}

void IdDictRegionEntry::reset_implementation() {
}

bool IdDictRegionEntry::verify() const {
  return(true);
}

void IdDictRegionEntry::clear() {
}
