/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SRCALOCALIBRATIONHITCONTAINERCNV
#define SRCALOCALIBRATIONHITCONTAINERCNV

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "CaloSimEvent/SrCaloCalibrationHitContainer.h"
#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainer_p2.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// typedef to the latest persistent versioCn
typedef SrCaloCalibrationHitContainer_p2 SrCaloCalibrationHitContainer_PERS;

class SrCaloCalibrationHitContainerCnv
    : public T_AthenaPoolCustomCnv<SrCaloCalibrationHitContainer,
                                   SrCaloCalibrationHitContainer_PERS> {
  friend class CnvFactory<SrCaloCalibrationHitContainerCnv>;

 public:
  SrCaloCalibrationHitContainerCnv(ISvcLocator* svcloc)
      : T_AthenaPoolCustomCnv<SrCaloCalibrationHitContainer,
                              SrCaloCalibrationHitContainer_PERS>(svcloc) {}

 protected:
  SrCaloCalibrationHitContainer_PERS* createPersistent(
      SrCaloCalibrationHitContainer* transCont);
  SrCaloCalibrationHitContainer* createTransient(const Token* token);
};

#endif
