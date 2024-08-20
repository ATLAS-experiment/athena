/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCMEASUREMENT_H
#define XAODMUONPREPDATA_RPCMEASUREMENT_H

#include "xAODMuonPrepData/RpcMeasurementFwd.h"
#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"
#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(xAOD::RpcMeasurement_v1, xAOD::UncalibratedMeasurement_v1);

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcMeasurement , 47915827 , 1 )
#endif  // XAODMUONPREPDATA_RPCSTRIP_H
