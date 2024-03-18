/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GenericMuonSimHitCOLLECTIONCNV_P3_H
#define GenericMuonSimHitCOLLECTIONCNV_P3_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "MuonSimEvent/GenericMuonSimHitCollection.h"
#include "GenericMuonSimHitCollection_p3.h"
// namespace Muon {
//     class GenericMuonSimHitCollection_p3;
// }

class GenericMuonSimHitCollectionCnv_p3 : public T_AthenaPoolTPCnvBase<GenericMuonSimHitCollection, Muon::GenericMuonSimHitCollection_p3>
{
 public:

  GenericMuonSimHitCollectionCnv_p3()  {};

  virtual GenericMuonSimHitCollection* createTransient(const Muon::GenericMuonSimHitCollection_p3* persObj, MsgStream &log);
  virtual void  persToTrans(const Muon::GenericMuonSimHitCollection_p3* persCont,
                GenericMuonSimHitCollection* transCont,
                MsgStream &log) ;
  virtual void  transToPers(const GenericMuonSimHitCollection* transCont,
                Muon::GenericMuonSimHitCollection_p3* persCont,
                MsgStream &log) ;

};

#endif
