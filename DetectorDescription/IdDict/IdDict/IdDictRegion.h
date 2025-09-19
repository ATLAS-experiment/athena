/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRegion_H
#define IDDICT_IdDictRegion_H

#include "IdDict/IdDictDictEntry.h"

#include <string>
#include <vector>

class Range;
class IdDictMgr;
class IdDictDictionary;
class IdDictRegionEntry;
class IdDictFieldImplementation;
class IdDictRegion;

class IdDictRegion : public IdDictDictEntry{ 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictRegion () = default; 
    virtual ~IdDictRegion () =  default; 


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& name() const;
    virtual std::string group_name () const override;
    size_t fieldSize() const; 
    size_t size() const;
    size_t index() const;
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
    size_t n_entries() const;
    const IdDictRegionEntry& entry(size_t i) const;



    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    void add_entry (IdDictRegionEntry* entry);

    void find_neighbours (const IdDictDictionary& dictionary);

    virtual void set_index (size_t index) override;
    virtual Range build_range () const override;
    virtual void resolve_references (const IdDictMgr& idd,
                                     IdDictDictionary& dictionary) override;
    virtual void generate_implementation (const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          const std::string& tag = "") override;
    virtual void reset_implementation () override;
    virtual bool verify () const override;
    virtual void clear () override;


    //@}

    //
    //data members are public
    std::vector <IdDictRegionEntry*>        m_entries;
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
    

protected:
    bool m_resolved_references{};
    bool m_generated_implementation{};
};


inline
const std::string& IdDictRegion::name() const
{
    return m_name;
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
size_t IdDictRegion::n_entries() const
{
    return m_entries.size();
}


inline
const IdDictRegionEntry& IdDictRegion::entry(size_t i) const
{
    return *m_entries.at(i);
}


#endif

