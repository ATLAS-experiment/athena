/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOSIMEVENTTPCNV_CALOCALIBRATIONHITCONTAINERCNV_P4_H
#define CALOSIMEVENTTPCNV_CALOCALIBRATIONHITCONTAINERCNV_P4_H

#include "CaloSimEventTPCnv/CaloCalibrationHitContainer_p4.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

//typedef T_AthenaHitsVectorCnv< CaloCalibrationHitContainer, CaloCalibrationHitContainer_p1, CaloCalibrationHitCnv_p1 >  CaloCalibrationHitContainerCnv_p1;

class CaloCalibrationHitContainerCnv_p4 : public T_AthenaPoolTPCnvBase<CaloCalibrationHitContainer, CaloCalibrationHitContainer_p4>
{
 public:
  CaloCalibrationHitContainerCnv_p4(){};
  
  virtual void  persToTrans(const CaloCalibrationHitContainer_p4* persColl, CaloCalibrationHitContainer* transColl,  MsgStream &log);
  virtual void  transToPers(const CaloCalibrationHitContainer* transColl, CaloCalibrationHitContainer_p4* persColl,  MsgStream &log);
 private:
};

#endif // not CALOSIMEVENTTPCNV_CALOCALIBRATIONHITCONTAINERCNV_P4_H
