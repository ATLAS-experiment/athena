/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRegion_H
#define IDDICT_IdDictRegion_H

#include "IdDict/IdDictDictEntry.h"
#include "IdDict/IdDictFieldImplementation.h"

#include <string>
#include <vector>
#include <memory>

class Range;
class IdDictMgr;
class IdDictDictionary;
class IdDictRegionEntry;
class IdDictRegion;

class IdDictRegion : public IdDictDictEntry{ 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictRegion (const std::string& name,
                  const std::string& group,
                  const std::string& tag);
    virtual ~IdDictRegion ();

    IdDictRegion (IdDictRegion&&);
    IdDictRegion& operator= (IdDictRegion&&);

    IdDictRegion (const IdDictRegion&) = delete;
    IdDictRegion& operator= (const IdDictRegion&) = delete;


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& name() const;
    virtual std::string group_name () const override;
    const std::string& tag() const;
    size_t fieldSize() const; 
    size_t size() const;
    size_t index() const;
    size_t n_implementation() const;
    const IdDictFieldImplementation& implementation(size_t i) const;
    double eta0() const;
    double deta() const;
    double phi0() const;
    double dphi() const;
    const IdDictRegion* next_abs_eta() const;
    const IdDictRegion* prev_abs_eta() const;
    const std::vector<const IdDictRegion*>& prev_samp() const;
    const std::vector<const IdDictRegion*>& next_samp() const;
    const std::vector<const IdDictRegion*>& prev_subdet() const;
    const std::vector<const IdDictRegion*>& next_subdet() const;
    const std::vector<std::string>& prev_samp_names() const;
    const std::vector<std::string>& next_samp_names() const;
    const std::vector<std::string>& prev_subdet_names() const;
    const std::vector<std::string>& next_subdet_names() const;
    size_t n_entries() const;
    const IdDictRegionEntry& entry(size_t i) const;
    bool is_empty() const;


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    /// Add entry to the end of the list.
    void add_entry (std::unique_ptr<IdDictRegionEntry> entry);

    /// Add entry to the start of the list.
    void prepend_entry (std::unique_ptr<IdDictRegionEntry> entry);

    /// Non-const access to entry pointers.
    const std::vector<std::unique_ptr<IdDictRegionEntry> >& entries();

    /// Non-const access to implementation objects.
    IdDictFieldImplementation& implementation(size_t i);

    /// Add a new implementation object and return a reference to it.
    IdDictFieldImplementation& new_implementation();

    /// Set the name for next_abs_eta.
    void set_next_abs_eta_name (const std::string& name);

    /// Add a previous sample name.
    void add_prev_samp_name (const std::string& name);

    /// Add a next sample name.
    void add_next_samp_name (const std::string& name);

    /// Add a previous subdetector name.
    void add_prev_subdet_name (const std::string& name);

    /// Add a next subdetector name.
    void add_next_subdet_name (const std::string& name);

    /// Set eta/phi variables.
    void set_etaphi (double eta0, double deta, double phi0, double dphi);

    /// Set is_empty flag.
    void set_is_empty();

    void find_neighbours (IdDictDictionary& dictionary);

    virtual void set_index (size_t index) override;
    virtual Range build_range () const override;
    virtual void resolve_references (IdDictMgr& idd,
                                     IdDictDictionary& dictionary) override;
    virtual void generate_implementation (const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          const std::string& tag = "") override;
    virtual void reset_implementation () override;
    virtual bool verify () const override;
    virtual void clear () override;

    // Loop over levels and set the bit offset for each FieldImplementation
    void integrate_bits();

    //@}


protected:
    std::vector <std::unique_ptr<IdDictRegionEntry> > m_entries;
private:
    std::vector <IdDictFieldImplementation> m_implementation; 
    size_t                                  m_index{}; 
    std::string                             m_name;
    std::string                             m_group; 
    std::string                             m_tag; 
    bool                                    m_is_empty{false};//?
    std::string                             m_next_abs_eta_name;  
    std::vector<std::string>                m_prev_samp_names;
    std::vector<std::string>                m_next_samp_names;
    std::vector<std::string>                m_prev_subdet_names;
    std::vector<std::string>                m_next_subdet_names;
    IdDictRegion*                           m_prev_abs_eta{};
    IdDictRegion*                           m_next_abs_eta{};
    std::vector<const IdDictRegion*>        m_prev_samp;
    std::vector<const IdDictRegion*>        m_next_samp;
    std::vector<const IdDictRegion*>        m_prev_subdet;
    std::vector<const IdDictRegion*>        m_next_subdet;
    float                                   m_eta0{};
    float                                   m_deta{};
    float                                   m_phi0{};
    float                                   m_dphi{};

    bool m_generated_implementation{};
};


inline
const std::string& IdDictRegion::name() const
{
    return m_name;
}


inline
const std::string& IdDictRegion::tag() const
{
    return m_tag;
}


inline
size_t IdDictRegion::index() const
{
    return m_index;
}


inline
double IdDictRegion::eta0() const
{
    return m_eta0;
}


inline
double IdDictRegion::deta() const
{
    return m_deta;
}


inline
double IdDictRegion::phi0() const
{
    return m_phi0;
}


inline
double IdDictRegion::dphi() const
{
    return m_dphi;
}


inline
const IdDictRegion* IdDictRegion::next_abs_eta() const
{
    return m_next_abs_eta;
}


inline
const IdDictRegion* IdDictRegion::prev_abs_eta() const
{
    return m_prev_abs_eta;
}


inline
const std::vector<const IdDictRegion*>& IdDictRegion::prev_samp() const
{
    return m_prev_samp;
}


inline
const std::vector<const IdDictRegion*>& IdDictRegion::next_samp() const
{
    return m_next_samp;
}


inline
const std::vector<const IdDictRegion*>& IdDictRegion::prev_subdet() const
{
    return m_prev_subdet;
}


inline
const std::vector<const IdDictRegion*>& IdDictRegion::next_subdet() const
{
    return m_next_subdet;
}


inline
const std::vector<std::string>& IdDictRegion::prev_samp_names() const
{
    return m_prev_samp_names;
}


inline
const std::vector<std::string>& IdDictRegion::next_samp_names() const
{
    return m_next_samp_names;
}


inline
const std::vector<std::string>& IdDictRegion::prev_subdet_names() const
{
    return m_prev_subdet_names;
}


inline
const std::vector<std::string>& IdDictRegion::next_subdet_names() const
{
    return m_next_subdet_names;
}


inline
size_t IdDictRegion::n_entries() const
{
    return m_entries.size();
}


inline
const IdDictRegionEntry& IdDictRegion::entry(size_t i) const
{
    return *m_entries.at(i);
}


inline
bool IdDictRegion::is_empty() const
{
    return m_is_empty;
}


/// Non-const access to entry pointers.
inline
const std::vector<std::unique_ptr<IdDictRegionEntry> >& IdDictRegion::entries()
{
    return m_entries;
}


#endif

