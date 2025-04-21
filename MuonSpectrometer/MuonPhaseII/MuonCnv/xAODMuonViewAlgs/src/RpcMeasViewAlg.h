
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONMEASVIEWALGS_RPCMEASVIEWALG_H
#define XAODMUONMEASVIEWALGS_RPCMEASVIEWALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <xAODMuonPrepData/RpcStripContainer.h>
#include <xAODMuonPrepData/RpcStrip2DContainer.h>
#include <xAODMuonPrepData/RpcMeasurementContainer.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>


/**
 * @brief: The RpcMeasViewAlg takes the BI & legacy Rpc measurements and pushes
 *         them into a common RpcMeasurmentContainer which is a SG::VIEW_ELEMENTS container
 * 
*/

namespace MuonR4 {
    class RpcMeasViewAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            ~RpcMeasViewAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        private:
            SG::ReadHandleKey<xAOD::RpcStripContainer> m_readKey1D{this, "Strip1DKey", "xRpcStrips", 
                                                                       "Name of the xAOD::RpcStripContainer"};

            SG::ReadHandleKey<xAOD::RpcStrip2DContainer> m_readKeyBI{this, "Strip2DKey", "xRpcBILStrips", 
                                                                     "Name of the xAOD::RpcStrip2DContainer"};

            SG::WriteHandleKey<xAOD::RpcMeasurementContainer> m_writeKey{this, "WriteKey", "xRpcMeasurements"};
    };
}

#endif