/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/MultiRange.h"
#include <iostream>
#include <algorithm> //remove_if
#include <ranges>
#include <numeric>

MultiRange::MultiRange (const Range& r, const Range& s) { 
  m_ranges.push_back (r); 
  m_ranges.push_back (s); 
} 

//----------------------------------------------- 
void MultiRange::clear () { 
  m_ranges.clear (); 
} 
 
//----------------------------------------------- 
void MultiRange::add (const Range& range) { 
  // Add new range ONLY if an equivalent does NOT exist
  if (std::ranges::find(m_ranges, range) == m_ranges.end()) {
    m_ranges.push_back(range);
  }
} 
 
//----------------------------------------------- 
void MultiRange::add (Range&& range) { 
  // Add new range ONLY if an equivalent does NOT exist
  if (std::ranges::find(m_ranges, range) == m_ranges.end()) {
    m_ranges.emplace_back (std::move(range));
  }
} 
 
//----------------------------------------------- 
void MultiRange::add (const ExpandedIdentifier& id) { 
  m_ranges.emplace_back (id); 
} 
 
//----------------------------------------------- 
void MultiRange::remove_range (const ExpandedIdentifier& id) {
  // Remove all ranges for which id matches
  [[maybe_unused]] auto erased = std::erase_if (m_ranges,
    [&id] (const Range& r) { return r.match(id); });
}


 
//----------------------------------------------- 
Range& MultiRange::back() { 
  return (m_ranges.back()); 
} 
 
//----------------------------------------------- 
int MultiRange::match(const ExpandedIdentifier& id) const { 
  bool result = std::ranges::any_of(m_ranges, [&id](const Range & r){return r.match(id);});
  return result ? 1:0;
} 
 
//----------------------------------------------- 
const Range& MultiRange::operator [] (MultiRange::size_type index) const { 
  static const Range null_range; 
  if (index >= m_ranges.size ()) return (null_range); 
  return (m_ranges[index]); 
} 
 
//----------------------------------------------- 
MultiRange::size_type MultiRange::size() const { 
  return (m_ranges.size ()); 
} 
 
MultiRange::const_iterator MultiRange::begin() const { 
  return (m_ranges.begin ()); 
} 
 
MultiRange::const_iterator MultiRange::end() const { 
  return (m_ranges.end ()); 
} 
 
MultiRange::size_type MultiRange::cardinality() const {
  size_type init = 0;
  return std::accumulate(m_ranges.begin(), m_ranges.end(), init, 
    [](size_type a, const Range& b){return a + b.cardinality();});
} 
 
MultiRange::size_type MultiRange::cardinalityUpTo(const ExpandedIdentifier& id) const{
  // Loop over ranges in MultiRange and calculate hash for each
  // range
  size_type init = 0;
  return std::accumulate(m_ranges.begin(), m_ranges.end(), init, 
    [&id](size_type a, const Range& b){return a + b.cardinalityUpTo(id);});
}


//----------------------------------------------- 
bool MultiRange::has_overlap() const { 
  range_vector::size_type i; 
  range_vector::size_type j; 
  for (i = 0; i < m_ranges.size (); ++i) { 
    const Range& r = m_ranges[i]; 
    for (j = i + 1; j < m_ranges.size (); ++j) { 
      const Range& s = m_ranges[j]; 
      if (r.overlaps_with (s)) return (true); 
    } 
  } 
  return (false); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory MultiRange::factory_begin() { 
  const MultiRange& me = *this; 
  return (identifier_factory (me)); 
} 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory MultiRange::factory_begin() const { 
  const MultiRange& me = *this; 
  return (const_identifier_factory (me)); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory MultiRange::factory_end() { 
  static const identifier_factory factory(*this);
  return (factory); 
} 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory MultiRange::factory_end() const { 
  static const const_identifier_factory factory; 
  return (factory); 
} 

//----------------------------------------------- 
MultiRange::identifier_factory::identifier_factory(const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end()){ 
    if (m_range_it == m_range_end)return;  // no ranges
    /** 
     *  Set up iterators over ranges and ids.
     */
    if (m_range_it != m_range_end) {
      m_id_fac_it  = ConstRangeIterator(*m_range_it).begin();
      m_id_fac_end = ConstRangeIterator(*m_range_it).end();
      if(m_id_fac_it != m_id_fac_end) {
        // Set id
        m_id = *m_id_fac_it;
      }
    }
}

//----------------------------------------------- 
void MultiRange::identifier_factory::operator ++() { 
    if (m_id.fields () == 0) return; 
    m_id.clear();
    if (m_range_it != m_range_end) {
      if (m_id_fac_it != m_id_fac_end) {
        ++m_id_fac_it;
      }
      if (m_id_fac_it == m_id_fac_end) {
        ++m_range_it;
        if (m_range_it != m_range_end) {
          m_id_fac_it  = ConstRangeIterator(*m_range_it).begin();
          m_id_fac_end = ConstRangeIterator(*m_range_it).end();
        }
      }
      if (m_id_fac_it != m_id_fac_end) {
        m_id = *m_id_fac_it;
      }
    }
} 

//----------------------------------------------- 
const ExpandedIdentifier& MultiRange::identifier_factory::operator *() const { 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool MultiRange::identifier_factory::operator == (const identifier_factory& other) const { 
  return (m_id == other.m_id); 
} 

//----------------------------------------------- 
MultiRange::const_identifier_factory::const_identifier_factory (const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end()){ 

    if (m_range_it == m_range_end) return;  // no ranges
    /** 
     *  Set up iterators over ranges and ids.
     */
    if (m_range_it != m_range_end) {
      m_id_fac_it  = ConstRangeIterator(*m_range_it).begin();
      m_id_fac_end = ConstRangeIterator(*m_range_it).end();
      if(m_id_fac_it != m_id_fac_end) {
        // Set id
        m_id = *m_id_fac_it;
      }
    }
} 

//----------------------------------------------- 
void MultiRange::const_identifier_factory::operator ++ () { 
    if (m_id.fields () == 0) return; 
    m_id.clear();
    if (m_range_it != m_range_end) {
      if (m_id_fac_it != m_id_fac_end) {
        ++m_id_fac_it;
      }
      if (m_id_fac_it == m_id_fac_end) {
        ++m_range_it;
        if (m_range_it != m_range_end) {
          m_id_fac_it  = ConstRangeIterator(*m_range_it).begin();
          m_id_fac_end = ConstRangeIterator(*m_range_it).end();
        }
      }
      if (m_id_fac_it != m_id_fac_end) {
        m_id = *m_id_fac_it;
      }
    }
} 
 
//----------------------------------------------- 
const ExpandedIdentifier& MultiRange::const_identifier_factory::operator * () const { 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool MultiRange::const_identifier_factory::operator == (const const_identifier_factory& other) const { 
  return (m_id == other.m_id);
} 
 
//----------------------------------------------- 
void MultiRange::show () const {
  show (std::cout);
}

void MultiRange::show (std::ostream& s) const { 
  range_vector::size_type i; 
  for (i = 0; i < m_ranges.size (); ++i) { 
    if (i > 0) s << std::endl; 
    const Range& r = m_ranges[i]; 
    r.show (s); 
  } 
} 
 
//----------------------------------------------- 
MultiRange::operator std::string () const { 
  std::string result; 
  range_vector::size_type i{}; 
  for ( ; i < m_ranges.size (); ++i){ 
    if (i > 0) result += " | "; 
    const Range& r = m_ranges[i]; 
    result += (std::string) r; 
  } 
  return (result); 
} 
