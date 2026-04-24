/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCPADHIT_H
#define XAODMUONPREPDATA_STGCPADHIT_H

#include "xAODMuonPrepData/sTgcPadHitFwd.h"
#include "xAODMuonPrepData/versions/sTgcPadHit_v1.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
DATAVECTOR_BASE(xAOD::sTgcPadHit_v1, xAOD::sTgcMeasurement_v1);
// Set up a CLID for the class:

CLASS_DEF( xAOD::sTgcPadHit , 28752257 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
