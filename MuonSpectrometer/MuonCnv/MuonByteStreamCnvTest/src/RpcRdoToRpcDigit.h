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


class RpcRdoToRpcDigit : public AthReentrantAlgorithm {
public:
    RpcRdoToRpcDigit(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~RpcRdoToRpcDigit() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    struct TempDigitContainer{
        TempDigitContainer(RpcDigitContainer* container):
           m_cont{container} {}
        StatusCode findCollection(const Identifier& elementId,
                                  const IdentifierHash& hash, RpcDigitCollection* &coll, MsgStream& msg);

      private:
        RpcDigitContainer* m_cont{nullptr};
        RpcDigitCollection* m_lastColl{nullptr};
        std::map<IdentifierHash, RpcDigitCollection*> m_digitMap{};
    };
    StatusCode decodeRpc(const RpcPad*, TempDigitContainer& container, const RpcCablingCondData* rpcCab) const;
   
    StatusCode decodeNRpc(const EventContext& ctx, RpcDigitContainer& container) const;
    ToolHandle<Muon::IRPC_RDO_Decoder> m_rpcRdoDecoderTool{this, "rpcRdoDecoderTool", "Muon::RpcRDO_Decoder", ""};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadHandleKey<RpcPadContainer> m_rpcRdoKey{this, "RpcRdoContainer", "RPCPAD", "Rpc RDO Input"};
    SG::WriteHandleKey<RpcDigitContainer> m_rpcDigitKey{this, "RpcDigitContainer", "RPC_DIGITS", "Rpc Digit Output"};
    SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};


    Gaudi::Property<bool> m_decodeNrpcRDO{this, "DecodeNrpcRDO", false};
    Gaudi::Property<bool> m_patch_for_rpc_time{this, "PatchForRpcTime", false, "flag for patching the RPC time"};

    SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_nRpcRdoKey{this, "NRpcRdoContainer", "NRPCRDO", "BIS78 RPC Rdo input with ToTs"};
    SG::ReadCondHandleKey<Muon::RpcCablingMap> m_nRpcCablingKey{this, "NRpcCablingKey", "MuonNRPC_CablingMap", "Key of input MDT cabling map"};

    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

};

#endif
