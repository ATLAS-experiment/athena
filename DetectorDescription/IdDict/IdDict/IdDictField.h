/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictField_H
#define IDDICT_IdDictField_H

#include "Identifier/ExpandedIdentifier.h"
#include <string>
#include <vector>

class IdDictLabel;
class IdDictMgr;
  
class IdDictField {  
public:
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictField (const std::string& name);


    // @}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& name() const;
    size_t index() const;
    size_t get_label_number () const;
    IdDictLabel* find_label (const std::string& name) const;
    const IdDictLabel& label (size_t index) const;
    const std::string& get_label (size_t index) const;
    ExpandedIdentifier::element_type get_label_value (const std::string& name) const; 


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    void add_label (IdDictLabel* label);
    void set_index (size_t index);
    bool verify () const;  
    void clear ();


    //@}


private:
    std::string                   m_name;  
    std::vector <IdDictLabel*>    m_labels; 
    size_t                        m_index{}; 
}; 


inline
const std::string& IdDictField::name() const
{
    return m_name;
}


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
