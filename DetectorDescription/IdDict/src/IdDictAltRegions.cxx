/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    m_selected_region(0)
{
}

IdDictAltRegions::~IdDictAltRegions()
{
    std::map<std::string, IdDictRegion* >::iterator first = m_regions.begin();
    std::map<std::string, IdDictRegion* >::iterator last  = m_regions.end();
    for (; first != last; ++first) {
	delete (*first).second;
    }
}

std::string 
IdDictAltRegions::group_name () const
{
    std::string result;
    if (1 <= m_regions.size()) result = (*m_regions.begin()).second->group_name();
    return (result);
}

 
void 
IdDictAltRegions::set_index (size_t index)
{
    map_iterator first = m_regions.begin();
    map_iterator last  = m_regions.end();
    for (; first != last; ++first) {
	(*first).second->set_index (index);
    }
}


void 
IdDictAltRegions::resolve_references (const IdDictMgr& idd,  
				      IdDictDictionary& dictionary)
{
    // We assume that it is not necessary to select only those with
    // the correct tag -> send to all in map
    map_iterator first = m_regions.begin();
    map_iterator last  = m_regions.end();
    for (; first != last; ++first) {
	(*first).second->resolve_references (idd, dictionary);
    }
}

void 
IdDictAltRegions::generate_implementation (const IdDictMgr& idd,  
					  IdDictDictionary& dictionary, 
					  const std::string& tag)
{
    // Find the region given by the tag
    map_iterator region_it = m_regions.find(tag);
    if(region_it == m_regions.end()) {
	std::cout << "IdDictAltRegions::generate_implementation could not find region for tag " 
		  << tag << " Keys in map " << std::endl;
	map_iterator first = m_regions.begin();
	map_iterator last  = m_regions.end();
	int i = 0;
	for (; first != last; ++first, ++i) {
	    std::cout << " i " << i << " key " << (*first).first;
	}
	std::cout << std::endl;
	return;
    }
    m_selected_region = (*region_it).second;


    m_selected_region->generate_implementation(idd, dictionary, tag);    

}

void 
IdDictAltRegions::reset_implementation ()
{
    if(m_selected_region)m_selected_region->reset_implementation();
}


bool 
IdDictAltRegions::verify () const
{
    return (true);
}

void 
IdDictAltRegions::clear ()
{
    map_iterator first = m_regions.begin();
    map_iterator last  = m_regions.end();
    for (; first != last; ++first) {
	(*first).second->clear();
	delete (*first).second;
    }
    m_regions.clear();
}
 
Range 
IdDictAltRegions::build_range () const{
    Range result;
    
    if(m_selected_region)result = m_selected_region->build_range();
    return (result);
}