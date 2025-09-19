/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictField_H
#define IDDICT_IdDictField_H

#include "Identifier/ExpandedIdentifier.h"
#include <string>
#include <vector>

struct IdDictLabel;
class IdDictMgr;
  
class IdDictField {  
public:
    // ==================================
    //** @name Simple accessors.
    // @{

    size_t index() const;
    size_t get_label_number () const;  
    IdDictLabel* find_label (const std::string& name) const;
    const IdDictLabel& label (size_t index) const;
    const std::string get_label (size_t index) const;  
    ExpandedIdentifier::element_type get_label_value (const std::string& name) const; 


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    void add_label (IdDictLabel* label);
    void resolve_references (const IdDictMgr& idd);  
    void generate_implementation (const IdDictMgr& idd, const std::string& tag = "");  
    void reset_implementation ();  
    bool verify () const;  
    void clear ();


    //@}

    //data members are public
    std::string                   m_name;  
    std::vector <IdDictLabel*>    m_labels; 
    size_t                        m_index{}; 
}; 


inline
size_t IdDictField::index() const
{
    return m_index;
}


inline
const IdDictLabel& IdDictField::label (size_t index) const
{
    return *m_labels.at(index);
}


#endif
