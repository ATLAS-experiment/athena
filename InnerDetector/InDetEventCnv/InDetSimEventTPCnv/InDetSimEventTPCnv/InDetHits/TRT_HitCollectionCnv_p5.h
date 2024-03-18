/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMEVENTTPCNV_TRTHITCOLLECTIONCNV_P5_H
#define INDETSIMEVENTTPCNV_TRTHITCOLLECTIONCNV_P5_H

#include "InDetSimEvent/TRTUncompressedHitCollection.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "TRT_HitCollection_p5.h"


class TRT_HitCollectionCnv_p5 : public T_AthenaPoolTPCnvBase<TRTUncompressedHitCollection, TRT_HitCollection_p5>
{
 public:

  TRT_HitCollectionCnv_p5() {};

  virtual TRTUncompressedHitCollection* createTransient(const TRT_HitCollection_p5* persObj, MsgStream &log);

  virtual void persToTrans(const TRT_HitCollection_p5* persCont,
                           TRTUncompressedHitCollection* transCont,
                           MsgStream &log) ;
  virtual void transToPers(const TRTUncompressedHitCollection* transCont,
                           TRT_HitCollection_p5* persCont,
                           MsgStream &log) ;
};

#endif // INDETSIMEVENTTPCNV_TRTHITCOLLECTIONCNV_P5_H
