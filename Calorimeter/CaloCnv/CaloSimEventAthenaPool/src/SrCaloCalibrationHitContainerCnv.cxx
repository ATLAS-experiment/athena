/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SrCaloCalibrationHitContainerCnv.h"

#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainerCnv_p1.h"

SrCaloCalibrationHitContainer_PERS*
SrCaloCalibrationHitContainerCnv::createPersistent(
    SrCaloCalibrationHitContainer* transCont) {
  MsgStream mlog(msgSvc(), "SrCaloCalibrationHitContainerConverter");
  SrCaloCalibrationHitContainerCnv_p1 converter;
  SrCaloCalibrationHitContainer_PERS* persObj =
      converter.createPersistent(transCont, mlog);
  return persObj;
}

SrCaloCalibrationHitContainer*
SrCaloCalibrationHitContainerCnv::createTransient() {
  MsgStream mlog(msgSvc(), "SrCaloCalibrationHitContainerConverter");
  SrCaloCalibrationHitContainerCnv_p1 converter_p1;

  SrCaloCalibrationHitContainer* trans_cont(0);
  static const pool::Guid p1_guid("A435D71D-5A7B-4B96-AE4A-F245D14432D9");

  if (this->compareClassGuid(p1_guid)) {
    std::unique_ptr<SrCaloCalibrationHitContainer_p1> col_vect(
        this->poolReadObject<SrCaloCalibrationHitContainer_p1>());
    trans_cont = converter_p1.createTransient(col_vect.get(), mlog);
  } else {
    throw std::runtime_error(
        "Unsupported persistent version of Data container");
  }
  return trans_cont;
}
