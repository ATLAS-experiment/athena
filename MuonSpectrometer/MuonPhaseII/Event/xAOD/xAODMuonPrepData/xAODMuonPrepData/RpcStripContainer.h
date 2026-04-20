/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONPREPDATA_RPCSTRIPCONTAINER_H
#define XAODMUONPREPDATA_RPCSTRIPCONTAINER_H


#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"

namespace xAOD{
   using RpcStripContainer_v1 = DataVector<RpcStrip_v1>;
   using RpcStripContainer = RpcStripContainer_v1;
}

CLASS_DEF(xAOD::RpcStripContainer, 1274417297, 1)

#endif