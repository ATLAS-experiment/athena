/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCMEASUREMENTCONTAINER_H
#define XAODMUONPREPDATA_RPCMEASUREMENTCONTAINER_H

#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/versions/RpcMeasurementContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef RpcMeasurementContainer_v1 RpcMeasurementContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcMeasurementContainer , 1183064885 , 1 )
#endif