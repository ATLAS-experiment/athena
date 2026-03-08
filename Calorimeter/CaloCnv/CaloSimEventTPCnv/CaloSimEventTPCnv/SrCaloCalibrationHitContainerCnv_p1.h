/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOSIMEVENTTPCNV_SRCaloCalibrationHitContainerCNV_P1_H
#define CALOSIMEVENTTPCNV_SRCaloCalibrationHitContainerCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "CaloSimEvent/SrCaloCalibrationHitContainer.h"
#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainer_p1.h"

class SrCaloCalibrationHitContainerCnv_p1
    : public T_AthenaPoolTPCnvBase<SrCaloCalibrationHitContainer,
                                   SrCaloCalibrationHitContainer_p1> {
 public:
  SrCaloCalibrationHitContainerCnv_p1() {};

  virtual void persToTrans(const SrCaloCalibrationHitContainer_p1* persColl,
                           SrCaloCalibrationHitContainer* transColl,
                           MsgStream& log);
  virtual void transToPers(const SrCaloCalibrationHitContainer* transColl,
                           SrCaloCalibrationHitContainer_p1* persColl,
                           MsgStream& log);

 private:
};

#endif  // not CALOSIMEVENTTPCNV_SRCALOCALIBRATIONHITCONTAINERCNV_P1_H
