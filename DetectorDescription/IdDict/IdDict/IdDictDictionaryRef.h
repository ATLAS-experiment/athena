/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictDictionaryRef_H
#define IDDICT_IdDictDictionaryRef_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
 
class IdDictDictionaryRef : public IdDictRegionEntry { 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictDictionaryRef (const std::string& dictionary_name);
    IdDictDictionaryRef (IdDictDictionary* dictionary); // For testing
    virtual ~IdDictDictionaryRef ();


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const std::string& dictionary_name() const;


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
    virtual void reset_implementation () override;
    virtual bool verify () const override;
    virtual Range build_range () const override;


    //@}


private:
    std::string m_dictionary_name; 
    IdDictDictionary* m_dictionary{}; 

    // We allow to regenerate the implementation with a tag. However,
    // propagation of information should only be done once.
    bool m_resolved_references{};
    bool m_generated_implementation{};
    bool m_propagated_information{};
}; 


inline
const std::string& IdDictDictionaryRef::dictionary_name() const
{
    return m_dictionary_name;
}


#endif
