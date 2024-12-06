/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictDictionaryRef_H
#define IDDICT_IdDictDictionaryRef_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
 
class IdDictDictionaryRef : public IdDictRegionEntry { 
public: 
    IdDictDictionaryRef () = default;
    ~IdDictDictionaryRef () = default; 
    void resolve_references (const IdDictMgr& idd, IdDictDictionary& dictionary, 
                             IdDictRegion& region);  
    void generate_implementation (const IdDictMgr& idd, IdDictDictionary& dictionary, 
                                  IdDictRegion& region, const std::string& tag = "");  
    void reset_implementation ();  
    bool verify () const;  
    Range build_range () const; 
    //data members made public
    std::string m_dictionary_name; 
    IdDictDictionary* m_dictionary{}; 

private:
    // We allow to regenerate the implementation with a tag. However,
    // propagation of information should only be done once.
    bool m_resolved_references{};
    bool m_generated_implementation{};
    bool m_propagated_information{};
}; 

#endif