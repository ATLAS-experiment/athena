/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_EVENTTPCNV_LUCID_SIMHITCOLLECTION_P3_H
#define LUCID_EVENTTPCNV_LUCID_SIMHITCOLLECTION_P3_H

#include <vector>
#include "LUCID_SimHit_p3.h"


class LUCID_SimHitCollection_p3
{
public:
  /// typedefs
  typedef std::vector<LUCID_SimHit_p3> HitVector;
  typedef HitVector::const_iterator const_iterator;
  typedef HitVector::iterator       iterator;


  /// Default constructor
  LUCID_SimHitCollection_p3 ();
  // Accessors
  const std::string&  name() const;
  const HitVector&    getVector() const;
  //private:
  std::vector<LUCID_SimHit_p3>   m_cont;
  std::string m_name;
};


// inlines

inline
LUCID_SimHitCollection_p3::LUCID_SimHitCollection_p3 () {}

inline
const std::string&
LUCID_SimHitCollection_p3::name() const
{return m_name;}

inline
const std::vector<LUCID_SimHit_p3>&
LUCID_SimHitCollection_p3::getVector() const
{return m_cont;}

#endif // LUCID_EVENTTPCNV_LUCID_SIMHITCOLLECTION_P3_H
