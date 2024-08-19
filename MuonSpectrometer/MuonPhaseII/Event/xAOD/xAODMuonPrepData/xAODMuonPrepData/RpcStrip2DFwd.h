/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_RpcStrip2DFWD_H
#define XAODMUONPREPDATA_RpcStrip2DFWD_H
#include "AthContainers/DataVector.h"
#include "xAODMuonPrepData/RpcMeasurementFwd.h"
/** @brief Forward declaration of the xAOD::RpcStrip2D */
namespace xAOD{
   class RpcMeasurement_v1;
   class RpcStrip2D_v1;
   using RpcStrip2D = RpcStrip2D_v1;

   class RpcStrip2DAuxContainer_v1;
   using RpcStrip2DAuxContainer = RpcStrip2DAuxContainer_v1;
}
DATAVECTOR_BASE(xAOD::RpcStrip2D_v1, xAOD::RpcMeasurement_v1);

namespace xAOD{
   using RpcStrip2DContainer_v1 = DataVector<RpcStrip2D_v1>;
   using RpcStrip2DContainer = RpcStrip2DContainer_v1;
}
#endif
