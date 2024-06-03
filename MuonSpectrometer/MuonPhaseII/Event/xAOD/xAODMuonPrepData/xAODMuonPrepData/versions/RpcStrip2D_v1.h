/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCSTRIP2D_V1_H
#define XAODMUONPREPDATA_VERSION_RPCSTRIP2D_V1_H

#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"

namespace xAOD {

class RpcStrip2D_v1 : public RpcMeasurement_v1 {

   public:
        /// Default constructor
        RpcStrip2D_v1() = default;
        /// Virtual RpcStrip2D_v1
        virtual ~RpcStrip2D_v1() = default;

        unsigned int numDimensions() const override final { return 2; }
        
        /** @brief returns whether the hit measures the phi coordinate */
        uint8_t measuresPhi() const override final;
     

};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::RpcStrip2D_v1, xAOD::RpcMeasurement_v1);
#endif
