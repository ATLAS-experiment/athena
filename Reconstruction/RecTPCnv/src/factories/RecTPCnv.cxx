/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

//#include "RecTPCnv/MuonContainerCnvARA_p1.h"
#include "RecTPCnv/MuonContainerCnv_p1.h"
#include "RecTPCnv/MuonContainerCnv_p2.h"
#include "RecTPCnv/MuonContainerCnv_p3.h"
#include "RecTPCnv/MuonContainerCnv_p4.h"
#include "RecTPCnv/MuonContainerCnv_p5.h"
#include "RecTPCnv/MuonContainerCnv_p6.h"

#include "RecTPCnv/MuonCaloEnergyContainerCnv_tlp1.h"
#include "RecTPCnv/MuonCaloEnergyContainerCnv_p1.h"
#include "RecTPCnv/MuonSpShowerContainerCnv_p1.h"

// DECLARE_TPCNV_FACTORY(MuonContainerCnvARA_p1,
//                       Analysis::MuonContainer,
//                       MuonContainer_p1,
//                       Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonContainerCnv_p2,
                      Analysis::MuonContainer,
                      MuonContainer_p2,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonContainerCnv_p3,
                      Analysis::MuonContainer,
                      MuonContainer_p3,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonContainerCnv_p4,
                      Analysis::MuonContainer,
                      MuonContainer_p4,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonContainerCnv_p5,
                      Analysis::MuonContainer,
                      MuonContainer_p5,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonContainerCnv_p6,
                      Analysis::MuonContainer,
                      MuonContainer_p6,
                      Athena::TPCnvVers::Current)
                      
DECLARE_TPCNV_FACTORY(MuonCaloEnergyContainerCnv_tlp1,
                      MuonCaloEnergyContainer,
                      MuonCaloEnergyContainer_tlp1,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(MuonCaloEnergyContainerCnv_p1,
                      MuonCaloEnergyContainer,
                      MuonCaloEnergyContainer_p1,
                      Athena::TPCnvVers::Current)
                      
DECLARE_TPCNV_FACTORY(MuonSpShowerContainerCnv_p1,
                      Rec::MuonSpShowerContainer,
                      MuonSpShowerContainer_p1,
                      Athena::TPCnvVers::Current)
                      
