/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCSTRIP2DCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIP2DCONTAINER_H

#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/RpcMeasurementContainer.h"

namespace xAOD{
   using RpcStrip2DContainer_v1 = DataVector<RpcStrip2D_v1>;
   using RpcStrip2DContainer = RpcStrip2DContainer_v1;
}
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::RpcStrip2DContainer , 1233075333 , 1 )

#endif