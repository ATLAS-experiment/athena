/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "IdDict/IdDictGroup.h"
#include "src/Debugger.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictAltRegions.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictMgr.h"
#include "Identifier/ExpandedIdentifier.h"
#include "Identifier/MultiRange.h"
#include "Identifier/Range.h"
#include "Identifier/RangeIterator.h"

#include <iostream>
#include <map>


IdDictGroup::IdDictGroup ()
    :
    m_generated_implementation(false)
{
}

IdDictGroup::IdDictGroup (const std::string& name)
    :
    m_name(name),
    m_generated_implementation(false)
{
}

IdDictGroup::~IdDictGroup ()
{
}

const std::string&  IdDictGroup::name()
{
    return (m_name);
}

const std::vector<IdDictDictEntry*>& 
IdDictGroup::entries()
{
    return (m_entries);
}

const std::vector<IdDictRegion*>&    
IdDictGroup::regions()
{
    return (m_regions);
}


MultiRange 
IdDictGroup::build_multirange () const
{
  MultiRange result; 
 
  IdDictDictionary::regions_const_it it; 

  for (it = m_regions.begin (); it != m_regions.end (); ++it) 
    { 
      const IdDictRegion& region = *(*it);

      // skip regions created from parents
      if("dummy" == region.m_name) continue;

      // skip empty regions - may arise from alternate_regions
      // where a tag selects an empty region
      if(region.m_is_empty) continue;

      Range r = region.build_range();
      result.add (std::move(r));

    } 
 
  return (result); 
}

void 
IdDictGroup::add_dictentry (IdDictDictEntry* region)
{ 
    m_entries.push_back (region);
} 

void 
IdDictGroup::resolve_references (const IdDictMgr& idd,  
				 IdDictDictionary& dictionary, 
				 size_t& index)
{
    IdDictDictionary::entries_it it; 
    for (it = m_entries.begin (); it != m_entries.end (); ++it) {  
        (*it)->set_index(index);  
        index++; 
 
        (*it)->resolve_references (idd, dictionary);  
    }  
}

void 
IdDictGroup::generate_implementation (const IdDictMgr& idd,  
				      IdDictDictionary& dictionary, 
				      const std::string& tag)
{
  if (Debugger::debug ()) 
    { 
      std::cout << "IdDictGroup::generate_implementation>" << std::endl; 
    } 

  if (!m_generated_implementation) {
      
      // Loop over entries and fill regions vec with selected region
      // (AltRegions have a selection)
      IdDictDictionary::entries_it it; 
      for (it = m_entries.begin (); it != m_entries.end (); ++it) { 
	  (*it)->generate_implementation (idd, dictionary, tag);  
	  // Get region and save in m_regions
	  IdDictRegion* region = dynamic_cast<IdDictRegion*> (*it);
	  if(region) {
	      m_regions.push_back(region);
	  }
	  else {
	      IdDictAltRegions* altregions = dynamic_cast<IdDictAltRegions*> (*it);
	      if(altregions) {
		  m_regions.push_back(altregions->m_selected_region);
	      }
	  }
      }  

      if (m_regions.size() != m_entries.size()) {
	  std::cout << "IdDictGroup::generate_implementation - mismatch of sizes: regions/entries " 
		    << m_regions.size() << " " << m_entries.size()
		    << std::endl; 
      }
 
      m_generated_implementation = true;
  } 
}

void 
IdDictGroup::reset_implementation ()
{
  if (m_generated_implementation) {
      
      m_regions.clear();
      IdDictDictionary::entries_it it; 
      for (it = m_entries.begin (); it != m_entries.end (); ++it) { 
	  (*it)->reset_implementation();  
      }  
      m_generated_implementation = false;
  } 
}


bool 
IdDictGroup::verify () const
{
    // Should check that all regions have the same number of levels,
    // which is part of the definition of a group
    return (true);
}

/** 
 * 
 *   Sort:
 *
 *      Loop over regions and sort according to their first identifier
 * 
 **/

void IdDictGroup::sort () 
{ 

    std::map< ExpandedIdentifier, IdDictDictEntry* > regions;
      
    IdDictDictionary::regions_it it; 

    for (it = m_regions.begin (); it != m_regions.end (); ++it) { 

	const IdDictRegion& region = *(*it);
	Range range = region.build_range ();
	RangeIterator itr(range);
	auto first = itr.begin();
	auto last = itr.end();
	if (first != last) {
	    regions[*first] = *it;
	} else {
	    std::cout << "IdDictDictionary::sort - WARNING empty region cannot sort "
		      << std::endl; 
	}
    } 
    if (regions.size() == m_regions.size()) {
	// Reorder the regions
	std::map< ExpandedIdentifier, IdDictDictEntry* >::iterator mapIt = regions.begin();
	std::vector<IdDictRegion*>::size_type vecIt = 0; 
	for (; mapIt != regions.end (); ++mapIt, ++vecIt) { 
	    m_entries[vecIt] = (*mapIt).second;
	}
    }
    else {
	std::cout << "IdDictGroup::sort - WARNING region map size is NOT the same as the vector size. Map size "
		  << regions.size() << " vector size " << m_regions.size()
		  << std::endl; 
    }
} 
 
void 
IdDictGroup::clear ()
{
    IdDictDictionary::entries_it it; 
  
    for (it = m_entries.begin (); it != m_entries.end (); ++it) {

	IdDictDictEntry* region = *it;
        region->clear ();  
        delete region; 
    }  
 
    m_entries.clear (); 
}
 
