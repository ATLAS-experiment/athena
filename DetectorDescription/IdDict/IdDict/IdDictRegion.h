/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    IdDictRegion () = default; 
    virtual ~IdDictRegion () =  default; 
    std::string group_name () const; 
    Range build_range () const; 
    void set_index (size_t index);
    void add_entry (IdDictRegionEntry* entry); 
    void resolve_references (const IdDictMgr& idd, 
                             IdDictDictionary& dictionary); 
    void generate_implementation (const IdDictMgr& idd, 
                                  IdDictDictionary& dictionary, 
                                  const std::string& tag = "");  
    void find_neighbours (const IdDictDictionary& dictionary);
    void reset_implementation ();  
    bool verify () const;  
    void clear ();
    size_t fieldSize() const; 
    size_t size() const;
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
    std::vector<IdDictRegion*>              m_prev_samp;
    std::vector<IdDictRegion*>              m_next_samp;
    std::vector<IdDictRegion*>              m_prev_subdet;
    std::vector<IdDictRegion*>              m_next_subdet;
    float                                   m_eta0{};
    float                                   m_deta{};
    float                                   m_phi0{};
    float                                   m_dphi{};
    

protected:
    bool m_resolved_references{};
    bool m_generated_implementation{};
}; 

#endif

