/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcDigitToTgcRDO.h"

#include "EventInfoMgt/ITagInfoMgr.h"
#include "MuonDigitContainer/TgcDigit.h"
#include "MuonDigitContainer/TgcDigitCollection.h"
#include "MuonRDO/TgcRdoIdHash.h"
#include "StoreGate/StoreGateSvc.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
namespace {
    static uint16_t identifyFragment(const TgcRawData& rawData) {
        return TgcRdo::identifyRawData(rawData);
    }
}

TgcDigitToTgcRDO::TgcDigitToTgcRDO(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator){}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode TgcDigitToTgcRDO::initialize() {
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tgc_cabling_server.retrieve());

    ATH_MSG_DEBUG("standard digitization job: "
                  << "initialize now the TGC cabling and TGC container.");
    ATH_CHECK(getCabling());

    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_rdoContainerKey);
    ATH_CHECK(m_digitContainerKey.initialize());
    ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_digitContainerKey);

    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode TgcDigitToTgcRDO::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
 
    SG::WriteHandle<TgcRdoContainer> rdoContainer(m_rdoContainerKey, ctx);
    ATH_CHECK(rdoContainer.record(std::make_unique<TgcRdoContainer>()));
    ATH_MSG_DEBUG("Recorded TgcRdoContainer called " << rdoContainer.name() << " in store " << rdoContainer.store());
    SG::ReadHandle<TgcDigitContainer> container(m_digitContainerKey, ctx);
    ATH_CHECK(container.isPresent());
    ATH_MSG_DEBUG("Found TgcDigitContainer called " << container.name() << " in store " << container.store());

    std::map<uint16_t, std::unique_ptr<TgcRdo>> tgcRdoMap{};

     const TgcRdoIdHash hashF;

    // loop over collections
    for (const TgcDigitCollection* tgcCollection : *container) {

        // Iterate on the digits of the collection
        for (const TgcDigit* tgcDigit : *tgcCollection) {
            const Identifier channelId = tgcDigit->identify();
            const uint16_t bctag = m_isNewTgcDigit ?  tgcDigit->bcTag() : 0;

            // Get the online Id of the channel
            int subDetectorID{0}, rodID{0}, sswID{0}, slbID{0}, channelID{0};

            // repeat two times for Adjacent Channel
            for (int iAd = 0; iAd < 2; ++iAd) {
                bool adFlag = false;

                // check if this channel has Adjacent partner only when 2nd time
                if (iAd != 0) {
                    bool a_found = m_cabling->hasAdjacentChannel(channelId);

                    // set Adjacent flag
                    if (a_found)
                        adFlag = true;
                    else
                        continue;
                }

                // get Online ID
                bool status = m_cabling->getReadoutIDfromOfflineID(channelId, subDetectorID, rodID, sswID, slbID, channelID, adFlag);

                if (!status) {
                    ATH_MSG_DEBUG("ITGCcablingSvc can't return an online ID for the channel : "
                                << MSG::dec << " N_" << m_idHelperSvc->toString(channelId) );
                    continue;
                }

                // Create the new Tgc RawData
                bool isStrip = m_idHelperSvc->tgcIdHelper().isStrip(channelId);
                std::string name = m_idHelperSvc->tgcIdHelper().stationNameString(m_idHelperSvc->tgcIdHelper().stationName(channelId));
                TgcRawData::SlbType type = TgcRawData::SLB_TYPE_UNKNOWN;
                if (name[1] == '4')
                    type = isStrip ? TgcRawData::SLB_TYPE_INNER_STRIP : TgcRawData::SLB_TYPE_INNER_WIRE;
                else if (name[1] == '1')
                    type = isStrip ? TgcRawData::SLB_TYPE_TRIPLET_STRIP : TgcRawData::SLB_TYPE_TRIPLET_WIRE;
                else
                    type = isStrip ? TgcRawData::SLB_TYPE_DOUBLET_STRIP : TgcRawData::SLB_TYPE_DOUBLET_WIRE;

                auto rawData = std::make_unique<TgcRawData>(bctag, subDetectorID, rodID, sswID, slbID, 0, 0, type, adFlag, 0, channelID);

                ATH_MSG_DEBUG("Adding a new RawData");
                ATH_MSG_DEBUG(MSG::hex << " Sub : " << subDetectorID << " ROD : " << rodID << " SSW : " << sswID << " SLB : " << slbID
                                       << " Ch  : " << channelID);

                // Add the RawData to the RDO
                const uint16_t rdoId = identifyFragment(*rawData);
                std::unique_ptr<TgcRdo>& tgcRdo = tgcRdoMap[rdoId];

                if(!tgcRdo) {
                    // create new TgcRdo
                    const  IdentifierHash hashId = hashF(rdoId);
                    tgcRdo = std::make_unique<TgcRdo>(rdoId, hashId);
                    tgcRdo->setOnlineId(rawData->subDetectorId(), rawData->rodId());
                }
                tgcRdo->push_back(std::move(rawData));
            }            
        }
    }

    ATH_MSG_DEBUG("Add RDOs to the RdoContainer");
    // Add RDOs to the RdoContainer
    for (auto&[onlineId, rdo] : tgcRdoMap) {
        unsigned int elementHash = hashF(onlineId);
        ATH_CHECK(rdoContainer->addCollection(rdo.release(), elementHash));
    }
    ATH_MSG_DEBUG("Added RDOs to the RdoContainer XXXXXX");

    return StatusCode::SUCCESS;
}

// NOTE: although this function has no clients in release 22, currently the Run2 trigger simulation is still run in
//       release 21 on RDOs produced in release 22. Since release 21 accesses the TagInfo, it needs to be written to the
//       RDOs produced in release 22. The fillTagInfo() function thus needs to stay in release 22 until the workflow changes
StatusCode TgcDigitToTgcRDO::fillTagInfo() const {
    ServiceHandle<ITagInfoMgr> tagInfoMgr("TagInfoMgr", name());
    if (tagInfoMgr.retrieve().isFailure()) return StatusCode::FAILURE;

    StatusCode sc = tagInfoMgr->addTag("TGC_CablingType", m_cablingType);

    if (sc.isFailure()) {
        ATH_MSG_WARNING("TGC_CablingType " << m_cablingType << " not added to TagInfo ");
        return sc;
    } else {
        ATH_MSG_DEBUG("TGC_CablingType " << m_cablingType << " is Added TagInfo ");
    }

    return StatusCode::SUCCESS;
}

StatusCode TgcDigitToTgcRDO::getCabling() {
    ATH_CHECK(m_tgc_cabling_server->giveCabling(m_cabling));

    int maxRodId, maxSRodId, maxSswId, maxSbloc, minChannelId, maxChannelId;
    m_cabling->getReadoutIDRanges(maxRodId, maxSRodId, maxSswId, maxSbloc, minChannelId, maxChannelId);
    if (maxRodId == 12) {
        ATH_MSG_INFO(m_cabling->name() << " (12-fold) is selected ");
        m_cablingType = "TGCcabling12Svc";
    } else {
        ATH_MSG_INFO("TGCcablingSvc (8-fold) is selected");
        m_cablingType = "TGCcabling8Svc";
    }

    // Fill Tag Info
    ATH_CHECK(fillTagInfo());

    return StatusCode::SUCCESS;
}
