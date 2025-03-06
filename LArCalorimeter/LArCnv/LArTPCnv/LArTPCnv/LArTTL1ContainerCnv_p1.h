/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARTTL1CONTAINERCNV_P1_H
#define LARTPCNV_LARTTL1CONTAINERCNV_P1_H

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// LArTPCnv includes
#include "LArTPCnv/LArTTL1Container_p1.h"
#include "LArTPCnv/LArTTL1Cnv_p1.h"

// LArRawEvent includes
#include "LArRawEvent/LArTTL1Container.h"

typedef T_AthenaPoolTPCnvVector<
            LArTTL1Container,
            LArTTL1Container_p1,
            LArTTL1Cnv_p1
       > LArTTL1ContainerCnv_p1;

#endif //> LARTPCNV_LARTTL1CONTAINERCNV_P1_H
