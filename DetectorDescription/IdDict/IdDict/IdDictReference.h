/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictReference_H
#define IDDICT_IdDictReference_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
class IdDictSubRegion;

class IdDictReference : public IdDictRegionEntry { 
public: 
    IdDictReference (); 
    ~IdDictReference (); 
    void resolve_references (const IdDictMgr& idd,  
                             IdDictDictionary& dictionary, 
                             IdDictRegion& region);  
    void generate_implementation (const IdDictMgr& idd,  
                                  IdDictDictionary& dictionary, 
                                  IdDictRegion& region,
                                  const std::string& tag = "");  
    void reset_implementation ();  
    bool verify () const;  
    Range build_range () const; 
    //data members made public
    std::string m_subregion_name; 
    IdDictSubRegion* m_subregion{}; 

private:
    bool m_resolved_references{};
}; 

#endif

