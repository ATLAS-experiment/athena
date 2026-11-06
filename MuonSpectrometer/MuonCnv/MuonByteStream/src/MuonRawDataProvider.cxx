/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRawDataProvider.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include <vector>

#include "StoreGate/ReadHandle.h"

StatusCode Muon::MuonRawDataProvider::initialize() {
    ATH_MSG_DEBUG("MuonRawDataProvider::initialize");
    ATH_MSG_DEBUG(m_seededDecoding);

    ATH_CHECK(m_rawDataTool.retrieve());
    ATH_CHECK(m_roiCollectionKey.initialize(m_seededDecoding));

    ATH_CHECK(m_regselTool.retrieve(EnableTool{m_seededDecoding}));

    return StatusCode::SUCCESS;
}

StatusCode Muon::MuonRawDataProvider::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("MuonRawDataProvider::execute");

    if (!m_seededDecoding) {
        if (m_rawDataTool->convert(ctx).isFailure()) {
            ATH_MSG_WARNING("BS conversion into RDOs failed");
            return m_failOnConvertError ? StatusCode::FAILURE : StatusCode::SUCCESS;
        }
        return StatusCode::SUCCESS;
    }

    const TrigRoiDescriptorCollection* muonRoI{};
    if (!SG::get(muonRoI, m_roiCollectionKey, ctx).isSuccess()) {
        ATH_MSG_WARNING("Cannot retrieve muonRoI " << m_roiCollectionKey.key());
        return m_ignoreMissingRoIs ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    if (m_useHashIds) {
        std::vector<IdentifierHash> hashIds;
        // Collect hash IDs from all RoIs first, then call convert once
        for (const TrigRoiDescriptor* roi : *muonRoI) {
            ATH_MSG_DEBUG("Get Hash IDs for RoI " << *roi);
            m_regselTool->lookup(ctx)->HashIDList(*roi, hashIds);
        }
        ATH_MSG_DEBUG("Decode the following detector hashes "<<hashIds);
        if (m_rawDataTool->convert(hashIds, ctx).isFailure()) {
            ATH_MSG_ERROR("RoI seeded BS conversion into RDOs failed");
            return m_failOnConvertError ? StatusCode::FAILURE : StatusCode::SUCCESS;
        }
        return StatusCode::SUCCESS;
    }

    std::vector<uint32_t> robs;
    // Collect ROBs from all RoIs first, then call convert once
    for (const TrigRoiDescriptor* roi : *muonRoI) {
        ATH_MSG_DEBUG("Get ROBs for RoI " << *roi);
        m_regselTool->lookup(ctx)->ROBIDList(*roi, robs);
    }
    ATH_MSG_DEBUG("Decode the following ROBs:\n "<<robs);
    if (m_rawDataTool->convert(robs, ctx).isFailure()) {
        ATH_MSG_ERROR("RoI seeded BS conversion into RDOs failed");
        return m_failOnConvertError ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }

    return StatusCode::SUCCESS;
}
