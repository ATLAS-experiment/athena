/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_STGCMEASUREMENT_H
#define XAODMUONPREPDATA_STGCMEASUREMENT_H

#include "xAODMuonPrepData/sTgcMeasurementFwd.h"
#include "xAODMuonPrepData/versions/sTgcMeasurement_v1.h"
#include "xAODMuonPrepData/MuonMeasurement.h"
DATAVECTOR_BASE(xAOD::sTgcMeasurement_v1, xAOD::MuonMeasurement_v1);
// Set up a CLID for the class:

CLASS_DEF( xAOD::sTgcMeasurement , 72401404 , 1 )

#endif  // XAODMUONPREPDATA_STGCSTRIP_H
