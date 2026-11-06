/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRdoToPrepDataAlg.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "Identifier/IdentifierHash.h"
#include <cstdint>
#include <vector>
#include <ranges>

StatusCode MuonRdoToPrepDataAlg::initialize() {
    ATH_MSG_DEBUG(" in initialize()");

    // verify that our tool handle is pointing to an accessible tool
    ATH_CHECK(m_tool.retrieve());
    ATH_CHECK(m_roiCollectionKey.initialize(m_seededDecoding));
    ATH_CHECK(m_regsel.retrieve(EnableTool{m_seededDecoding}));
    return StatusCode::SUCCESS;
}

StatusCode MuonRdoToPrepDataAlg::execute(const EventContext& ctx)  const {
    ATH_MSG_DEBUG("**************** in MuonRdoToPrepDataAlg::execute() ***********************************************");
    const TrigRoiDescriptorCollection* muonRoI{};
    ATH_CHECK(SG::get(muonRoI, m_roiCollectionKey, ctx));

    std::vector<IdentifierHash> chamberHashes{};
    std::vector<uint32_t> robs{};    
    if (muonRoI) {  // decoding from trigger roi
        for (const auto *roi : *muonRoI) {
            if (m_robDecoding) {
                std::vector<uint32_t> toDecode{};
                m_regsel->lookup(ctx)->ROBIDList(*roi, toDecode);
                std::ranges::move(toDecode, std::back_inserter(robs));
            } else {
                std::vector<IdentifierHash> toDecode{};
                m_regsel->lookup(ctx)->HashIDList(*roi, toDecode);
                std::ranges::move(toDecode, std::back_inserter(chamberHashes));
            }
        } 
        if (!robs.empty()) {
            ATH_MSG_DEBUG("Decode the following robs \n"<<robs);
            ATH_CHECK(m_tool->decode(ctx, robs));
        } else if (!chamberHashes.empty()) {
            ATH_MSG_DEBUG("Decode the following chamber hashes \n"<<chamberHashes);
            ATH_CHECK(m_tool->decode(ctx, chamberHashes));
        } else {
            ATH_CHECK(m_tool->provideEmptyContainer(ctx));
        }    
    } else {
        ATH_CHECK(m_tool->decode(ctx, chamberHashes));
    }
    return StatusCode::SUCCESS;
}
