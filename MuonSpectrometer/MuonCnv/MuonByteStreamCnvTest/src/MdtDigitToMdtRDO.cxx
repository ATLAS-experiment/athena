/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtDigitToMdtRDO.h"

#include <algorithm>
#include <atomic>
#include <cmath>

#include "EventInfoMgt/ITagInfoMgr.h"
#include "MuonDigitContainer/MdtDigit.h"
#include "MuonDigitContainer/MdtDigitCollection.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonRDO/MdtCsm.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "MuonRDO/MdtCsmIdHash.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
namespace {
    /// Print one-time warings about cases where the BMGs are part of the
    /// geometry but not implemented in the cabling. That should only happen in
    /// mc16a like setups.
    std::atomic<bool> bmgWarningPrinted = false;

}  // namespace

MdtDigitToMdtRDO::MdtDigitToMdtRDO(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MdtDigitToMdtRDO::initialize() {
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK(m_csmContainerKey.initialize());
    ATH_CHECK(m_digitContainerKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_cablingKey.initialize());
    ATH_CHECK(m_condKey.initialize());

    if (fillTagInfo().isFailure()) { ATH_MSG_WARNING("Could not fill the tagInfo for MDT cabling"); }

    m_BMG_station_name = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
    m_BMGpresent = m_BMG_station_name != -1;
    if (m_BMGpresent) { 
        ATH_MSG_DEBUG("Processing configuration for layouts with BME chambers (stationID: " << m_BMG_station_name << ")."); 
    }
    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MdtDigitToMdtRDO::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute() : fill_MDTdata");
    // create an empty pad container and record it
    SG::WriteHandle<MdtCsmContainer> csmContainer(m_csmContainerKey, ctx);
    ATH_CHECK(csmContainer.record(std::make_unique<MdtCsmContainer>()));
    ATH_MSG_DEBUG("Recorded MdtCsmContainer called " << csmContainer.fullKey());

    SG::ReadHandle<MdtDigitContainer> container(m_digitContainerKey, ctx);
    if (!container.isValid()) {
        ATH_MSG_ERROR("Could not find MdtDigitContainer called " << container.fullKey());
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Found MdtDigitContainer called " << container.fullKey());

    SG::ReadCondHandle<MuonMDT_CablingMap> readHandle_Cabling{m_cablingKey, ctx};
    const MuonMDT_CablingMap* cabling_ptr{*readHandle_Cabling};
    if (!cabling_ptr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }

    SG::ReadCondHandle<MdtCondDbData> readHandle_Conditions{m_condKey, ctx};
    const MdtCondDbData* condtionsPtr{*readHandle_Conditions};
    if (!condtionsPtr) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
    /// Internal map to cache all the CSMs
    std::vector<std::unique_ptr<MdtCsm>> csm_cache{};
    csm_cache.resize(id_helper.detectorElement_hash_max());
    // Iterate on the collections
    for (const MdtDigitCollection* mdtCollection : *container) {
        const Identifier chid1 = mdtCollection->identify();
        /// Remove dead tubes from the container
        if (!condtionsPtr->isGood(chid1)) continue;

        MuonMDT_CablingMap::CablingData cabling_data;
        if (!cabling_ptr->convert(chid1, cabling_data)) {
            ATH_MSG_FATAL("Found a non mdt identifier " << m_idHelperSvc->toString(chid1));
            return StatusCode::FAILURE;
        }

        /// Iterate on the digits of the collection
        for (const MdtDigit* mdtDigit : *mdtCollection) {
            const Identifier channelId = mdtDigit->identify();

            if (!id_helper.valid(channelId) || !cabling_ptr->convert(channelId, cabling_data)) {
                ATH_MSG_DEBUG("Found invalid mdt identifier " << channelId);
                continue;
            }

            /// Get the online Id of the channel
            bool cabling = cabling_ptr->getOnlineId(cabling_data, msgStream());

            if (!cabling) {
                if (cabling_data.stationIndex == m_BMG_station_name) {
                    if (!bmgWarningPrinted) {
                        ATH_MSG_WARNING("Apparently BMG chambers are disconnected to the cabling. "
                                        << "This has been checked to only appear in mc16a-like setups as the chambers were installed in "
                                           "the end-of-the-year shutdown 2016. "
                                        << "In any other case, be despaired in facing the villian and check what has gone wrong");
                        bmgWarningPrinted = true;
                    }
                    continue;
                }
                ATH_MSG_ERROR("MDTcabling can't return an online ID for the channel : " << cabling_data);
                return StatusCode::FAILURE;
            }

            // Create the new AMT hit
            auto amtHit = std::make_unique<MdtAmtHit>(cabling_data.tdcId, cabling_data.channelId, mdtDigit->isMasked());
            // Get coarse time and fine time
            int tdc_counts = mdtDigit->tdc();

            uint16_t coarse = (tdc_counts >> 5) & 0xfff;
            uint16_t fine = tdc_counts & 0x1f;
            uint16_t width = mdtDigit->adc();

            amtHit->setValues(coarse, fine, width);

            ATH_MSG_DEBUG("Adding a new AmtHit -- " << cabling_data);
            ATH_MSG_DEBUG(" Coarse time : " << coarse << " Fine time : " << fine << " Width : " << width);

            /// Get the proper CSM hash and Csm multilayer Id
            IdentifierHash csm_hash{0};
            Identifier csmId{0};
            /// Copy the online information take the 0-th channel and the 0-th tdc
            if (!cabling_ptr->getMultiLayerCode(cabling_data, csmId, csm_hash, msgStream())) {
                ATH_MSG_ERROR("Hash generation failed for " << cabling_data);
                return StatusCode::FAILURE;
            }

            /// Find the proper csm card otherwise create a new one
            std::unique_ptr<MdtCsm>& mdtCsm = csm_cache[csm_hash];
            if (!mdtCsm) {
                ATH_MSG_DEBUG("Insert new CSM module using " << cabling_data << " " << m_idHelperSvc->toString(csmId));
                mdtCsm = std::make_unique<MdtCsm>(csmId, csm_hash, cabling_data.subdetectorId, cabling_data.mrod, cabling_data.csm);
            }
            // Check that the CSM is correct
            if (cabling_data.csm != mdtCsm->CsmId() || cabling_data.subdetectorId != mdtCsm->SubDetId() || cabling_data.mrod != mdtCsm->MrodId()) {
                MdtCablingData wrongCsm{};
                wrongCsm.csm = mdtCsm->CsmId();
                wrongCsm.mrod = mdtCsm->MrodId();
                wrongCsm.subdetectorId = mdtCsm->SubDetId();
                wrongCsm.channelId =0;
                wrongCsm.tdcId =0;

                std::ignore = cabling_ptr->getOfflineId(wrongCsm, msgStream());
                Identifier wrongId{};
                cabling_ptr->convert(wrongCsm,wrongId);

                ATH_MSG_FATAL("CSM collection "<<static_cast<const MdtCablingOnData&>(wrongCsm) <<" ("<<m_idHelperSvc->toStringDetEl(wrongId)
                            <<") does not match with " <<static_cast<const MdtCablingOnData&>(cabling_data)<<" ("<<m_idHelperSvc->toStringDetEl(chid1)<<")");
                return StatusCode::FAILURE;
            }
            // Add the digit to the CSM
            mdtCsm->push_back(std::move(amtHit));
        }
    }
    /// Add the CSM to the CsmContainer
    for (unsigned int hash= 0; hash < csm_cache.size(); ++hash) {
        if (!csm_cache[hash]) continue;
        ATH_CHECK(csmContainer->addCollection(csm_cache[hash].release(), hash));
    }
    return StatusCode::SUCCESS;
}

// NOTE: although this function has no clients in release 22, currently the Run2 trigger simulation is still run in
//       release 21 on RDOs produced in release 22. Since release 21 accesses the TagInfo, it needs to be written to the
//       RDOs produced in release 22. The fillTagInfo() function thus needs to stay in release 22 until the workflow changes
StatusCode MdtDigitToMdtRDO::fillTagInfo() const {
    ServiceHandle<ITagInfoMgr> tagInfoMgr("TagInfoMgr", name());
    if (tagInfoMgr.retrieve().isFailure()) { return StatusCode::FAILURE; }

    std::string cablingType = "NewMDT_Cabling";  // everything starting from Run2 should be 'New'
    StatusCode sc = tagInfoMgr->addTag("MDT_CablingType", cablingType);

    if (sc.isFailure()) {
        ATH_MSG_WARNING("MDT_CablingType " << cablingType << " not added to TagInfo ");
        return sc;
    } else {
        ATH_MSG_DEBUG("MDT_CablingType " << cablingType << " is Added TagInfo ");
    }

    return StatusCode::SUCCESS;
}
