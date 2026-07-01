/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRegionEntry_H
#define IDDICT_IdDictRegionEntry_H

#include <string>
#include <string_view>
class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;

class IdDictRegionEntry { 
public: 
    IdDictRegionEntry (); 
    virtual ~IdDictRegionEntry (); 
    virtual void resolve_references (IdDictMgr&,
        IdDictDictionary& , IdDictRegion& );
    virtual void generate_implementation (const IdDictMgr& ,  
        IdDictDictionary& , IdDictRegion& , std::string_view tag );
    virtual void reset_implementation (); 
    virtual bool verify () const;  
    virtual void clear ();
    virtual Range build_range () const = 0; 
}; 

#endif
