/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcRdoToRpcDigit.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
namespace {
    constexpr double inverseSpeedOfLight = 1 / Gaudi::Units::c_light; 
}

namespace Muon {
    StatusCode RpcRdoToRpcDigit::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());    
        ATH_CHECK(m_rpcDigitKey.initialize());
        if(!m_decodeLegacyRDO && !m_decodeNrpcRDO) {
            ATH_MSG_ERROR("Neither legacy or xAOD::Rpcs shall be translated.");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_rpcRdoKey.initialize(m_decodeLegacyRDO));
        ATH_CHECK(m_rpcRdoDecoderTool.retrieve(EnableTool{m_decodeLegacyRDO}));
        ATH_CHECK(m_rpcReadKey.initialize(m_decodeLegacyRDO));

        ATH_CHECK(m_nRpcRdoKey.initialize(m_decodeNrpcRDO));
        ATH_CHECK(m_nRpcCablingKey.initialize(m_decodeNrpcRDO));
        ATH_CHECK(m_DetectorManagerKey.initialize(m_decodeNrpcRDO));
    
        return StatusCode::SUCCESS;
    }
    StatusCode RpcRdoToRpcDigit::execute(const EventContext& ctx) const {
        ATH_MSG_DEBUG("in execute()");
        // retrieve the collection of RDO

        const size_t modHashMax{m_idHelperSvc->rpcIdHelper().module_hash_max()};
        TempDigitContainer tempOut(modHashMax);

        ATH_CHECK(decodeLegacyRdo(ctx, tempOut));
        ATH_CHECK(decodeNRpc(ctx, tempOut));

        SG::WriteHandle writeHandle{m_rpcDigitKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<RpcDigitContainer>(modHashMax)));

        for (size_t coll_hash = 0; coll_hash < modHashMax; ++coll_hash) {
            if (tempOut[coll_hash] && tempOut[coll_hash]->size()) {
                ATH_CHECK(writeHandle->addCollection(tempOut[coll_hash].release(), coll_hash));
            }
        }

        ATH_MSG_DEBUG("Decoding RPC RDO into RPC Digit");
        return StatusCode::SUCCESS;
    }

    StatusCode RpcRdoToRpcDigit::decodeLegacyRdo(const EventContext& ctx, TempDigitContainer& container) const {
        if (!m_decodeLegacyRDO) {
            ATH_MSG_VERBOSE("No legacy containers to be decoded ");
            return StatusCode::SUCCESS;
        }
        SG::ReadHandle rdoContainer(m_rpcRdoKey, ctx);
        ATH_CHECK(rdoContainer.isPresent());
        ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " RPC RDOs.");
        SG::ReadCondHandle cablingMap{m_rpcReadKey, ctx};
        ATH_CHECK(cablingMap.isValid());

        for (const RpcPad* rdoColl : *rdoContainer) {
            ATH_MSG_DEBUG(" Number of CMs in this Pad " << rdoColl->size());
            // Get pad online id and sector id
            uint16_t padId = rdoColl->onlineId();
            uint16_t sectorId = rdoColl->sector();
    
            /// For each pad, loop on the coincidence matrices
            for (const RpcCoinMatrix* coinMatrix : *rdoColl) {
                // Get CM online Id
                uint16_t cmaId = coinMatrix->onlineId();

                // For each CM, loop on the fired channels
                for (const RpcFiredChannel* rpcChan : *coinMatrix) {
                    std::vector<std::unique_ptr<RpcDigit>> digitVec{m_rpcRdoDecoderTool->getDigit(rpcChan, sectorId, 
                                                                                                  padId, cmaId, 
                                                                                                  cablingMap.cptr())};

                    if (digitVec.empty()) continue;

                    /// Loop on the digits corresponding to the fired channel
                    for (std::unique_ptr<RpcDigit>& newDigit : digitVec) {                    
                        const IdentifierHash collHash = m_idHelperSvc->moduleHash(newDigit->identify());
                        std::unique_ptr<RpcDigitCollection>& digitColl = container[collHash];
                        if (!digitColl) {
                            digitColl = std::make_unique<RpcDigitCollection>(m_idHelperSvc->chamberId(newDigit->identify()),
                                                                                                      collHash);
                        }
                        digitColl->push_back(std::move(newDigit));
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode RpcRdoToRpcDigit::decodeNRpc(const EventContext& ctx, TempDigitContainer& digit_map) const {
        if (!m_decodeNrpcRDO) {
            ATH_MSG_VERBOSE("NRPC rdo decoding has been switched off ");
            return StatusCode::SUCCESS;
        }
    
        SG::ReadHandle rdoContainer{m_nRpcRdoKey, ctx};
        ATH_CHECK(rdoContainer.isPresent());
        SG::ReadCondHandle cabling{m_nRpcCablingKey, ctx};
        if (!cabling.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_nRpcCablingKey.fullKey());
            return StatusCode::FAILURE;
        }
        SG::ReadCondHandle muonDetMgr{m_DetectorManagerKey,ctx};
        if (!muonDetMgr.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the readout geometry "<<muonDetMgr.fullKey());
            return StatusCode::FAILURE;
        }
        const RpcIdHelper& id_helper = m_idHelperSvc->rpcIdHelper();

        /// Loop over the container
        for (const xAOD::NRPCRDO* rdo : *rdoContainer) {
            ATH_MSG_VERBOSE("Convert RDO tdcSector: "<< static_cast<int>(rdo->tdcsector())<<", tdc:"
                    <<static_cast<int>(rdo->tdc())<<" channel: "<<static_cast<int>(rdo->channel()) <<", time: "
                    <<rdo->time()<<", ToT: "<<rdo->timeoverthr());
        
            /// Fill the cabling object
            RpcCablingData convObj{};
            convObj.subDetector = rdo->subdetector();
            convObj.tdcSector = rdo->tdcsector();
            convObj.tdc = rdo->tdc();
            convObj.channelId = rdo->channel();
        
            if (!cabling->getOfflineId(convObj, msgStream())) {
                ATH_MSG_FATAL("Failed to convert online -> offline");
                return StatusCode::FAILURE;
            }
            Identifier chanId{0};
            if (!cabling->convert(convObj, chanId)) {
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
        
            std::unique_ptr<RpcDigit> digit = std::make_unique<RpcDigit>(chanId, digit_time, ToT, convObj.stripSide());
            coll->push_back(std::move(digit));
        }
        return StatusCode::SUCCESS;
    }
}