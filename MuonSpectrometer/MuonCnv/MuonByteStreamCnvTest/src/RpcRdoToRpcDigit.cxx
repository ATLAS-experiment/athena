/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcRdoToRpcDigit.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
namespace {
    constexpr double inverseSpeedOfLight = 1 / Gaudi::Units::c_light; 
}

StatusCode RpcRdoToRpcDigit::TempDigitContainer::findCollection(const Identifier& elementId,
                                                                const IdentifierHash& hash, RpcDigitCollection* &coll, MsgStream& msg) {
    if (m_lastColl && m_lastColl->identifierHash() == hash) {
        coll = m_lastColl;
        return StatusCode::SUCCESS;
    }
    m_lastColl = m_digitMap[hash];
    if (m_lastColl) {
        coll = m_lastColl;
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<RpcDigitCollection> new_coll = std::make_unique<RpcDigitCollection>(elementId, hash);
    coll = new_coll.get();
    RpcDigitContainer::IDC_WriteHandle lock = m_cont->getWriteHandle(hash);
    if(lock.addOrDelete(std::move(new_coll)).isFailure()){
        msg<<MSG::ERROR<<" Failed to add digit collection "<<elementId<<endmsg;
        coll = nullptr;
        return StatusCode::FAILURE;
    }    
    m_lastColl = m_digitMap[hash] = coll;
    return StatusCode::SUCCESS;
}
RpcRdoToRpcDigit::RpcRdoToRpcDigit(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode RpcRdoToRpcDigit::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_rpcRdoKey.initialize());
    ATH_CHECK(m_rpcDigitKey.initialize());
    ATH_CHECK(m_rpcRdoDecoderTool.retrieve());
    ATH_CHECK(m_rpcReadKey.initialize());

    ATH_CHECK(m_nRpcRdoKey.initialize(m_decodeNrpcRDO));
    ATH_CHECK(m_nRpcCablingKey.initialize(m_decodeNrpcRDO));
    ATH_CHECK(m_DetectorManagerKey.initialize(m_decodeNrpcRDO));
  
    return StatusCode::SUCCESS;
}

