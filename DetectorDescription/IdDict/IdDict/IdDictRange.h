 /*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictRange () = default; 
    ~IdDictRange () = default; 


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& field_name() const;
    const std::string& label() const;


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    virtual void resolve_references (const IdDictMgr& idd,
                                     IdDictDictionary& dictionary,
                                     IdDictRegion& region) override;
    virtual void generate_implementation (const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          IdDictRegion& region,
                                          const std::string& tag = "") override;
    virtual Range build_range () const override;


    //@}

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


inline
const std::string& IdDictRange::field_name() const
{
  return m_field_name;
}


inline
const std::string& IdDictRange::label() const
{
    return m_label;
}


#endif

