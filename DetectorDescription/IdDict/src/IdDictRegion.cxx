/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictRegionEntry.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "src/Debugger.h"
#include <iostream>

IdDictRegion::IdDictRegion (const std::string& name,
                            const std::string& group,
                            const std::string& tag)
  : m_name (name),
    m_group (group),
    m_tag (tag)
{
}
IdDictRegion::~IdDictRegion () = default;

IdDictRegion::IdDictRegion (IdDictRegion&&) = default;
IdDictRegion& IdDictRegion::operator= (IdDictRegion&&) = default;


std::string
IdDictRegion::group_name() const {
  return(m_group);
}


size_t IdDictRegion::n_implementation() const
{
    return m_implementation.size();
}


const IdDictFieldImplementation& IdDictRegion::implementation(size_t i) const
{
    return m_implementation.at(i);
}


void
IdDictRegion::set_index(size_t index) {
  m_index = index;
}

/// Add entry to the end of the list.
void
IdDictRegion::add_entry(std::unique_ptr<IdDictRegionEntry> entry) {
  m_entries.push_back(std::move(entry));
}

/// Add entry to the start of the list.
void
IdDictRegion::prepend_entry(std::unique_ptr<IdDictRegionEntry> entry) {
  m_entries.insert(m_entries.begin(), std::move(entry));
}


/// Non-const access to implementation objects.
IdDictFieldImplementation& IdDictRegion::implementation(size_t i)
{
    return m_implementation.at(i);
}


IdDictFieldImplementation& IdDictRegion::new_implementation()
{
  m_implementation.resize (m_implementation.size() + 1);
  return m_implementation.back();
}


/// Set the name for next_abs_eta.
void IdDictRegion::set_next_abs_eta_name (const std::string& name)
{
  m_next_abs_eta_name = name;
}


/// Add a previous sample name.
void IdDictRegion::add_prev_samp_name (const std::string& name)
{
  m_prev_samp_names.push_back (name);

}


/// Add a next sample name.
void IdDictRegion::add_next_samp_name (const std::string& name)
{
  m_next_samp_names.push_back (name);
}


/// Add a previous subdetector name.
void IdDictRegion::add_prev_subdet_name (const std::string& name)
{
  m_prev_subdet_names.push_back (name);
}


/// Add a next subdetector name.
void IdDictRegion::add_next_subdet_name (const std::string& name)
{
  m_next_subdet_names.push_back (name);
}


/// Set eta/phi variables.
void IdDictRegion::set_etaphi (double eta0, double deta,
                               double phi0, double dphi)
{
  m_eta0 = eta0;
  m_deta = deta;
  m_phi0 = phi0;
  m_dphi = dphi;
}


/// Set is_empty flag.
void IdDictRegion::set_is_empty()
{
  m_is_empty = true;
}


void
IdDictRegion::resolve_references(IdDictMgr& idd, IdDictDictionary& dictionary) {
  for (auto& entry : m_entries) {
    entry->resolve_references(idd, dictionary, *this);
  }
}

void
IdDictRegion::generate_implementation(const IdDictMgr& idd,
                                      IdDictDictionary& dictionary,
                                      const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictRegion::generate_implementation>" << std::endl;
  }
  if (!m_generated_implementation) {
    for (auto& entry : m_entries) {
      entry->generate_implementation(idd, dictionary, *this, tag);
    }
    m_generated_implementation = true;
  }
}

void
IdDictRegion::find_neighbours(IdDictDictionary& dictionary) {
  // Find the neighbours
  IdDictRegion* region = 0;

  if ("" != m_next_abs_eta_name) {
    region = dictionary.find_region(m_next_abs_eta_name, m_group);
    if (region) {
      region->m_prev_abs_eta = this;
      m_next_abs_eta = region;
    }
  }
  for (unsigned int i = 0; i < m_prev_samp_names.size(); ++i) {
    if ("" != m_prev_samp_names[i]) {
      region = dictionary.find_region(m_prev_samp_names[i], m_group);
      if (region) {
        m_prev_samp.push_back(region);
      }
    }
  }
  for (unsigned int i = 0; i < m_next_samp_names.size(); ++i) {
    if ("" != m_next_samp_names[i]) {
      region = dictionary.find_region(m_next_samp_names[i], m_group);
      if (region) {
        m_next_samp.push_back(region);
      }
    }
  }

  for (unsigned int i = 0; i < m_prev_subdet_names.size(); ++i) {
    if ("" != m_prev_subdet_names[i]) {
      region = dictionary.find_region(m_prev_subdet_names[i], m_group);
      if (region) {
        m_prev_subdet.push_back(region);
      }
    }
  }
  for (unsigned int i = 0; i < m_next_subdet_names.size(); ++i) {
    if ("" != m_next_subdet_names[i]) {
      region = dictionary.find_region(m_next_subdet_names[i], m_group);
      if (region) {
        m_next_subdet.push_back(region);
      }
    }
  }
}

void
IdDictRegion::reset_implementation() {
  if (m_generated_implementation) {
    m_implementation.clear();  // remove implementation
    for (auto& entry : m_entries) {
      entry->reset_implementation();
    }
    // reset neighbours
    m_prev_abs_eta = 0;
    m_next_abs_eta = 0;
    m_prev_samp.clear();
    m_next_samp.clear();
    m_prev_subdet.clear();
    m_next_subdet.clear();

    m_generated_implementation = false;
  }
}

bool IdDictRegion::verify() const {
  return(true);
}

void
IdDictRegion::clear() {
  m_entries.clear();
}

size_t
IdDictRegion::fieldSize() const {
  return m_implementation.size();
}

size_t
IdDictRegion::size() const {
  return m_entries.size();
}

void
IdDictRegion::integrate_bits() {
  // For each region, loop over its levels and set the bit offset
  // for each FieldImplementation

  size_t bits_offset = 0;
  for (IdDictFieldImplementation& impl : m_implementation) {
    impl.optimize(); // optimize for decoding
    impl.set_bits_offset(bits_offset);
    bits_offset += impl.bits();

    // Set whether or not to decode index
    Range::field field = impl.ored_field();
    if ((not field.isBounded()) || (0 != field.get_minimum())) {
      impl.set_decode_index(true);
    }
  }
}

Range
IdDictRegion::build_range() const {
  Range result;

  for (auto& entry : m_entries) {
    Range r = entry->build_range();
    result.add(std::move(r));
  }
  return(result);
}
