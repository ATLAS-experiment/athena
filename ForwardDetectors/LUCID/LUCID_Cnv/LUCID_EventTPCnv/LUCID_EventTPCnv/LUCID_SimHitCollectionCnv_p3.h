/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_SimHitCollection_p3.h"
#include "LUCID_SimEvent/LUCID_SimHitCollection.h"
#include "LUCID_SimHitCnv_p3.h"
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

typedef T_AtlasHitsVectorCnv< LUCID_SimHitCollection, LUCID_SimHitCollection_p3, LUCID_SimHitCnv_p3 >  LUCID_SimHitCollectionCnv_p3;
