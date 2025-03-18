/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MM_RdoToDigit.h"


StatusCode MM_RdoToDigit::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_mmRdoDecoderTool.retrieve());
    ATH_CHECK(m_mmRdoKey.initialize());
    ATH_CHECK(m_mmDigitKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MM_RdoToDigit::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
    SG::ReadHandle<Muon::MM_RawDataContainer> rdoContainer(m_mmRdoKey, ctx);
    if (!rdoContainer.isPresent()) {
        ATH_MSG_ERROR("No MM RDO container found!");
        return StatusCode::FAILURE;
    }
    SG::WriteHandle<MmDigitContainer> wh_mmDigit(m_mmDigitKey, ctx);
    ATH_CHECK(wh_mmDigit.record(std::make_unique<MmDigitContainer>(m_idHelperSvc->mmIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Decoding MM RDO into MM Digit");
    DigitCollection digitMap{};
    for (const Muon::MM_RawDataCollection* coll : *rdoContainer) { 
        ATH_CHECK(decodeMM(ctx, *coll, digitMap)); 
    }
    
    for (auto& [hash, collection]: digitMap) {
        ATH_CHECK(wh_mmDigit->addCollection(collection.release(), hash));
    }

    return StatusCode::SUCCESS;
}

StatusCode MM_RdoToDigit::decodeMM(const EventContext& ctx,
                                   const Muon::MM_RawDataCollection& rdoColl, 
                                   DigitCollection& digitContainer) const {
    if (rdoColl.empty()) {
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG(" Number of RawData in this rdo " << rdoColl.size());

    // for each RDO, loop over RawData, converter RawData to digit
    // retrieve/create digit collection, and insert digit into collection
    for (const Muon::MM_RawData* data : rdoColl) {
        std::unique_ptr<MmDigit> newDigit{m_mmRdoDecoderTool->getDigit(ctx, data)};
        if (!newDigit) {
            ATH_MSG_WARNING("Error in MM RDO decoder");
            continue;
        }

        // find here the Proper Digit Collection identifier, using the rdo-hit id
        // (since RDO collections are not in a 1-to-1 relation with digit collections)
        const Identifier elementId = m_idHelperSvc->mmIdHelper().elementID(newDigit->identify());
        IdentifierHash coll_hash = m_idHelperSvc->moduleHash(elementId);
        std::unique_ptr<MmDigitCollection>& outCollection = digitContainer[coll_hash];
        if (!outCollection) {
             outCollection = std::make_unique<MmDigitCollection>(elementId, coll_hash);
        }
        outCollection->push_back(std::move(newDigit));
    }
    
    return StatusCode::SUCCESS;
}
