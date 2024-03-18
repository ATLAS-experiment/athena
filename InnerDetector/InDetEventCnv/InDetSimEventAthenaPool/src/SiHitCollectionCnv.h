/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMEVENTATHENAPOOL_SIHITCOLLECTIONCNV_H
#define INDETSIMEVENTATHENAPOOL_SIHITCOLLECTIONCNV_H

#include "InDetSimEvent/SiHitCollection.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollection_p1.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p1.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollection_p2.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p2.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollection_p3.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p3.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollection_p4.h"
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p4.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// typedef to the latest persistent version
typedef SiHitCollection_p3     SiHitCollection_PERS;
typedef SiHitCollectionCnv_p3  SiHitCollectionCnv_PERS;

class SiHitCollectionCnv  : public T_AthenaPoolCustomCnv<SiHitCollection, SiHitCollection_PERS > {
  friend class CnvFactory<SiHitCollectionCnv>;
public:
  SiHitCollectionCnv(ISvcLocator* svcloc) :
        T_AthenaPoolCustomCnv<SiHitCollection, SiHitCollection_PERS >( svcloc) {}
protected:
  SiHitCollection_PERS*  createPersistent(SiHitCollection* transCont);
  SiHitCollection*       createTransient ();
};


#endif // INDETSIMEVENTATHENAPOOL_SIHITCOLLECTIONCNV_H
