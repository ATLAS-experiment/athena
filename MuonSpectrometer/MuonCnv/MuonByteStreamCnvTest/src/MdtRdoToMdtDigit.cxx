/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToMdtDigit.h"

MdtRdoToMdtDigit::MdtRdoToMdtDigit(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtRdoToMdtDigit::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_mdtRdoDecoderTool.retrieve());
    ATH_CHECK(m_mdtRdoKey.initialize());
    ATH_CHECK(m_mdtDigitKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode MdtRdoToMdtDigit::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
    SG::ReadHandle<MdtCsmContainer> rdoRH(m_mdtRdoKey, ctx);
    if (!rdoRH.isValid()) {
        ATH_MSG_WARNING("No MDT RDO container found!");
        return StatusCode::SUCCESS;
    }
    const MdtCsmContainer* rdoContainer = rdoRH.cptr();
    ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " MDT RDOs.");

    SG::WriteHandle<MdtDigitContainer> wh_mdtDigit(m_mdtDigitKey, ctx);
    ATH_CHECK(wh_mdtDigit.record(std::make_unique<MdtDigitContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Decoding MDT RDO into MDT Digit");

    // now decode RDO into digits
    std::unordered_map<IdentifierHash, std::unique_ptr<MdtDigitCollection>> digitMap{};
    for (const MdtCsm* csmColl : *rdoContainer) { 
        ATH_CHECK(decodeMdt(*csmColl, digitMap)); 
    }
    
    for (auto& [hash, collection]: digitMap) {
        ATH_CHECK(wh_mdtDigit->addCollection(collection.release(), hash));
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtRdoToMdtDigit::decodeMdt(const MdtCsm& rdoColl, DigitCollection& digitMap) const {
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
        std::unique_ptr<MdtDigit> newDigit{m_mdtRdoDecoderTool->getDigit(amtHit, subdetId, mrodId, csmId, Gaudi::Hive::currentContext())};

        if (!newDigit) {
            ATH_MSG_WARNING("Error in MDT RDO decoder");
            continue;
        }

        // find here the Proper Digit Collection identifier, using the rdo-hit id
        // (since RDO collections are not in a 1-to-1 relation with digit collections)
        const Identifier elementId = m_idHelperSvc->mdtIdHelper().elementID(newDigit->identify());
        const IdentifierHash coll_hash = m_idHelperSvc->moduleHash(newDigit->identify());
        std::unique_ptr<MdtDigitCollection>& outCollection = digitMap[coll_hash];
        if(!outCollection) {
            outCollection = std::make_unique<MdtDigitCollection>(elementId, coll_hash);
        }
        outCollection->push_back(std::move(newDigit));
    }
    return StatusCode::SUCCESS;
}
