/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGCSimHitCOLLECTIONCNV_P4_H
#define sTGCSimHitCOLLECTIONCNV_P4_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "sTGCSimHitCollection_p4.h"

class sTGCSimHitCollectionCnv_p4 : public T_AthenaPoolTPCnvBase<sTGCSimHitCollection, Muon::sTGCSimHitCollection_p4>
{
 public:

  sTGCSimHitCollectionCnv_p4()  {};

  virtual sTGCSimHitCollection* createTransient(const Muon::sTGCSimHitCollection_p4* persObj, MsgStream &log);
  virtual void  persToTrans(const Muon::sTGCSimHitCollection_p4* persCont,
                sTGCSimHitCollection* transCont,
                MsgStream &log) ;
  virtual void  transToPers(const sTGCSimHitCollection* transCont,
                Muon::sTGCSimHitCollection_p4* persCont,
                MsgStream &log) ;

};

#endif
