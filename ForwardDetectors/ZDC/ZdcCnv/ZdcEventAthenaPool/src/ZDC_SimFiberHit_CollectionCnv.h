/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** 
 * @file ZDC_SimFiberHit_CollectionCnv.h
 * @brief Generated header file which defines a typedef for templated converter class
 * @author RD Schaffer <R.D. Schaffer@cern.ch>
 */

#ifndef ZDC_SimFiberHit_CollectionCnv_H
#define ZDC_SimFiberHit_CollectionCnv_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_Collection_p1.h"

typedef ZDC_SimFiberHit_Collection_p1 ZDC_SimFiberHit_Collection_PERS;

typedef T_AthenaPoolCustomCnv<ZDC_SimFiberHit_Collection, ZDC_SimFiberHit_Collection_PERS> ZDC_SimFiberHit_CollectionCnvBase;

class ZDC_SimFiberHit_CollectionCnv  : public ZDC_SimFiberHit_CollectionCnvBase {

  friend class CnvFactory<ZDC_SimFiberHit_CollectionCnv>;

protected:

public:
  ZDC_SimFiberHit_CollectionCnv(ISvcLocator* svcloc) : ZDC_SimFiberHit_CollectionCnvBase ( svcloc) {}
protected:
  virtual ZDC_SimFiberHit_Collection_PERS*  createPersistent(ZDC_SimFiberHit_Collection* transCont);
  virtual ZDC_SimFiberHit_Collection*       createTransient(const Token* token);

};

#endif

