/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCSTRIP2D_H
#define XAODMUONPREPDATA_RPCSTRIP2D_H

#include "xAODMuonPrepData/versions/RpcStrip2D_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Defined the version of the RpcStrip
typedef RpcStrip2D_v1 RpcStrip2D;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcStrip2D , 223619931 , 1 )

#endif  // XAODMUONPREPDATA_RPCSTRIP_H
