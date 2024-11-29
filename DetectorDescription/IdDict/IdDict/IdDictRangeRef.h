/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRangeRef_H
#define IDDICT_IdDictRangeRef_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
class IdDictRange;


class IdDictRangeRef : public IdDictRegionEntry { 
public: 
    IdDictRangeRef () = default; 
    ~IdDictRangeRef () = default; 
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
    //data member is public
    IdDictRange* m_range{};
}; 

#endif
