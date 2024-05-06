/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCSTRIP2DCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIP2DCONTAINER_H

#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/versions/RpcStrip2DContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
/// Define the version of the pixel cluster container
typedef RpcStrip2DContainer_v1 RpcStrip2DContainer;
}  // namespace xAOD

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcStrip2DContainer , 1233075333 , 1 )

#endif