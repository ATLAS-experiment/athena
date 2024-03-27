/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKRECORDCOLLECTION_P3_H
#define TRACKRECORDCOLLECTION_P3_H

/*

Persistent represenation of TrackRecordCollection
Author: Davide Costanzo

*/

#include <vector>
#include <string>
#include "G4SimTPCnv/TrackRecord_p2.h"


class TrackRecordCollection_p3
{
public:
    /// typedefs
    typedef std::vector<TrackRecord_p2> HitVector;
    typedef HitVector::const_iterator const_iterator;
    typedef HitVector::iterator       iterator;


    /// Default constructor
    TrackRecordCollection_p3 ();

    // Accessors
    const std::string&  name() const;
    const HitVector&    getVector() const;

    std::vector<TrackRecord_p2>   m_cont;
    std::string m_name;
};


// inlines

inline
TrackRecordCollection_p3::TrackRecordCollection_p3 () {}

inline
const std::string&
TrackRecordCollection_p3::name() const
{return m_name;}

inline
const std::vector<TrackRecord_p2>&
TrackRecordCollection_p3::getVector() const
{return m_cont;}

#endif
