/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p3.h"
#include "CaloSimEventTPCnv/CaloCalibrationHitContainerCnv_p4.h"

DECLARE_TPCNV_FACTORY(CaloCalibrationHitContainerCnv_p3,
                      CaloCalibrationHitContainer,
                      CaloCalibrationHitContainer_p3,
                      Athena::TPCnvVers::Current)

DECLARE_TPCNV_FACTORY(CaloCalibrationHitContainerCnv_p4,
                      CaloCalibrationHitContainer,
                      CaloCalibrationHitContainer_p4,
                      Athena::TPCnvVers::Old)
