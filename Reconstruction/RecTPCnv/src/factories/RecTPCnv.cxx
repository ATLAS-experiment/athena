/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

#include "RecTPCnv/MuonCaloEnergyContainerCnv_tlp1.h"
#include "RecTPCnv/MuonCaloEnergyContainerCnv_p1.h"

DECLARE_TPCNV_FACTORY(MuonCaloEnergyContainerCnv_tlp1,
                      MuonCaloEnergyContainer,
                      MuonCaloEnergyContainer_tlp1,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonCaloEnergyContainerCnv_p1,
                      MuonCaloEnergyContainer,
                      MuonCaloEnergyContainer_p1,
                      Athena::TPCnvVers::Current)
                      
