/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCMEASUREMENTCONTAINER_H
#define XAODMUONPREPDATA_RPCMEASUREMENTCONTAINER_H

#include "xAODMuonPrepData/RpcMeasurementFwd.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"

namespace xAOD{
   using RpcMeasurementContainer_v1 = DataVector<RpcMeasurement_v1>;
   using RpcMeasurementContainer = RpcMeasurementContainer_v1;
}
// Set up a CLID for the class:

CLASS_DEF( xAOD::RpcMeasurementContainer , 1183064885 , 1 )
#endif