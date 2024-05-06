/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCMEASUREMENT_H
#define XAODMUONPREPDATA_RPCMEASUREMENT_H

#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the RpcStrip
typedef RpcMeasurement_v1 RpcMeasurement;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcMeasurement , 47915827 , 1 )

#endif  // XAODMUONPREPDATA_RPCSTRIP_H
