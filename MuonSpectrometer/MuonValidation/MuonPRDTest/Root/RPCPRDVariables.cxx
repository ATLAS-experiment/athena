/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/RPCPRDVariables.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
namespace MuonPRDTest {
    RPCPRDVariables::RPCPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "PRD_RPC", msglvl), m_key{container_name} {}
    bool RPCPRDVariables::declare_keys() { return declare_dependency(m_key); }

    bool RPCPRDVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillRPCPRDVariables()");
        
        SG::ReadHandle<Muon::RpcPrepDataContainer> rpcprdContainer{m_key, ctx};
        if (!rpcprdContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve prd container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved RPC PRD Container with size " << rpcprdContainer->size());

        unsigned int n_PRD{0};
        for(const Muon::RpcPrepDataCollection* coll : *rpcprdContainer ) {
            for (const Muon::RpcPrepData* prd: *coll) {

                m_RPC_PRD_id.push_back(prd->identify());
                m_RPC_PRD_globalPos.push_back(prd->globalPosition());
                m_RPC_PRD_locX.push_back(prd->localPosition().x());     
                m_RPC_PRD_error.push_back(Amg::error(prd->localCovariance(), Trk::locX));
                m_RPC_PRD_time.push_back(prd->time());
                ++n_PRD;
            }
        }
        m_RPC_nPRD = n_PRD;
        ATH_MSG_DEBUG(" finished fillRPCPRDVariables()");
        return true;
    }
}