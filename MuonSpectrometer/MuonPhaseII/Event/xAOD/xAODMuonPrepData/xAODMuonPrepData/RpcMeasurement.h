/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCMEASUREMENT_H
#define XAODMUONPREPDATA_RPCMEASUREMENT_H

#include "xAODMuonPrepData/RpcMeasurementFwd.h"
#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"
#include "xAODMuonPrepData/MuonMeasurement.h"

DATAVECTOR_BASE(xAOD::RpcMeasurement_v1, xAOD::MuonMeasurement_v1);

// Set up a CLID for the class:
CLASS_DEF( xAOD::RpcMeasurement , 47915827 , 1 )
#endif  // XAODMUONPREPDATA_RPCSTRIP_H
