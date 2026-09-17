/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBRATIONHITCONTAINERCNV
#define CALOCALIBRATIONHITCONTAINERCNV

#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloSimEventTPCnv/CaloCalibrationHitContainer_p4.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// typedef to the latest persistent version
typedef CaloCalibrationHitContainer_p4  CaloCalibrationHitContainer_PERS;

class CaloCalibrationHitContainerCnv  : public T_AthenaPoolCustomCnv<CaloCalibrationHitContainer, CaloCalibrationHitContainer_PERS > {
  friend class CnvFactory<CaloCalibrationHitContainerCnv>;
public:
  CaloCalibrationHitContainerCnv(ISvcLocator* svcloc) :
        T_AthenaPoolCustomCnv<CaloCalibrationHitContainer, CaloCalibrationHitContainer_PERS >( svcloc) {}
protected:
  CaloCalibrationHitContainer_PERS*  createPersistent(CaloCalibrationHitContainer* transCont);
  CaloCalibrationHitContainer*       createTransient(const Token* token);
};

#endif
