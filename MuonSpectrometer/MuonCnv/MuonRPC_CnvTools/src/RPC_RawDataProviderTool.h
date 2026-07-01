/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRPCRAWDATAPROVIDERTOOL_H
#define MUONRPCRAWDATAPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/RpcPad_Cache.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRDO/RpcSectorLogicContainer.h"
#include "MuonRPC_CnvTools/IRpcROD_Decoder.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace Muon {

    class RPC_RawDataProviderTool : public extends<AthAlgTool, IMuonRawDataProviderTool> {
    public:
        using base_class::base_class;
     
        virtual ~RPC_RawDataProviderTool() = default;

        virtual StatusCode initialize() override;

        /** Decoding method - IMuonRawDataProviderTool interface (EventContext-based) */
        
        virtual StatusCode convert(const EventContext&) const override;
        virtual StatusCode convert(const std::vector<IdentifierHash>&, const EventContext&) const override;
        virtual StatusCode convert(const std::vector<uint32_t>&, const EventContext&) const override;

    private:
        StatusCode convertIntoContainer(const ROBFragmentList& vecRobs, 
                                        const std::vector<IdentifierHash>& collections,
                                        const EventContext& ctx) const;
        // This function does all the actual work of decoding the data
        StatusCode convertIntoContainers(const ROBFragmentList& vecRobs,
                                         const std::vector<IdentifierHash>& collections, RpcPadContainer* pad,
                                         RpcSectorLogicContainer* logic, const bool& decodeSL,
                                         const EventContext& ctx) const;

        std::vector<IdentifierHash> to_be_converted(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                                    const std::vector<IdentifierHash>& coll) const;

    private:
        SG::ReadCondHandleKey<RpcCablingCondData> m_readKey{this, "ReadKey", "RpcCablingCondData", "Key of RpcCablingCondData"};

        // Rob Data Provider handle
        ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this, "ROBDataProviderSvc", "ROBDataProviderSvc"};

        // ROD decoding tool
        ToolHandle<IRpcROD_Decoder> m_decoder{this, "Decoder", "Muon::RpcROD_Decoder/RpcROD_Decoder"};

        // WriteHandleKey for RPC PAD container
        SG::WriteHandleKey<RpcPadContainer> m_containerKey{this, "RdoLocation", "RPCPAD", "Name of the RPCPAD produced by RawDataProvider"};
        // WriteHandleKey for RPC sector logic container
        SG::WriteHandleKey<RpcSectorLogicContainer> m_sec{this, "RPCSec", "RPC_SECTORLOGIC",
                                                          "Name of the RPC_SECTORLOGIC produced by RawDataProvider"};

        /// RPC container cache key
        SG::UpdateHandleKey<RpcPad_Cache> m_rdoContainerCacheKey{this, "RpcContainerCacheKey", 
                                                "", "Optional external cache for the RPC container"};
        /// Turn on/off RpcSectorConfig writing
        Gaudi::Property<bool> m_WriteOutRpcSectorLogic{this, "WriteOutRpcSectorLogic", true, "Turn on/off RpcSectorLogic writing"};
   
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    };

}  // namespace Muon

#endif
