/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_csmContainerKey);
    ATH_CHECK(m_digitContainerKey.initialize());
    ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_digitContainerKey);
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_cablingKey.initialize());
    ATH_CHECK(m_condKey.initialize());

    if (fillTagInfo().isFailure()) { ATH_MSG_WARNING("Could not fill the tagInfo for MDT cabling"); }

    m_BMG_station_name = m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG");
    m_BMGpresent = m_BMG_station_name != -1;
    if (m_BMGpresent) { ATH_MSG_INFO("Processing configuration for layouts with BME chambers (stationID: " << m_BMG_station_name << ")."); }
    m_BIS_station_name = m_idHelperSvc->mdtIdHelper().stationNameIndex("BIS");
    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode MdtDigitToMdtRDO::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute() : fill_MDTdata");
    // create an empty pad container and record it
    SG::WriteHandle<MdtCsmContainer> csmContainer(m_csmContainerKey, ctx);
    ATH_CHECK(csmContainer.record(std::make_unique<MdtCsmContainer>()));
    ATH_MSG_DEBUG("Recorded MdtCsmContainer called " << csmContainer.name() << " in store " << csmContainer.store());

    SG::ReadHandle<MdtDigitContainer> container(m_digitContainerKey, ctx);
    if (!container.isValid()) {
        ATH_MSG_ERROR("Could not find MdtDigitContainer called " << container.name() << " in store " << container.store());
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Found MdtDigitContainer called " << container.name() << " in store " << container.store());

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
    auto& msg = msgStream();

    /// Internal map to cache all the CSMs
    using csmMap = std::map<IdentifierHash, std::unique_ptr<MdtCsm>>;
    csmMap csm_cache{};
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
            bool cabling = cabling_ptr->getOnlineId(cabling_data, msg);

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
                /// For the moment remove the BIS stations from the geometry
                if (m_isPhaseII && id_helper.stationName(channelId) == m_BIS_station_name && m_idHelperSvc->issMdt(channelId)) {
                    ATH_MSG_DEBUG("Found BIS sMDT which cannot be mapped " << cabling_data
                                                                           << ". This should only happen in the Phase-II geometry.");
                    continue;
                }
                ATH_MSG_ERROR("MDTcabling can't return an online ID for the channel : " << cabling_data);
                return StatusCode::FAILURE;
            }

            // Create the new AMT hit
            std::unique_ptr<MdtAmtHit> amtHit =
                std::make_unique<MdtAmtHit>(cabling_data.tdcId, cabling_data.channelId, mdtDigit->is_masked());
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
            if (!cabling_ptr->getMultiLayerCode(cabling_data, csmId, csm_hash, msg)) {
                ATH_MSG_ERROR("Hash generation failed for " << cabling_data);
                return StatusCode::FAILURE;
            }

            /// Find the proper csm card otherwise create a new one
            csmMap::iterator csm_itr = csm_cache.find(csm_hash);
            if (csm_itr == csm_cache.end()) {
                ATH_MSG_DEBUG("Insert new CSM module using " << cabling_data << " " << m_idHelperSvc->toString(csmId));
                std::unique_ptr<MdtCsm> csm =
                    std::make_unique<MdtCsm>(csmId, csm_hash, cabling_data.subdetectorId, cabling_data.mrod, cabling_data.csm);
                csm_itr = csm_cache.insert(std::make_pair(csm_hash, std::move(csm))).first;
            }
            std::unique_ptr<MdtCsm>& mdtCsm = csm_itr->second;
            // Check that the CSM is correct
            if (cabling_data.csm != mdtCsm->CsmId() || cabling_data.subdetectorId != mdtCsm->SubDetId() ||
                cabling_data.mrod != mdtCsm->MrodId()) {
                ATH_MSG_FATAL("Cannot create AmtHit " << cabling_data);
                return StatusCode::FAILURE;
            }
            // Add the digit to the CSM
            mdtCsm->push_back(std::move(amtHit));
        }
    }
    /// Add the CSM to the CsmContainer
    for (auto& csm_coll : csm_cache) {
        ATH_MSG_DEBUG("Add CSM container " << csm_coll.first << "  " << csm_coll.first);
        ATH_CHECK(csmContainer->addCollection(csm_coll.second.release(), csm_coll.first));
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
