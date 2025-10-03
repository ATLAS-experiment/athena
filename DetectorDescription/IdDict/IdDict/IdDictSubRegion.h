/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictSubRegion_H
#define IDDICT_IdDictSubRegion_H

#include "IdDict/IdDictRegion.h"

class IdDictMgr;
class IdDictDictionary;

class IdDictSubRegion : public IdDictRegion { 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{
    IdDictSubRegion (const std::string& name,
                     const std::string& group,
                     const std::string& tag);
    virtual ~IdDictSubRegion (); 


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    // Dummy: never called - subregions are only generated via
    // IdDictReference where one already has a region which is passed in
    // as an arg with the method below
    virtual void generate_implementation (const IdDictMgr& idd,
                                          IdDictDictionary& dictionary,
                                          const std::string& tag = "") override;
    // Generate implementation - pass region to be filled for this subregion
    void generate_implementation (const IdDictMgr& idd,  
                                  IdDictDictionary& dictionary, 
                                  IdDictRegion& region,
                                  const std::string& tag = "");  
    virtual void reset_implementation () override;


    //@}
};

#endif

