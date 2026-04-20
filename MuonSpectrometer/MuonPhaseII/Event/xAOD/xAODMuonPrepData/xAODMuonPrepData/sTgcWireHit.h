/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCWIREHIT_H
#define XAODMUONPREPDATA_STGCWIREHIT_H

#include "xAODMuonPrepData/sTgcWireHitFwd.h"
#include "xAODMuonPrepData/versions/sTgcWireHit_v1.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

DATAVECTOR_BASE(xAOD::sTgcWireHit_v1, xAOD::sTgcMeasurement_v1);

// Set up a CLID for the class:
CLASS_DEF( xAOD::sTgcWireHit , 73038134 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
