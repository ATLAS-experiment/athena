/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MM_DigitToRDO.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

MM_DigitToRDO::MM_DigitToRDO(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MM_DigitToRDO::initialize() {
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_rdoContainer.initialize());
    ATH_CHECK(m_digitContainer.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_cablingKey.initialize(!m_cablingKey.empty()));
    return StatusCode::SUCCESS;
}

StatusCode MM_DigitToRDO::execute(const EventContext& ctx) const {
    using namespace Muon;
    ATH_MSG_DEBUG("in execute()");
    SG::ReadHandle<MmDigitContainer> digits(m_digitContainer, ctx);
    ATH_CHECK(digits.isPresent());
    std::unique_ptr<MM_RawDataContainer> rdos = std::make_unique<MM_RawDataContainer>(m_idHelperSvc->mmIdHelper().module_hash_max());

    const Nsw_CablingMap* mmCablingMap{nullptr};
    if (!m_cablingKey.empty()) {
        SG::ReadCondHandle<Nsw_CablingMap> readCondHandle{m_cablingKey, ctx};
        if(!readCondHandle.isValid()){
          ATH_MSG_ERROR("Cannot find Micromegas cabling map!");
          return StatusCode::FAILURE;
        }
        mmCablingMap = readCondHandle.cptr();
    }
   
    for (const MmDigitCollection* digitColl : *digits) {
        // the identifier of the digit collection is the detector element Id ( multilayer granularity )
        Identifier digitId = digitColl->identify();

        // get the hash of the RDO collection
        IdentifierHash hash;
        int getRdoCollHash = m_idHelperSvc->mmIdHelper().get_module_hash(digitId, hash);
        if (getRdoCollHash != 0) {
            ATH_MSG_ERROR("Could not get the module hash Id");
            return StatusCode::FAILURE;
        }

        MM_RawDataCollection* coll = new MM_RawDataCollection(hash);
        if (rdos->addCollection(coll, hash).isFailure()) {
            ATH_MSG_WARNING("Failed to add collection with hash " << (int)hash);
            delete coll;
            continue;
        }

        for (const MmDigit* digit : *digitColl) {
            Identifier newId = digit->identify();               
            // RDO has time and charge in counts
            int tdo{0}, relBcid{0}, pdo{0};
            m_calibTool->timeToTdo(ctx, digit->stripResponseTime(), newId, tdo, relBcid);
            m_calibTool->chargeToPdo(ctx, digit->stripResponseCharge(), newId, pdo);

            // the cabling map is introduced here for studies of the MM strip misalignment. It does not run in standart jobs (read key is empty) 
            if (mmCablingMap) {
                std::optional<Identifier> correctedChannelId = mmCablingMap->correctChannel(newId, msgStream());
                if (!correctedChannelId) {
                    ATH_MSG_DEBUG("Channel was shifted outside its connector and is therefore not decoded into and RDO");
                    continue;
                }
                newId = (*correctedChannelId);
            }
            // Fill object
            std::unique_ptr<MM_RawData> rdo = std::make_unique<MM_RawData>(newId, m_idHelperSvc->mmIdHelper().channel(newId), 
                                                                           tdo, pdo, relBcid, true);
            coll->push_back(std::move(rdo));                
        }
    }
    
    SG::WriteHandle<MM_RawDataContainer> writeHanlde(m_rdoContainer, ctx);
    ATH_CHECK(writeHanlde.record(std::move(rdos)));

    ATH_MSG_DEBUG("done execute()");
    return StatusCode::SUCCESS;
}
