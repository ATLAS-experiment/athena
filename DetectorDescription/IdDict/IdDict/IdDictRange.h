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
    enum specification_type{
        unknown,
        by_value,
        by_values,
        by_label,
        by_labels,
        by_minmax
    };
    enum continuation_mode{
        none,
        has_next,
        has_previous,
        has_both,
        wrap_around
    };


    // ==================================
    //** @name Constructor/destructor
    // @{

    /// Set name only; no range information.
    IdDictRange (const std::string& field_name);

    /// By label.
    IdDictRange (const std::string& field_name,  const std::string& label);

    /// By value.
    IdDictRange (const std::string& field_name, int value);

    /// By minmax.
    IdDictRange (const std::string& field_name, int minvalue, int maxvalue);

    /// By list of values.
    IdDictRange (const std::string& field_name, const std::vector<int>& values);

    /// By list of labels.
    IdDictRange (const std::string& field_name,
                 const std::vector<std::string>& labels);

    virtual ~IdDictRange () = default;


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& field_name() const;
    const std::string& label() const;
    const IdDictField* field() const;
    specification_type specification() const;
    const std::vector<int>& values() const;
    const std::vector<std::string>& labels() const;


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    /// By label.
    void set_range (const std::string& label);

    /// By value.
    void set_range (int value);

    /// By minmax.
    void set_range (int minvalue, int maxvalue);

    /// By list of values.
    void set_range (const std::vector<int>& values);

    /// By list of labels.
    void set_range (const std::vector<std::string>& labels);


    /// Set previous value and adjust continuation mode.
    void set_prev(int prev);


    /// Set next value and adjust continuation mode.
    void set_next(int next);


    /// Enable wraparound.
    void set_wrap_around();

    virtual void resolve_references (IdDictMgr& idd,
                                     IdDictDictionary& dictionary,
                                     IdDictRegion& region) override;
    virtual void generate_implementation (const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          IdDictRegion& region,
                                          const std::string& tag = "") override;
    virtual Range build_range () const override;


    //@}


private:
    std::string m_field_name; 
    IdDictField* m_field{}; 
    
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


inline
const IdDictField* IdDictRange::field() const
{
  return m_field;
}


inline
IdDictRange::specification_type IdDictRange::specification() const
{
    return m_specification;
}


inline
const std::vector<int>& IdDictRange::values() const
{
    return m_values;
}


inline
const std::vector<std::string>& IdDictRange::labels() const
{
    return m_labels;
}


#endif

