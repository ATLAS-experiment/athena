/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_MDTMDRIFTCIRCLE_H
#define XAODMUONPREPDATA_MDTMDRIFTCIRCLE_H

#include "xAODMuonPrepData/MdtDriftCircleFwd.h"
#include "xAODMuonPrepData/MuonMeasurement.h"
#include "xAODMuonPrepData/versions/MdtDriftCircle_v1.h"

// Define inheritance pattern
DATAVECTOR_BASE(xAOD::MdtDriftCircle_v1, xAOD::MuonMeasurement_v1);
// Set up a CLID for the class:
CLASS_DEF(xAOD::MdtDriftCircle, 92538182, 1)
#endif  