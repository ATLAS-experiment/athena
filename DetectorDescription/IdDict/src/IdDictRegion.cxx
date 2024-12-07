/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictRegionEntry.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "src/Debugger.h"
#include <iostream>

std::string 
IdDictRegion::group_name () const{
    return (m_group);
}

void 
IdDictRegion::set_index (size_t index){
    m_index = index;
}

void 
IdDictRegion::add_entry (IdDictRegionEntry* entry) { 
  m_entries.push_back (entry); 
} 
 
void 
IdDictRegion::resolve_references (const IdDictMgr& idd, IdDictDictionary& dictionary) { 
  std::vector<IdDictRegionEntry*>::iterator it;  
  for (it = m_entries.begin (); it != m_entries.end (); ++it) {  
      IdDictRegionEntry* entry = *it;  
      entry->resolve_references (idd, dictionary, *this);  
    } 
} 
  
void 
IdDictRegion::generate_implementation (const IdDictMgr& idd, 
					    IdDictDictionary& dictionary,
					    const std::string& tag){ 

  if (Debugger::debug ()) { 
      std::cout << "IdDictRegion::generate_implementation>" << std::endl; 
  } 
  if (!m_generated_implementation) {
      std::vector<IdDictRegionEntry*>::iterator it;  
      for (it = m_entries.begin (); it != m_entries.end (); ++it)  {  
        IdDictRegionEntry* entry = *it;  
        entry->generate_implementation (idd, dictionary, *this, tag);  
      } 
      m_generated_implementation = true;
  }
} 
  
void 
IdDictRegion::find_neighbours (const IdDictDictionary& dictionary){
    // Find the neighbours
    IdDictRegion* region = 0;
    if ("" != m_next_abs_eta_name) {
      region = dictionary.find_region(m_next_abs_eta_name,m_group );
      if (region) {
          region->m_prev_abs_eta = this;
          m_next_abs_eta         = region;
      }
    }
    for (unsigned int i = 0; i < m_prev_samp_names.size(); ++i) {
      if ("" != m_prev_samp_names[i]) {
        region = dictionary.find_region(m_prev_samp_names[i],m_group );
        if (region) {
          m_prev_samp.push_back(region);
        }
	    }
    }
    for (unsigned int i = 0; i < m_next_samp_names.size(); ++i) {
      if ("" != m_next_samp_names[i]) {
          region = dictionary.find_region(m_next_samp_names[i],m_group );
          if (region) {
            m_next_samp.push_back(region);
          }
      }
    }

    for (unsigned int i = 0; i < m_prev_subdet_names.size(); ++i) {
      if ("" != m_prev_subdet_names[i]) {
          region = dictionary.find_region(m_prev_subdet_names[i],m_group );
          if (region) {
            m_prev_subdet.push_back(region);
          }
      }
    }
    for (unsigned int i = 0; i < m_next_subdet_names.size(); ++i) {
      if ("" != m_next_subdet_names[i]) {
          region = dictionary.find_region(m_next_subdet_names[i],m_group );
          if (region) {
            m_next_subdet.push_back(region);
          }
          
      }
    }
}


void 
IdDictRegion::reset_implementation () { 
  if (m_generated_implementation) {
      m_implementation.clear();  // remove implementation
      std::vector<IdDictRegionEntry*>::iterator it;  
      for (it = m_entries.begin (); it != m_entries.end (); ++it)  {  
        IdDictRegionEntry* entry = *it;  
        entry->reset_implementation ();  
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
  
bool IdDictRegion::verify () const { 
  return (true); 
} 
 
void 
IdDictRegion::clear () { 
  std::vector<IdDictRegionEntry*>::iterator it;  
  for (it = m_entries.begin (); it != m_entries.end (); ++it){  
      IdDictRegionEntry* entry = *it;  
      entry->clear (); 
      delete entry; 
    }  
  m_entries.clear (); 
} 

size_t 
IdDictRegion::fieldSize() const{
  return m_implementation.size();
}

size_t 
IdDictRegion::size() const{
  return m_entries.size();
}
  
Range 
IdDictRegion::build_range () const {
  Range result; 
  std::vector <IdDictRegionEntry*>::const_iterator it;
  for (it = m_entries.begin (); it != m_entries.end (); ++it){ 
      const IdDictRegionEntry& entry = *(*it); 
      Range r = entry.build_range (); 
      result.add (std::move(r)); 
  } 
  return (result); 
} 

