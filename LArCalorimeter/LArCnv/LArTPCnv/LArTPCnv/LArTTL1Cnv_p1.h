/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARTTL1CNV_P1_H
#define LARTPCNV_LARTTL1CNV_P1_H

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// LArTPCnv includes
#include "LArTPCnv/LArTTL1_p1.h"

// LArRawEvent includes
#include "LArRawEvent/LArTTL1.h"

class MsgStream;

class LArTTL1Cnv_p1 : public T_AthenaPoolTPCnvBase<LArTTL1, LArTTL1_p1> {

public:

  /** Default constructor:
   */
  LArTTL1Cnv_p1() {}

  /** Method creating the transient representation LArTTL1
   *  from its persistent representation LArTTL1_p1
   */
  virtual void persToTrans(const LArTTL1_p1* persObj, LArTTL1* transObj, MsgStream &log);

  /** Method creating the persistent representation LArTTL1_p1
   *  from its transient representation LArTTL1
   */
  virtual void transToPers(const LArTTL1* transObj, LArTTL1_p1* persObj, MsgStream &log);

};

#endif //> LARTPCNV_LARTTL1CNV_P1_H
