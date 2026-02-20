/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictAltRegions.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/Range.h"
#include "IdDict/IdDictFieldImplementation.h"

#include <iostream>

/**
 *
 */
IdDictAltRegions::IdDictAltRegions()
  :
  m_selected_region(0) {
}

IdDictAltRegions::~IdDictAltRegions() = default;

std::string
IdDictAltRegions::group_name() const {
  std::string result;
  if (!m_regions.empty()) result = m_regions.begin()->second->group_name();
  return(result);
}

void
IdDictAltRegions::set_index(size_t index) {
  for (auto& p : m_regions) {
    p.second->set_index(index);
  }
}

void
IdDictAltRegions::resolve_references(IdDictMgr& idd,
                                     IdDictDictionary& dictionary) {
  // We assume that it is not necessary to select only those with
  // the correct tag -> send to all in map
  for (auto& p : m_regions) {
    p.second->resolve_references(idd, dictionary);
  }
}

void
IdDictAltRegions::generate_implementation(const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          const std::string& tag) {
  // Find the region given by the tag
  map_iterator region_it = m_regions.find(tag);

  if (region_it == m_regions.end()) {
    std::cout << "IdDictAltRegions::generate_implementation could not find region for tag "
              << tag << " Keys in map " << std::endl;
    for (int i = 0; const auto& p : m_regions) {
      std::cout << " i " << i++ << " key " << p.first;
    }
    std::cout << std::endl;
    return;
  }
  m_selected_region = region_it->second.get();
  m_selected_region->generate_implementation(idd, dictionary, tag);
}

void
IdDictAltRegions::reset_implementation() {
  if (m_selected_region) m_selected_region->reset_implementation();
}

bool
IdDictAltRegions::verify() const {
  return(true);
}

void
IdDictAltRegions::clear() {
  m_regions.clear();
}

Range
IdDictAltRegions::build_range() const {
  Range result;

  if (m_selected_region) result = m_selected_region->build_range();
  return(result);
}


/// Add a new region, with key given by the tag.
void IdDictAltRegions::add_region (std::unique_ptr<IdDictRegion> region)
{
  m_regions[region->tag()] = std::move(region);
}


/// Select the named region.
void IdDictAltRegions::select_region (const std::string& name)
{
  map_iterator region_it = m_regions.find (name);
  if (region_it == m_regions.end()){
    std::cout << "IdDictAltRegion::select_region could not find region \""
              << name << "\". Keys in map " << std::endl;
    for (const auto& p : m_regions) {
      std::cout << " key " << p.first;
    }
    std::cout << std::endl;
  } else {
    m_selected_region = region_it->second.get();
  }
}
