/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CSC_RawDataProviderTool.h"

#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/ROBData.h"
#include "ByteStreamData/RawEvent.h"
#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

//================ Constructor =================================================


//================ Destructor =================================================

Muon::CSC_RawDataProviderTool::~CSC_RawDataProviderTool() = default;

//================ Initialisation =================================================

StatusCode Muon::CSC_RawDataProviderTool::initialize() {
    ATH_CHECK(m_cabling.retrieve());
    ATH_CHECK(m_robDataProvider.retrieve());
    ATH_MSG_INFO("Retrieved service " << m_robDataProvider);

    ATH_CHECK(m_idHelperSvc.retrieve());
    m_hid2re.set(m_cabling.get(), &m_idHelperSvc->cscIdHelper());

    // Retrieve decoder
    ATH_CHECK(m_decoder.retrieve());
    ATH_MSG_INFO("Retrieved tool " << m_decoder);

    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

    // Initialise the container cache if available
    ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));

    ATH_MSG_INFO("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

//============================================================================================

StatusCode Muon::CSC_RawDataProviderTool::convertIntoContainer(
    const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs, const EventContext& ctx,
    CscRawDataContainer& container) const {
    std::set<uint32_t> robIdSet;
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

    ATH_MSG_DEBUG("Before processing numColls=" << container.numberOfCollections());

    ATH_MSG_DEBUG("vector of ROB ID to decode: size = " << vecRobs.size());

    for (const ROBFragment* frag : vecRobs) {
        uint32_t robid = frag->rod_source_id();

        // check if this ROBFragment was already decoded (EF case in ROIs
        if (!robIdSet.insert(robid).second) {
            ATH_MSG_DEBUG(" ROB Fragment with ID  " << std::hex << robid << std::dec << " already decoded, skip");
        } else {
            m_decoder->fillCollection(*eventInfo, *frag, container);
        }
    }

    ATH_MSG_DEBUG("After processing numColls=" << container.numberOfCollections());

    return StatusCode::SUCCESS;
}

//============================================================================================
// New EventContext-based convert methods

StatusCode Muon::CSC_RawDataProviderTool::convert(const std::vector<IdentifierHash>& rdoIdhVect, const EventContext& ctx) const {
    IdContext cscContext = m_idHelperSvc->cscIdHelper().module_context();

    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf;
    std::vector<uint32_t> robIds;

    for (unsigned int i = 0; i < rdoIdhVect.size(); ++i) {
        uint32_t rob_id = 0xffff;
        m_cabling->hash2RobFull(rdoIdhVect[i], rob_id);
        robIds.push_back(rob_id);
    }
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    ATH_MSG_VERBOSE("Number of ROB fragments " << vecOfRobf.size());

    // This would be passed to the function which does not use the IdentifierHash further
    return convert(vecOfRobf, ctx);
}

StatusCode Muon::CSC_RawDataProviderTool::convert(const EventContext& ctx) const {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf;
    const std::vector<uint32_t>& robIds = m_hid2re.allRobIds();
    ATH_MSG_VERBOSE("Number of ROB ids " << robIds.size());
    // ask ROBDataProviderSvc for the vector of ROBFragment for all CSC ROBIDs
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    ATH_MSG_VERBOSE("Number of ROB fragments " << vecOfRobf.size());

    return convert(vecOfRobf, ctx);
}


StatusCode Muon::CSC_RawDataProviderTool::convert(const ROBFragmentList& vecRobs, const EventContext& ctx) const {
    SG::WriteHandle<CscRawDataContainer> rdoContainerHandle(m_containerKey, ctx);

    // Split the methods to have one where we use the cache and one where we just setup the container
    const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
    if (!externalCacheRDO) {
        ATH_CHECK(rdoContainerHandle.record(std::make_unique<CscRawDataContainer>(m_idHelperSvc->cscIdHelper().module_hash_max())));
        ATH_MSG_DEBUG("Created CSCRawDataContainer");
    } else {
        SG::UpdateHandle<CscRawDataCollection_Cache> update(m_rdoContainerCacheKey, ctx);
        ATH_CHECK(update.isValid());
        ATH_CHECK(rdoContainerHandle.record(std::make_unique<CscRawDataContainer>(update.ptr())));
        ATH_MSG_DEBUG("Created container using cache for " << m_rdoContainerCacheKey.key());
    }

    CscRawDataContainer* container = rdoContainerHandle.ptr();

    if (!container) {
        ATH_MSG_ERROR("CSC RDO container pointer is null, cannot decode data");
        return StatusCode::FAILURE;
    }

    // call conversion function
    ATH_CHECK(convertIntoContainer(vecRobs, ctx, *container));

    return StatusCode::SUCCESS;
}
