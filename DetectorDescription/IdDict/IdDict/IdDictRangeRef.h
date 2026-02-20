/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictRangeRef_H
#define IDDICT_IdDictRangeRef_H

#include "IdDict/IdDictRegionEntry.h"
#include <string>

class IdDictMgr;
class IdDictDictionary;
class IdDictRegion;
class Range;
class IdDictRange;


class IdDictRangeRef : public IdDictRegionEntry { 
public: 
    // ==================================
    //** @name Constructor/destructor
    // @{

    IdDictRangeRef (IdDictRange& range);
    virtual ~IdDictRangeRef () = default;


    //@}
    // ==================================
    //** @name Simple accessors.
    // @{

    const IdDictRange& range() const;

    //@}
    // ==================================
    //** @name Methods used to initialize the object.
    // @{

    virtual void resolve_references (IdDictMgr& idd,
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
    IdDictRange& m_range;
}; 


inline
const IdDictRange& IdDictRangeRef::range() const
{
    return m_range;
}


#endif
