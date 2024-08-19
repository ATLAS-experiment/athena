/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_RPCSTRIP_V1_H

#include "xAODMuonPrepData/versions/RpcMeasurement_v1.h"

namespace xAOD {

class RpcStrip_v1 : public RpcMeasurement_v1 {

   public:
        /// Default constructor
        RpcStrip_v1() = default;
        /// Virtual destructor
        virtual ~RpcStrip_v1() = default;

        unsigned int numDimensions() const override final { return 1; }
        
        /** @brief returns whether the hit measures the phi coordinate */
        uint8_t measuresPhi() const override final;
        /** @brief sets the measuresPhi value */
        void setMeasuresPhi(uint8_t measPhi);

};

}  // namespace xAOD

#endif
