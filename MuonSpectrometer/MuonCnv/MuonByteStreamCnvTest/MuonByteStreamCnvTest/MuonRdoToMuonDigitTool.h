/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDOTOMUONDIGITTOOL_H
#define MUONRDOTOMUONDIGITTOOL_H

#include <unordered_map>

#include "AthenaBaseComps/AthAlgTool.h"
#include "CscCalibTools/ICscCalibTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCSC_CnvTools/ICSC_RDO_Decoder.h"
#include "MuonDigToolInterfaces/IMuonDigitizationTool.h"
#include "MuonDigitContainer/CscDigitContainer.h"
#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"
#include "MuonMM_CnvTools/IMM_RDO_Decoder.h"
#include "MuonRDO/CscRawDataContainer.h"
#include "MuonRDO/MM_RawDataContainer.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRDO/STGC_RawDataContainer.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"
#include "MuonSTGC_CnvTools/ISTGC_RDO_Decoder.h"
#include "MuonTGC_CnvTools/ITGC_RDO_Decoder.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TGCcablingInterface/ITGCcablingSvc.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

class MdtDigitContainer;
class CscDigitContainer;
class RpcDigitContainer;
class TgcDigitContainer;
class sTgcDigitContainer;
class MmDigitContainer;

class MdtDigitCollection;
class CscDigitCollection;
class RpcDigitCollection;
class TgcDigitCollection;
class sTgcDigitCollection;
class MmDigitCollection;

class CscRawDataCollection;
class STGC_RawDataCollection;
class STGC_RawDataContainer;
class STGC_RawData;
class MM_RawDataCollection;
class MM_RawDataContainer;
class MM_RawData;

// Author: Ketevi A. Assamagan
// BNL, January 24, 2004

// algorithm to decode RDO into digits
// get the RDO container from Storegate
// loop over the RDO
// Decode RDO into digits
// loop over the digits and build the digit container
// store the digit container in StoreGate

class MuonRdoToMuonDigitTool : virtual public IMuonDigitizationTool, public AthAlgTool {
public:
    MuonRdoToMuonDigitTool(const std::string& type, const std::string& name, const IInterface* pIID);
    ~MuonRdoToMuonDigitTool() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode digitize(const EventContext& ctx) override;

private:
    // private method for the decoding RDO --> digits
    using MdtDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<MdtDigitCollection> >;
    StatusCode decodeMdtRDO(const EventContext& ctx, MdtDigitContainer*) const;
    StatusCode decodeMdt(const MdtCsm& rdoColl, MdtDigitMap_t& mdtDigitVec) const;

    using CscDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<CscDigitCollection> >;
    StatusCode decodeCscRDO(const EventContext& ctx, CscDigitContainer*) const;
    StatusCode decodeCsc(const CscRawDataCollection& rdoColl, CscDigitMap_t& cscDigitMap) const;

    using RpcDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<RpcDigitCollection> >;
    StatusCode decodeRpcRDO(const EventContext& ctx, RpcDigitContainer*) const;
    
    StatusCode decodeNRpcRDO(const EventContext& ctx, RpcDigitContainer* container ) const;

    StatusCode decodeRpc(const RpcPad& rpcColl, RpcDigitMap_t& rpcDigitMap, const RpcCablingCondData* rpcCab) const;

    using TgcDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<TgcDigitCollection> >;
    StatusCode decodeTgcRDO(const EventContext& ctx, TgcDigitContainer*) const;
    StatusCode decodeTgc(const TgcRdo& rdoColl, TgcDigitMap_t& tgcDigitMap) const;

    using sTgcDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<sTgcDigitCollection> >;
    StatusCode decodeSTGC_RDO(const EventContext& ctx, sTgcDigitContainer*) const;
    StatusCode decodeSTGC(const Muon::STGC_RawDataCollection& rdoColl, sTgcDigitMap_t& stgcDigitMap) const;

    using MmDigitMap_t = std::unordered_map<IdentifierHash, std::unique_ptr<MmDigitCollection> >;
    StatusCode decodeMM_RDO(const EventContext& ctx, MmDigitContainer*) const;
    StatusCode decodeMM(const EventContext& ctx, const Muon::MM_RawDataCollection& rdoColl, MmDigitMap_t& mmDigitMap) const;

    StatusCode getTgcCabling();

private:
   

