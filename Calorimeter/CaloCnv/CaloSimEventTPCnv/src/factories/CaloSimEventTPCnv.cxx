/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"
#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p3.h"
#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p4.h"
#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainerCnv_p1.h"
#include "CaloSimEventTPCnv/SrCaloCalibrationHitContainerCnv_p2.h"

DECLARE_TPCNV_FACTORY(CaloCalibrationHitContainerCnv_p3,
                      CaloCalibrationHitContainer,
                      CaloCalibrationHitContainer_p3,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(CaloCalibrationHitContainerCnv_p4,
                      CaloCalibrationHitContainer,
                      CaloCalibrationHitContainer_p4,
                      Athena::TPCnvVers::Current)

DECLARE_TPCNV_FACTORY(SrCaloCalibrationHitContainerCnv_p1,
                      SrCaloCalibrationHitContainer,
                      SrCaloCalibrationHitContainer_p1,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(SrCaloCalibrationHitContainerCnv_p2,
                      SrCaloCalibrationHitContainer,
                      SrCaloCalibrationHitContainer_p2,
                      Athena::TPCnvVers::Current)
