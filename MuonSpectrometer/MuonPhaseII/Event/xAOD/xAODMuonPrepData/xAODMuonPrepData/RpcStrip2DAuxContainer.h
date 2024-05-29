/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCSTRIP2DAUXCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIP2DAUXCONTAINER_H

#include "xAODMuonPrepData/versions/RpcStrip2DAuxContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the RpcStrip
typedef RpcStrip2DAuxContainer_v1 RpcStrip2DAuxContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcStrip2DAuxContainer , 1173306900 , 1 )
#endif  // XAODMUONRDO_NRPCRDO_H
