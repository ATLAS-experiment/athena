/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RPC_RawDataProviderTool.h"

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonRPC_CnvTools/IRpcROD_Decoder.h"
#include "eformat/SourceIdentifier.h"

using eformat::helper::SourceIdentifier;
using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

namespace Muon{
StatusCode RPC_RawDataProviderTool::initialize() {
    // retrieve ROD decoder
    ATH_CHECK(m_decoder.retrieve());

    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_sec.initialize());

    // get ROB data provider service
    ATH_CHECK(m_robDataProvider.retrieve());

    ATH_CHECK(m_readKey.initialize());

    // Additional initialization from MT class
    ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));
    ATH_CHECK(m_idHelperSvc.retrieve());

    // We should only turn off the sector logic when running with cached data a la trigger mode
    if (!m_rdoContainerCacheKey.key().empty() && m_WriteOutRpcSectorLogic) {
        ATH_MSG_FATAL(
            "Cannot write out RpcSectorLogic while running with cached RpcPad containers"
            " as the RpcSectorLogic is not cached at the same time and the written containers will desync."
            " Please turn off RpcSectorLogic writing when running with cached bytestream container");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("initialize() successful in " << name());

    return StatusCode::SUCCESS;
}

StatusCode RPC_RawDataProviderTool::convertIntoContainers(
    const ROBFragmentList& vecRobs, const std::vector<IdentifierHash>& collections,
    RpcPadContainer* pad, RpcSectorLogicContainer* logic, const bool& decodeSL) const {
    for (auto itFrag = vecRobs.begin(); itFrag != vecRobs.end(); itFrag++) {
        // convert only if data payload is delivered
        if ((**itFrag).rod_ndata() != 0) {
            std::vector<IdentifierHash> coll = to_be_converted(**itFrag, collections);

            if (m_decoder->fillCollections(**itFrag, *pad, coll, logic, decodeSL).isFailure()) {
                // store the error conditions into the StatusCode and continue
            }
        } else {
            if (msgLvl(MSG::DEBUG)) {
                uint32_t sourceId = (**itFrag).source_id();
                msg(MSG::DEBUG) << " ROB " << MSG::hex << sourceId << " is delivered with an empty payload" << MSG::dec << endmsg;
            }
            // store the error condition into the StatusCode and continue
        }
    }

    ATH_MSG_DEBUG("After processing, number of collections in container : " << pad->numberOfCollections());

    // in presence of errors return FAILURE
    return StatusCode::SUCCESS;
}

std::vector<IdentifierHash> RPC_RawDataProviderTool::to_be_converted(const ROBFragment& robFrag,
                                                                            const std::vector<IdentifierHash>& coll) const {
    SG::ReadCondHandle<RpcCablingCondData> readHandle{m_readKey};
    const RpcCablingCondData* readCdo{*readHandle};

    std::vector<IdentifierHash> to_return;
    if (coll.empty()) {
        // get SubdetectorId and ModuleId
        uint32_t source_id = robFrag.rod_source_id();
        SourceIdentifier sid(source_id);
        to_return = (readCdo->rod2hash(sid.subdetector_id(), sid.module_id()));
    } else {
        uint32_t source_id = robFrag.rod_source_id();
        std::vector<IdentifierHash>::const_iterator it = coll.begin();
        to_return.reserve(coll.size());
        for (; it != coll.end(); ++it) {
            if (source_id == readCdo->hash2source(*it)) to_return.emplace_back(*it);
        }
    }

    return to_return;
}

StatusCode RPC_RawDataProviderTool::convert(const EventContext& ctx) const {
    const RpcCablingCondData* readCdo{nullptr};
    ATH_CHECK(SG::get(readCdo, m_readKey, ctx));
    return convert(readCdo->giveFullListOfRobIds(), ctx);
}

StatusCode RPC_RawDataProviderTool::convert(const std::vector<uint32_t>& robIds, 
                                            const EventContext& ctx) const {
    std::vector<IdentifierHash> collections;
    ROBFragmentList vecOfRobf;
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    return convertIntoContainer(vecOfRobf, collections, ctx);
}

StatusCode RPC_RawDataProviderTool::convert(const std::vector<IdentifierHash>& rdoIdhVect, 
                                            const EventContext& ctx) const {
    const RpcCablingCondData* readCdo{nullptr};
    ATH_CHECK(SG::get(readCdo, m_readKey, ctx));
    ROBFragmentList vecOfRobf;
    std::vector<uint32_t> robIds;
    ATH_CHECK(readCdo->giveROB_fromRDO(rdoIdhVect, robIds));
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    return convertIntoContainer(vecOfRobf, rdoIdhVect, ctx);
}

StatusCode RPC_RawDataProviderTool::convertIntoContainer(const ROBFragmentList& vecRobs, 
                                                         const std::vector<IdentifierHash>& collections,
                                                         const EventContext& ctx) const {
    SG::WriteHandle<RpcPadContainer> rdoContainerHandle(m_containerKey, ctx);
    SG::WriteHandle<RpcSectorLogicContainer> logicHandle(m_sec, ctx);

    // run 3 mode, here we always try to write the containers

    // Split the methods to have one where we use the cache and one where we just setup the container
    const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
    if (!externalCacheRDO) {
        ATH_CHECK(rdoContainerHandle.record(std::make_unique<RpcPadContainer>(m_idHelperSvc->rpcIdHelper().module_hash_max())));
        ATH_MSG_DEBUG("Created RpcPadContainer");
    } else {
        SG::UpdateHandle<RpcPad_Cache> update(m_rdoContainerCacheKey, ctx);
        ATH_CHECK(update.isValid());
        ATH_CHECK(rdoContainerHandle.record(std::make_unique<RpcPadContainer>(update.ptr())));
        ATH_MSG_DEBUG("Created container using cache for " << m_rdoContainerCacheKey.key());
    }
    RpcPadContainer* pad = rdoContainerHandle.ptr();

    RpcSectorLogicContainer* logic = nullptr;
    if (m_WriteOutRpcSectorLogic) {
        ATH_MSG_DEBUG("Writing out RpcSectorLogicContainer");
        ATH_CHECK(logicHandle.record(std::make_unique<RpcSectorLogicContainer>()));
        logic = logicHandle.ptr();
    }

    // pass the containers to the convertIntoContainers function
    ATH_CHECK(convertIntoContainers(vecRobs, collections, pad, logic, true));

    return StatusCode::SUCCESS;
}
}