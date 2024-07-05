/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigInDetEventTPCnv
 * @class  : TrigInDetTrackCollection_p1
 *
 * @brief persistent partner for TrigInDetTrackCollection
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 **********************************************************************************/
#ifndef TRIGINDETEVENTTPCNV_TRIGINDETTRACKCOLLECTION_P1_H
#define TRIGINDETEVENTTPCNV_TRIGINDETTRACKCOLLECTION_P1_H

#include "AthenaPoolUtilities/TPObjRef.h"
#include <vector>


class TrigInDetTrackCollection_p1
{

 public:

  TrigInDetTrackCollection_p1() {}
  friend class TrigInDetTrackCollectionCnv_p1; 

  int m_RoI_ID{};

  //the DataVector<TrigInDetTrack> base class
  std::vector<TPObjRef> m_trigInDetTrackVector;
  
};

#endif