StatusCode RpcRdoToRpcDigit::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
    // retrieve the collection of RDO
    SG::ReadHandle<RpcPadContainer> rdoRH(m_rpcRdoKey, ctx);
    if (!rdoRH.isValid()) {
        ATH_MSG_WARNING("No RPC RDO container found!");
        return StatusCode::SUCCESS;
    }
    const RpcPadContainer* rdoContainer = rdoRH.cptr();
    ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " RPC RDOs.");

    SG::WriteHandle<RpcDigitContainer> wh_rpcDigit(m_rpcDigitKey, ctx);
    ATH_CHECK(wh_rpcDigit.record(std::make_unique<RpcDigitContainer>(m_idHelperSvc->rpcIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Decoding RPC RDO into RPC Digit");

    
    SG::ReadCondHandle<RpcCablingCondData> cablingCondData{m_rpcReadKey, ctx};
    TempDigitContainer temp_out{wh_rpcDigit.ptr()};
    for (const RpcPad* rpcPad : *rdoContainer) {
        if (!rpcPad->empty()) { ATH_CHECK(decodeRpc(rpcPad, temp_out, cablingCondData.cptr())); }
    }
    ATH_CHECK(decodeNRpc(ctx, *wh_rpcDigit));
   
    return StatusCode::SUCCESS;
}

StatusCode RpcRdoToRpcDigit::decodeRpc(const RpcPad* rdoColl, TempDigitContainer& container, const RpcCablingCondData* rpcCab) const {
   
    ATH_MSG_DEBUG(" Number of CMs in this Pad " << rdoColl->size());
    // Get pad online id and sector id
    uint16_t padId = rdoColl->onlineId();
    uint16_t sectorId = rdoColl->sector();
    
    // For each pad, loop on the coincidence matrices
    for (const RpcCoinMatrix* coinMatrix : *rdoColl) {
        // Get CM online Id
        uint16_t cmaId = coinMatrix->onlineId();

        // For each CM, loop on the fired channels
        for (const RpcFiredChannel* rpcChan : *coinMatrix) {
            std::vector<std::unique_ptr<RpcDigit>> digitVec{m_rpcRdoDecoderTool->getDigit(rpcChan, 
                                                                        sectorId, 
                                                                        padId, 
                                                                        cmaId, 
                                                                        rpcCab)};
           
            if (digitVec.empty()) continue;
            
            
            // Loop on the digits corresponding to the fired channel
            for (std::unique_ptr<RpcDigit>& newDigit : digitVec) {
                Identifier elementId = m_idHelperSvc->rpcIdHelper().elementID(newDigit->identify());
                IdentifierHash coll_hash{0};
                if (m_idHelperSvc->rpcIdHelper().get_module_hash(elementId, coll_hash)) {
                    ATH_MSG_ERROR("Unable to get RPC digit collection hash id the identifier is "
                                    <<m_idHelperSvc->toString(newDigit->identify()));
                    return StatusCode::FAILURE;
                }
                RpcDigitCollection* collection{nullptr};
                ATH_CHECK(container.findCollection(elementId,coll_hash,collection, msgStream()));
                collection->push_back(std::move(newDigit));
            }          
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode RpcRdoToRpcDigit::decodeNRpc(const EventContext& ctx, RpcDigitContainer& out_container) const {
    if (!m_decodeNrpcRDO) {
        ATH_MSG_VERBOSE("NRPC rdo decoding has been switched off ");
        return StatusCode::SUCCESS;
    }
    
    SG::ReadHandle<xAOD::NRPCRDOContainer> rdoContainer{m_nRpcRdoKey, ctx};
    if (!rdoContainer.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve "<<m_nRpcRdoKey.fullKey());
        return StatusCode::FAILURE;
    }
    SG::ReadCondHandle cabling{m_nRpcCablingKey, ctx};
    if (!cabling.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve "<<m_nRpcCablingKey.fullKey());
        return StatusCode::FAILURE;
    }
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muonDetMgr{m_DetectorManagerKey,ctx};
    if (!muonDetMgr.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the readout geometry "<<muonDetMgr.fullKey());
        return StatusCode::FAILURE;
    }
    const RpcIdHelper& id_helper = m_idHelperSvc->rpcIdHelper();
    std::map<IdentifierHash, std::unique_ptr<RpcDigitCollection>> digit_map{};
    /// Loop over the container
    for (const xAOD::NRPCRDO* rdo : *rdoContainer) {
        ATH_MSG_VERBOSE("Convert RDO boardSector: "<< static_cast<int>(rdo->boardsector())<<", tdc:"
                <<static_cast<int>(rdo->board())<<" channel: "<<static_cast<int>(rdo->channel()) <<", time: "<<
                rdo->time()<<", ToT: "<<rdo->timeoverthr());
        
        /// Fill the cabling object
        Muon::RpcCablingData conv_obj{};
        conv_obj.subDetector = rdo->subdetector();
        conv_obj.boardSector = rdo->boardsector();
        conv_obj.board = rdo->board();
        conv_obj.channelId = rdo->channel();
        
        if (!cabling->getOfflineId(conv_obj, msgStream())) {
            ATH_MSG_FATAL("Failed to convert online -> offline");
            return StatusCode::FAILURE;
        }
        Identifier chanId{0};
        if (!cabling->convert(conv_obj, chanId)) {
            return StatusCode::FAILURE;
        }
        /// Find the proper Digit collection
        IdentifierHash modHash = m_idHelperSvc->moduleHash(chanId);
        std::unique_ptr<RpcDigitCollection>& coll = digit_map[modHash];
        /// The collection has not been made thus far
        if (!coll) {
            coll = std::make_unique<RpcDigitCollection>(id_helper.elementID(chanId), modHash);           
        }
        /// We can fill the digit
        /// Need to add the correction
        const float digit_time = rdo->time();
        const float ToT = m_patch_for_rpc_time ? rdo->timeoverthr() 
                                            + inverseSpeedOfLight * (muonDetMgr->getRpcReadoutElement(chanId)->stripPos(chanId)).mag() : rdo->timeoverthr() ;
        
        std::unique_ptr<RpcDigit> digit = std::make_unique<RpcDigit>(chanId, digit_time, ToT);
        coll->push_back(std::move(digit));
    }
    
    for (auto& [hash, coll] : digit_map){
        if (coll->empty()) continue;
        RpcDigitContainer::IDC_WriteHandle lock = out_container.getWriteHandle(hash);
        ATH_CHECK(lock.addOrDelete(std::move(coll)));      
    }
    return StatusCode::SUCCESS;
}
