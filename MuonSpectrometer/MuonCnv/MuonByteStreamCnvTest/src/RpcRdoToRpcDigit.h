/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAMCNVTEST_RPCRDOTORPCDIGIT_H
#define MUONBYTESTREAMCNVTEST_RPCRDOTORPCDIGIT_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

namespace Muon {
    class RpcRdoToRpcDigit : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~RpcRdoToRpcDigit() = default;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext &ctx) const override final;

        private:
            using TempDigitContainer = std::vector<std::unique_ptr<RpcDigitCollection>>;
            /** @brief: Decode the legacy RpcRdo format*/
            StatusCode decodeLegacyRdo(const EventContext &ctx, TempDigitContainer &container) const;

            StatusCode decodeNRpc(const EventContext &ctx, TempDigitContainer &container) const;
            ToolHandle<IRPC_RDO_Decoder> m_rpcRdoDecoderTool{this, "rpcRdoDecoderTool", "RpcRDO_Decoder", ""};
            ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::ReadHandleKey<RpcPadContainer> m_rpcRdoKey{this, "RpcRdoContainer", "RPCPAD", "Rpc RDO Input"};
            SG::WriteHandleKey<RpcDigitContainer> m_rpcDigitKey{this, "RpcDigitContainer", "RPC_DIGITS", "Rpc Digit Output"};
            SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};

            Gaudi::Property<bool> m_decodeLegacyRDO{this, "DecodeLegacyRDO", true};
            Gaudi::Property<bool> m_decodeNrpcRDO{this, "DecodeNrpcRDO", false};
            Gaudi::Property<bool> m_patch_for_rpc_time{this, "PatchForRpcTime", false, "flag for patching the RPC time"};

            SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_nRpcRdoKey{this, "NRpcRdoContainer", "NRPCRDO", "BIS78 RPC Rdo input with ToTs"};
            SG::ReadCondHandleKey<RpcCablingMap> m_nRpcCablingKey{this, "NRpcCablingKey", "MuonNRPC_CablingMap", "Key of input MDT cabling map"};

            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                "Key of input MuonDetectorManager condition data"};
    };
}

#endif
