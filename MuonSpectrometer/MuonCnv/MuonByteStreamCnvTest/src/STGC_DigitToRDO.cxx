/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "STGC_DigitToRDO.h"

class NswCalibDbTimeChargeData;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

STGC_DigitToRDO::STGC_DigitToRDO(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode STGC_DigitToRDO::initialize() {
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_rdoContainer.initialize());
    ATH_CHECK(m_digitContainer.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_cablingKey.initialize(!m_cablingKey.empty()));

    return StatusCode::SUCCESS;
}

StatusCode STGC_DigitToRDO::execute(const EventContext& ctx) const {
    using namespace Muon;
    ATH_MSG_DEBUG("in execute()");
    SG::ReadHandle<sTgcDigitContainer> digits(m_digitContainer, ctx);
    ATH_CHECK(digits.isPresent());
    std::unique_ptr<STGC_RawDataContainer> rdos = std::make_unique<STGC_RawDataContainer>(m_idHelperSvc->stgcIdHelper().module_hash_max());
    
    const Nsw_CablingMap* sTgcCablingMap{nullptr};
    if (!m_cablingKey.empty()) {
        SG::ReadCondHandle<Nsw_CablingMap> readCondHandle{m_cablingKey, ctx};
        if(!readCondHandle.isValid()){
          ATH_MSG_ERROR("Cannot find Micromegas cabling map!");
          return StatusCode::FAILURE;
        }
        sTgcCablingMap = readCondHandle.cptr();
    }

    for (const sTgcDigitCollection* digitColl : *digits) {
        // Transform the hash id ( digit collection use detector element ID hash, RDO's use
        // module Id hash
        Identifier digitId = digitColl->identify();
        IdentifierHash hash = m_idHelperSvc->moduleHash(digitId);

        STGC_RawDataCollection* coll = new STGC_RawDataCollection(hash);
        if (rdos->addCollection(coll, hash).isFailure()) {
            ATH_MSG_WARNING("Failed to add collection with hash " << (int)hash);
            delete coll;
            continue;
        }

        constexpr double lowerTimeBound = Muon::STGC_RawData::s_lowerTimeBound;
        constexpr int BCWindow = Muon::STGC_RawData::s_BCWindow;
        for (const sTgcDigit* digit : *digitColl) {
            // Set proper data time window in simulated sTGC RDOs
            // BC0 has t = [-12.5, +12.5]
            // and data will use BC = [-3,+4]
            // totaling digits within t = [-87.5, 112.5]
            float digitTime = digit->time();
            if (digitTime < lowerTimeBound || digitTime >= lowerTimeBound + BCWindow * 25) continue;
            Identifier id = digit->identify();
            if (sTgcCablingMap) {
                std::optional<Identifier> correctedChannelId = sTgcCablingMap->correctChannel(id, msgStream());
                if (!correctedChannelId) {
                    ATH_MSG_DEBUG("Channel was shifted outside its connector and is therefore not decoded into and RDO");
                    continue;
                }
                id = (*correctedChannelId);
            }
            bool isDead = digit->isDead();
            int tdo{0}, pdo{0}, relBCID{0};
            m_calibTool->timeToTdo(ctx, digitTime, id, tdo, relBCID);
            m_calibTool->chargeToPdo(ctx, digit->charge(), id, pdo);
            STGC_RawData* rdo = new STGC_RawData(id, static_cast<unsigned int>(relBCID), static_cast<float>(tdo),
                                                    static_cast<unsigned int>(tdo), static_cast<unsigned int>(pdo), isDead, true);
            coll->push_back(rdo);
        }
    }
    
    SG::WriteHandle<STGC_RawDataContainer> writeHandle(m_rdoContainer, ctx);
    ATH_CHECK(writeHandle.record(std::move(rdos)));

    ATH_MSG_DEBUG("done execute()");
    return StatusCode::SUCCESS;
}
