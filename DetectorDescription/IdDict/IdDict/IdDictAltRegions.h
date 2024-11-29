/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictAltRegions_H
#define IDDICT_IdDictAltRegions_H

#include "IdDict/IdDictDictEntry.h"

#include <string>
#include <map>

class Range;
class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;


 
class IdDictAltRegions : public IdDictDictEntry{ 
public: 
    IdDictAltRegions (); 
    ~IdDictAltRegions (); 
    std::string group_name () const; 
    Range build_range () const; 
    void set_index (size_t index);
    void resolve_references (const IdDictMgr& idd, 
                             IdDictDictionary& dictionary);
    void generate_implementation (const IdDictMgr& idd, 
                                  IdDictDictionary& dictionary, 
                                  const std::string& tag = "");  
    void reset_implementation ();  
    bool verify () const;  
    void clear (); 
    //data members are public
    typedef std::map<std::string, IdDictRegion* > map_type;
    typedef map_type::iterator                    map_iterator;
    typedef map_type::value_type                  value_type;
    map_type      m_regions;
    IdDictRegion* m_selected_region{};

}; 

#endif


