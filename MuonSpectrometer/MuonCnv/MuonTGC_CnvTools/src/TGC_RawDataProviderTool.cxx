/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TGC_RawDataProviderTool.h"

//================ Initialisation =================
namespace Muon {
StatusCode TGC_RawDataProviderTool::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());

    ATH_CHECK(m_decoder.retrieve());

    // Get ROBDataProviderSvc
    ATH_CHECK(m_robDataProvider.retrieve());

    m_maxhashtoUse = m_idHelperSvc->tgcIdHelper().module_hash_max();

    ATH_CHECK(m_rdoContainerKey.initialize());

    ATH_CHECK(m_cablingKey.initialize());
    m_hid2re.fillAllRobIds();
    // Initialise the container cache if available
    ATH_CHECK(m_rdoContainerCacheKey.initialize(
        !m_rdoContainerCacheKey.key().empty()));

    ATH_MSG_INFO("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

StatusCode TGC_RawDataProviderTool::convertIntoContainer(
    const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
    TgcRdoContainer& tgcRdoContainer, const EventContext& ctx) const {

    /// Static variables are not thread safe
    static thread_local int DecodeErrCount = 0;

    // Update to range based loop
    for (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* fragment : vecRobs) {
        if (m_decoder->fillCollection(*fragment, tgcRdoContainer, ctx)
                .isFailure()) {
            if (DecodeErrCount < 100) {
                ATH_MSG_INFO("Problem with TGC ByteStream Decoding!");
                DecodeErrCount++;
            } else if (100 == DecodeErrCount) {
                ATH_MSG_INFO(
                    "Too many Problems with TGC Decoding messages. Turning "
                    "message off.");
                DecodeErrCount++;
            }
        }
    }
    ATH_MSG_DEBUG("Size of TgcRdoContainer is " << tgcRdoContainer.size());
    return StatusCode::SUCCESS;
}

std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>
TGC_RawDataProviderTool::getROBData(
    const std::vector<IdentifierHash>& rdoIdhVect,
    const EventContext& ctx) const {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf;
    const TgcCablingMap* cabling{};
    if (!SG::get(cabling, m_cablingKey, ctx).isSuccess()) {
        ATH_MSG_ERROR(
            "Could not get cabling, return empty vector of ROB fragments");
        return vecOfRobf;
    }

    IdContext tgcContext = m_idHelperSvc->tgcIdHelper().module_context();

    std::vector<uint32_t> robIds;

    unsigned int size = rdoIdhVect.size();
    for (unsigned int i = 0; i < size; ++i) {
        Identifier Id;
        if (m_idHelperSvc->tgcIdHelper().get_id(rdoIdhVect[i], Id,
                                                &tgcContext)) {
            ATH_MSG_WARNING(
                "Unable to get TGC Identifier from collection hash id ");
            continue;
        }
        const Identifier tgcId = Id;
        uint32_t rodId = m_hid2re.getRodID(tgcId, cabling);
        uint32_t robId = m_hid2re.getRobID(rodId);
        std::vector<uint32_t>::iterator it_robId =
            std::find(robIds.begin(), robIds.end(), robId);
        if (it_robId == robIds.end()) {
            robIds.push_back(robId);
        }
    }
    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);
    ATH_MSG_VERBOSE("Number of ROB fragments " << vecOfRobf.size());
    return vecOfRobf;
}

//============================================================================================

StatusCode TGC_RawDataProviderTool::convert(
    const ROBFragmentList& vecRobs) const {
    return convert(vecRobs, Gaudi::Hive::currentContext());
}

StatusCode TGC_RawDataProviderTool::convert(const ROBFragmentList& vecRobs,
                                            const EventContext& ctx) const {

    SG::WriteHandle<TgcRdoContainer> rdoContainerHandle(m_rdoContainerKey, ctx);

    // Split the methods to have one where we use the cache and one where we
    // just setup the container
    const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
    if (!externalCacheRDO) {
        ATH_CHECK(rdoContainerHandle.record(
            std::make_unique<TgcRdoContainer>(m_maxhashtoUse)));
        ATH_MSG_DEBUG("Created TGC container");
    } else {
        SG::UpdateHandle<TgcRdo_Cache> update(m_rdoContainerCacheKey, ctx);
        ATH_CHECK(update.isValid());
        ATH_CHECK(rdoContainerHandle.record(
            std::make_unique<TgcRdoContainer>(update.ptr())));
        ATH_MSG_DEBUG("Created container using cache for "
                      << m_rdoContainerCacheKey.key());
    }

    TgcRdoContainer* rdoContainer = rdoContainerHandle.ptr();

    // this should never happen, but since we dereference the pointer, we should
    // check
    if (!rdoContainer) {
        ATH_MSG_ERROR("TGC RDO Container is null, cannot decode TGC data");
        return StatusCode::FAILURE;
    }

    return convertIntoContainer(vecRobs, *rdoContainer, ctx);
}

StatusCode TGC_RawDataProviderTool::convert(
    const ROBFragmentList& vecRobs, const std::vector<IdentifierHash>&) const {
    // This function does not use the IdentifierHash so we pass to the
    // EventContext function which also does not use it
    return convert(vecRobs, Gaudi::Hive::currentContext());
}

StatusCode TGC_RawDataProviderTool::convert(
    const ROBFragmentList& vecRobs, const std::vector<IdentifierHash>& /*ids*/,
    const EventContext& ctx) const {
    return convert(vecRobs, ctx);
}

StatusCode TGC_RawDataProviderTool::convert() const {
    return convert(Gaudi::Hive::currentContext());
}

StatusCode TGC_RawDataProviderTool::convert(const EventContext& ctx) const {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf;
    const std::vector<uint32_t>& robIds = m_hid2re.allRobIds();

    m_robDataProvider->getROBData(ctx, robIds, vecOfRobf);

    return convert(vecOfRobf, ctx);
}

StatusCode TGC_RawDataProviderTool::convert(
    const std::vector<IdentifierHash>& rdoIdhVect) const {
    return convert(rdoIdhVect, Gaudi::Hive::currentContext());
}

StatusCode TGC_RawDataProviderTool::convert(
    const std::vector<IdentifierHash>& rdoIdhVect,
    const EventContext& ctx) const {
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecOfRobf =
        getROBData(rdoIdhVect, ctx);

    return convert(vecOfRobf, rdoIdhVect, ctx);
}

}  // namespace Muon