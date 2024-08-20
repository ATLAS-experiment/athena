/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RPCSTRIP_H
#define XAODMUONPREPDATA_RPCSTRIP_H

#include "xAODMuonPrepData/RpcStripFwd.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/versions/RpcStrip_v1.h"

DATAVECTOR_BASE(xAOD::RpcStrip_v1, xAOD::RpcMeasurement_v1);
// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF(xAOD::RpcStrip, 47111859, 1)

#endif