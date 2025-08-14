/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETEVENTATHENAPOOL_INDETSIMDATACOLLECTIONCNV_H
#define INDETEVENTATHENAPOOL_INDETSIMDATACOLLECTIONCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetSimDataCollectionCnv_p1.h"
#include "InDetSimDataCollectionCnv_p2.h"
#include "InDetSimDataCollectionCnv_p3.h"
#include "InDetSimDataCollectionCnv_p4.h"

// Gaudi
#include "GaudiKernel/MsgStream.h"
// typedef to the latest persistent version
typedef  InDetSimDataCollection_p4 InDetSimDataCollection_PERS;
typedef  InDetSimDataCollectionCnv_p4 InDetSimDataCollectionCnv_PERS;

// base class
typedef  T_AthenaPoolCustomCnv<InDetSimDataCollection, InDetSimDataCollection_PERS >   InDetSimDataCollectionCnvBase;

class InDetSimDataCollectionCnv : public InDetSimDataCollectionCnvBase {
  friend class CnvFactory<InDetSimDataCollectionCnv >;

protected:
public:
  InDetSimDataCollectionCnv (ISvcLocator* svcloc) : InDetSimDataCollectionCnvBase(svcloc) {}
protected:
  virtual InDetSimDataCollection_PERS*   createPersistent (InDetSimDataCollection* transCont);
  virtual InDetSimDataCollection* createTransient ();
};

#endif // INDETEVENTATHENAPOOL_INDETSIMDATACOLLECTIONCNV_H
