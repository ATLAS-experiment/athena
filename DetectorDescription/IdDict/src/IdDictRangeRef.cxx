/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "IdDict/IdDictRangeRef.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictFieldImplementation.h"

void 
IdDictRangeRef::resolve_references (const IdDictMgr& idd,  
					 IdDictDictionary& dictionary, 
					 IdDictRegion& region) { 
    if (m_range) m_range->resolve_references (idd, dictionary, region);
} 
  
void 
IdDictRangeRef::generate_implementation (const IdDictMgr& idd,  
					      IdDictDictionary& dictionary, 
					      IdDictRegion& region,
					      const std::string& tag) { 
    if (m_range) m_range->generate_implementation (idd, dictionary, region, tag);
} 
  
void 
IdDictRangeRef::reset_implementation () { 
    if (m_range) m_range->reset_implementation ();
} 
  
bool 
IdDictRangeRef::verify () const { 
    if (m_range) return (m_range->verify());
    return (true); 
} 
 
Range 
IdDictRangeRef::build_range () const { 
  Range result; 
  if (m_range) result = m_range->build_range();
  return (result);
}
 
 