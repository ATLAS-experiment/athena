/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RpcStripFWD_H
#define XAODMUONPREPDATA_RpcStripFWD_H
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/RpcMeasurementFwd.h"
/** @brief Forward declaration of the xAOD::RpcStrip */
namespace xAOD{
   class RpcMeasurement_v1;
   class RpcStrip_v1;
   using RpcStrip = RpcStrip_v1;

   class RpcStripAuxContainer_v1;
   using RpcStripAuxContainer = RpcStripAuxContainer_v1;
}
DATAVECTOR_BASE(xAOD::RpcStrip_v1, xAOD::RpcMeasurement_v1);

namespace xAOD{
   using RpcStripContainer_v1 = DataVector<RpcStrip_v1>;
   using RpcStripContainer = RpcStripContainer_v1;
}
#endif
