/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToMdtDigit.h"

StatusCode MdtRdoToMdtDigit::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_mdtRdoDecoderTool.retrieve());
    ATH_CHECK(m_mdtRdoKey.initialize());
    ATH_CHECK(m_mdtDigitKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MdtRdoToMdtDigit::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
    SG::ReadHandle rdoContainer{m_mdtRdoKey, ctx};
    ATH_CHECK(rdoContainer.isPresent());
    ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " MDT RDOs.");

    SG::WriteHandle<MdtDigitContainer> wh_mdtDigit(m_mdtDigitKey, ctx);
    const unsigned int hashMax = m_idHelperSvc->mdtIdHelper().module_hash_max();
    ATH_CHECK(wh_mdtDigit.record(std::make_unique<MdtDigitContainer>(hashMax)));
    ATH_MSG_DEBUG("Decoding MDT RDO into MDT Digit");

    // now decode RDO into digits
    DigitCollection digitMap{};
    digitMap.resize(hashMax);
    for (const MdtCsm* csmColl : *rdoContainer) { 
        ATH_CHECK(decodeMdt(ctx, *csmColl, digitMap)); 
    }
    
    for (std::unique_ptr<MdtDigitCollection>&collection: digitMap) {
        if (!collection) continue;
        const IdentifierHash hash = collection->identifierHash();
        ATH_CHECK(wh_mdtDigit->addCollection(collection.release(), hash));
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtRdoToMdtDigit::decodeMdt(const EventContext& ctx, const MdtCsm& rdoColl, DigitCollection& digitMap) const {
    if (rdoColl.empty()) {
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG(" Number of AmtHit in this Csm " << rdoColl.size());

    uint16_t subdetId = rdoColl.SubDetId();
    uint16_t mrodId = rdoColl.MrodId();
    uint16_t csmId = rdoColl.CsmId();

    
    // for each Csm, loop over AmtHit, converter AmtHit to digit
    // retrieve/create digit collection, and insert digit into collection
    for (const MdtAmtHit* amtHit : rdoColl) {
        std::unique_ptr<MdtDigit> newDigit{m_mdtRdoDecoderTool->getDigit(ctx, *amtHit, subdetId, mrodId, csmId)};

        if (!newDigit) {
            ATH_MSG_WARNING("Digit rejected");
            continue;
        }

        // find here the Proper Digit Collection identifier, using the rdo-hit id
        // (since RDO collections are not in a 1-to-1 relation with digit collections)
        const IdentifierHash coll_hash = m_idHelperSvc->moduleHash(newDigit->identify());
        std::unique_ptr<MdtDigitCollection>& outCollection = digitMap[coll_hash];
        if(!outCollection) {
            outCollection = std::make_unique<MdtDigitCollection>(m_idHelperSvc->chamberId(newDigit->identify()), coll_hash);
        }
        outCollection->push_back(std::move(newDigit));
    }
    return StatusCode::SUCCESS;
}
