
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
            RpcMeasViewAlg(const std::string& name, ISvcLocator* pSvcLocator);

            ~RpcMeasViewAlg() = default;

            StatusCode execute(const EventContext& ctx) const override;
            StatusCode initialize() override;
        private:
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;


            SG::ReadHandleKey<xAOD::RpcStripContainer> m_readKey1D{this, "Strip1DKey", "xRpcStrips", 
                                                                       "Name of the xAOD::RpcStripContainer"};

            SG::ReadHandleKey<xAOD::RpcStrip2DContainer> m_readKeyBI{this, "WireKey", "xRpcBILStrips", 
                                                                     "Name of the xAOD::RpcStrip2DContainer"};

            SG::WriteHandleKey<xAOD::RpcMeasurementContainer> m_writeKey{this, "WriteKey", "xRpcMeasurements"};
    };
}

#endif