    /// Decoder tools
    ToolHandle<ICscCalibTool> m_cscCalibTool{this, "cscCalibTool", "CscCalibTool"};
    ToolHandle<Muon::IMDT_RDO_Decoder> m_mdtRdoDecoderTool{this, "mdtRdoDecoderTool", "Muon::MdtRDO_Decoder"};
    ToolHandle<Muon::ICSC_RDO_Decoder> m_cscRdoDecoderTool{this, "cscRdoDecoderTool", "Muon::CscRDO_Decoder"};
    ToolHandle<Muon::IRPC_RDO_Decoder> m_rpcRdoDecoderTool{this, "rpcRdoDecoderTool", "Muon::RpcRDO_Decoder"};
    ToolHandle<Muon::ITGC_RDO_Decoder> m_tgcRdoDecoderTool{this, "tgcRdoDecoderTool", "Muon::TgcRDO_Decoder"};
    ToolHandle<Muon::ISTGC_RDO_Decoder> m_stgcRdoDecoderTool{this, "stgcRdoDecoderTool", "Muon::STGC_RDO_Decoder"};
    ToolHandle<Muon::IMM_RDO_Decoder> m_mmRdoDecoderTool{this, "mmRdoDecoderTool", "Muon::MM_RDO_Decoder"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   
    // cabling service
    const ITGCcablingSvc* m_tgcCabling{nullptr};
 
    // algorithm properties
    Gaudi::Property<bool> m_decodeMdtRDO{this, "DecodeMdtRDO", true};
    Gaudi::Property<bool> m_decodeCscRDO{this, "DecodeCscRDO", true};
    Gaudi::Property<bool> m_decodeRpcRDO{this, "DecodeRpcRDO", true};
    Gaudi::Property<bool> m_decodeNrpcRDO{this, "DecodeNrpcRDO", false};
    
    Gaudi::Property<bool> m_decodeTgcRDO{this, "DecodeTgcRDO", true};
    Gaudi::Property<bool> m_decodesTgcRDO{this, "DecodeSTGC_RDO", true};
    Gaudi::Property<bool> m_decodeMmRDO{this, "DecodeMM_RDO", true};

    /** Switch for warning message disabling on one invalid channel in
        TGC sector A09 seen in 2008 data, at least run 79772 - 91800.
        bug #48828: TgcRdoToTgcDigit WARNING ElementID not found for
        sub=103 rod=9 ssw=6 slb=20 bitpos=151 +offset=0 orFlag=0
    */
    bool m_show_warning_level_invalid_TGC_A09_SSW6_hit;

    /** Flag to distinguish 12-fold TGC cabling and 8-fold TGC cabling */
    bool m_is12foldTgc;

    SG::ReadHandleKey<MdtCsmContainer> m_mdtRdoKey{this, "MdtRdoContainer", "MDTCSM", "Mdt RDO Input"};
    SG::WriteHandleKey<MdtDigitContainer> m_mdtDigitKey{this, "MdtDigitContainer", "MDT_DIGITS", "Mdt Digit Output"};
    
    SG::ReadHandleKey<CscRawDataContainer> m_cscRdoKey{this, "CscRdoContainer", "CSCRDO", "Csc RDO Input"};
    SG::WriteHandleKey<CscDigitContainer> m_cscDigitKey{this, "CscDigitContainer", "CSC_DIGITS", "Csc Digit Output"};
    
    /// Legacy RPC rdo container + cabling
    SG::ReadHandleKey<RpcPadContainer> m_rpcRdoKey{this, "RpcRdoContainer", "RPCPAD", "Rpc RDO Input"};
    SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};

    /// New BIS78 RDO container
    SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_nRpcRdoKey{this, "NRpcRdoContainer", "NRPCRDO", "BIS78 RPC Rdo input with ToTs"};
    SG::ReadCondHandleKey<Muon::RpcCablingMap> m_nRpcCablingKey{this, "NRpcCablingKey", "MuonNRPC_CablingMap", "Key of input MDT cabling map"};
    
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

    BooleanProperty m_patch_for_rpc_time{this, "PatchForRpcTime", false, "flag for patching the RPC time"};
    SG::WriteHandleKey<RpcDigitContainer> m_rpcDigitKey{this, "RpcDigitContainer", "RPC_DIGITS", "Rpc Digit Output"};
    

    
    
    SG::ReadHandleKey<TgcRdoContainer> m_tgcRdoKey{this, "TgcRdoContainer", "TGCRDO", "Tgc RDO Input"};
    SG::WriteHandleKey<TgcDigitContainer> m_tgcDigitKey{this, "TgcDigitContainer", "TGC_DIGITS", "Tgc Digit Output"};
    
    SG::ReadHandleKey<Muon::STGC_RawDataContainer> m_stgcRdoKey{this, "sTgcRdoContainer", "sTGCRDO", "sTgc RDO Input"};
    SG::WriteHandleKey<sTgcDigitContainer> m_stgcDigitKey{this, "sTgcDigitContainer", "sTGC_DIGITS", "sTgc Digit Output"};
    
    SG::ReadHandleKey<Muon::MM_RawDataContainer> m_mmRdoKey{this, "MmRdoContainer", "MMRDO", "MM RDO Input"};
    SG::WriteHandleKey<MmDigitContainer> m_mmDigitKey{this, "MmDigitContainer", "MM_DIGITS", "MM Digit Output"};
};

#endif
