 /*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRange_H
#define IDDICT_IdDictRange_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>
#include <vector>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
class IdDictField;

class IdDictRange : public IdDictRegionEntry { 
public: 
    IdDictRange () = default; 
    ~IdDictRange () = default; 
    void resolve_references (const IdDictMgr& idd,  
                             IdDictDictionary& dictionary, 
                             IdDictRegion& region);  
    void generate_implementation (const IdDictMgr& idd,  
                                  IdDictDictionary& dictionary, 
                                  IdDictRegion& region,
                                  const std::string& tag = "");   
    Range build_range () const; 
    //data members made public
    std::string m_field_name; 
    IdDictField* m_field{}; 
    
    enum specification_type{ 
        unknown, 
        by_value, 
        by_values, 
        by_label, 
        by_labels,
        by_minmax 
    } ; 
 
    enum continuation_mode{ 
        none, 
        has_next, 
        has_previous, 
        has_both,
        wrap_around
    }; 
    specification_type m_specification{unknown}; 
    std::string m_tag; 
    std::string m_label; 
    int m_value{}; 
    int m_minvalue{}; 
    int m_maxvalue{};
    int m_prev_value{};
    int m_next_value{};
    continuation_mode m_continuation_mode{none};
    std::vector <std::string> m_labels; 
    std::vector <int> m_values; 

private:
    bool m_resolved_references{};
}; 
#endif

