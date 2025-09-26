/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictReference_H
#define IDDICT_IdDictReference_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
class IdDictSubRegion;

class IdDictReference : public IdDictRegionEntry { 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictReference (const std::string& subregion_name);
    IdDictReference (IdDictSubRegion* subregion);
    virtual ~IdDictReference ();


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
    std::string m_subregion_name; 
    IdDictSubRegion* m_subregion{}; 

    bool m_resolved_references{};
}; 

#endif

