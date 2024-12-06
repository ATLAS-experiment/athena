/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRegionEntry_H
#define IDDICT_IdDictRegionEntry_H

#include <string>
class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;

class IdDictRegionEntry { 
public: 
    IdDictRegionEntry (); 
    virtual ~IdDictRegionEntry (); 
    virtual void resolve_references (const IdDictMgr& ,  
        IdDictDictionary& , IdDictRegion& );
    virtual void generate_implementation (const IdDictMgr& ,  
        IdDictDictionary& , IdDictRegion& , const std::string& );
    virtual void reset_implementation (); 
    virtual bool verify () const;  
    virtual void clear ();
    virtual Range build_range () const = 0; 
}; 

#endif