/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "IdDict/IdDictSubRegion.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictRegionEntry.h"
#include "src/Debugger.h"
#include <iostream>


 
void 
IdDictSubRegion::generate_implementation (const IdDictMgr& /*idd*/, 
					  IdDictDictionary& /*dictionary*/, 
					  const std::string& /*tag*/){
    std::cout << "IdDictSubRegion::generate_implementation - SHOULD NEVER BE CALLED "  << std::endl;
}


void 
IdDictSubRegion::generate_implementation (const IdDictMgr& idd,  
					  IdDictDictionary& dictionary, 
					  IdDictRegion& region,
					  const std::string& tag) { 

  if (Debugger::debug ()) { 
    std::cout << "IdDictSubRegion::generate_implementation>" << std::endl; 
  } 
  // NOTE: we DO NOT protect this method with
  // m_generated_implementation because a subregion is a "reference"
  // and must be looped over to fully implement a region.
  std::vector<IdDictRegionEntry*>::iterator it;  

  for (it = m_entries.begin (); it != m_entries.end (); ++it) { 
    IdDictRegionEntry* entry = *it;  
    entry->generate_implementation (idd, dictionary, region, tag);  
  } 
} 
 
void 
IdDictSubRegion::reset_implementation (){ 
  for (auto * entry:m_entries) { 
	  entry->reset_implementation ();  
  } 
} 
