 /*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictAltRegions_H
#define IDDICT_IdDictAltRegions_H

#include "IdDict/IdDictDictEntry.h"

#include <string>
#include <map>
#include <memory>

class Range;
class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;


 
class IdDictAltRegions : public IdDictDictEntry{ 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictAltRegions (); 
    virtual ~IdDictAltRegions ();


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    /// Group name for this region.
    virtual std::string group_name () const override;

    /// Currently selected region.
    IdDictRegion* selected_region();


    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    /// Add a new region, with key given by the tag.
    void add_region (std::unique_ptr<IdDictRegion> region);

    /// Select the named region.
    void select_region (const std::string& name);


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


    //@}


private:
    using map_type = std::map<std::string, std::unique_ptr<IdDictRegion> >;
    using map_iterator = map_type::iterator;
    using value_type = map_type::value_type;

    map_type      m_regions;
    IdDictRegion* m_selected_region{};
}; 


/// Currently selected region.
inline
IdDictRegion* IdDictAltRegions::selected_region()
{
    return m_selected_region;
}


#endif
