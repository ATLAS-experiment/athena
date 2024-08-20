/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIPCONTAINER_H

#include "xAODMuonPrepData/RpcStripFwd.h"
#include "xAODMuonPrepData/RpcStrip.h"


namespace xAOD{
   using RpcStripContainer_v1 = DataVector<RpcStrip_v1>;
   using RpcStripContainer = RpcStripContainer_v1;
}
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::RpcStripContainer, 1274417297, 1)

#endif