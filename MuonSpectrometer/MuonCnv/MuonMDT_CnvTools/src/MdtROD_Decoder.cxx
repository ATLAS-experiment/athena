/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// The data format is described in ATL-DAQ-2003-023 / ATL-COM-MUON-2003-011
// https://cds.cern.ch/record/685552
// http://www.nikhef.nl/pub/experiments/atlas/daq/mrod/mrodprr/daq-2003-023.pdf

#include "MdtROD_Decoder.h"

#include <algorithm>

#include "eformat/Issue.h"

using eformat::helper::SourceIdentifier;

static const InterfaceID IID_IMdtROD_Decoder("MdtROD_Decoder", 1, 0);

/** constructor
 */
MdtROD_Decoder::MdtROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<MdtROD_Decoder>(this);

    declareProperty("SpecialROBNumber", m_specialROBNumber = -1);
}

const InterfaceID& MdtROD_Decoder::interfaceID() { return IID_IMdtROD_Decoder; }

StatusCode MdtROD_Decoder::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());

    // Here the mapping service has to be initialized
    m_hid2re = std::make_unique<MDT_Hid2RESrcID>();
    ATH_CHECK(m_hid2re->set(&m_idHelperSvc->mdtIdHelper()));
    ATH_CHECK(m_readKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MdtROD_Decoder::finalize() {
    if (m_nCache > 0 || m_nNotCache > 0) {
        const float cacheFraction = ((float)m_nCache) / ((float)(m_nCache + m_nNotCache));
        ATH_MSG_INFO("Fraction of fills that use the cache = " << cacheFraction);
    }

    return StatusCode::SUCCESS;
}

StatusCode MdtROD_Decoder::fillCollections(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag, MdtCsmContainer& rdoIDC) const {
#define WARNING_WITH_LINE(msg) ATH_MSG_WARNING(__func__ << "():" << __LINE__<< " " << msg)
    //  m_debug = (m_log.level() <= MSG::DEBUG); // use to control if output debug info.
    //  decoding classes
    MdtCsmReadOut csmReadOut;
    MdtAmtReadOut amtReadOut;
    MdtHptdcReadOut hptdcReadOut;

    ATH_MSG_DEBUG("fillCollection: starting");
    uint32_t wordPos = 0;

    // Check the ROB and ROD fragment for lenght and version consistency. E.P.

    try {
        robFrag.check();
    } catch (const eformat::Issue& ex) {
        WARNING_WITH_LINE(ex.what());
        return StatusCode::FAILURE;  // error in fragment - we search for no collection
    }

    // Check the ROB status word for truncation or corruption E.P.

    uint32_t nstat = robFrag.nstatus();

    if (nstat) {
        const uint32_t* it{nullptr};
        robFrag.status(it);

        if (*it) {
            WARNING_WITH_LINE("Error in ROB status word: 0x" << std::hex << *it << std::dec);

            // the status word analysis in case of eformat version > 3.0
            // if ( (robFrag.version() >> 16) > 0x0300 ) {

            // various checks to be defined

            //}
            return StatusCode::FAILURE;
        }
    }

    // here go the checks on the ROD header

    const unsigned int size = robFrag.rod_ndata();  // Think this is right - it was size() before. Ed.  // Yes it is... Enrico

    // FIXME - Removed debug output for the moment. EJWM
    //   if (m_debug) log << MSG::DEBUG
    //       << "Collection identifier : " << csmOfflineId.getString() << endmsg;
    //
    //   if (m_debug) log << MSG::DEBUG
    //               << "**********Decoder dumping the words******** " << endmsg;
    //   if (size > 0) {
    //     log << MSG::DEBUG << "The size of this ROD-read is " << size << endmsg;
    //     for (unsigned int i=0; i < size; i++)
    //       if (m_debug) log << MSG::DEBUG << "word " << i
    //                           << " = " << MSG::hex << robFrag[i] << MSG::dec << endmsg;
    //   }
    //   else {
    //       if (m_debug) log << MSG::DEBUG << " Buffer size 0 ! "
    //                                           << endmsg;
    //       return;
    //   }

    leading_amt_map leadingHitMap{};

    // Decode the SourceId from the ROD Header
    SourceIdentifier sid(robFrag.rod_source_id());

    MuonMDT_CablingMap::CablingData cabling_data{};
    cabling_data.subdetectorId = sid.subdetector_id();
    cabling_data.mrod = sid.module_id();

    // Get the first word of a buffer:
    OFFLINE_FRAGMENTS_NAMESPACE::PointerType vint;
    robFrag.rod_data(vint);

    ATH_MSG_DEBUG("**********Decoder dumping the words******** ");

    if (size > 0) {
        ATH_MSG_DEBUG("The size of this ROD-read is ");
        for (unsigned int i = 0; i < size; i++) ATH_MSG_DEBUG("word " << i << " = " << MSG::hex << vint[i] << MSG::dec);
    } else {
        WARNING_WITH_LINE("Buffer size 0 ! ");
        return StatusCode::FAILURE;
    }

    csmReadOut.decodeWord(vint[wordPos]);

    if (csmReadOut.is_BOB()) {
        ATH_MSG_DEBUG("Found the beginning of buffer ");
        // Check that Lvl1d matches the one from the ROD header
        ATH_MSG_DEBUG("Level 1 Id : " << csmReadOut.lvl1Id());
    } else {
        ATH_MSG_WARNING(" Beginning of block not found ! ");
        ATH_MSG_WARNING(" Subdetector, ROD ID: 0x" << MSG::hex << cabling_data.subdetectorId << MSG::dec << ", 0x" << cabling_data.mrod
                                                 << MSG::dec);
    }

    if (size < 2) {
      WARNING_WITH_LINE("Too few ROD words: " << size);
      return StatusCode::FAILURE;
    }
    ++wordPos;
    csmReadOut.decodeWord(vint[wordPos]);
    if (csmReadOut.is_EOB()) {
      ATH_MSG_WARNING("No CSM data ROD ID: 0x" << MSG::hex << cabling_data.subdetectorId <<
                      ", 0x" << static_cast<unsigned int>(cabling_data.mrod) << MSG::dec);
      return StatusCode::SUCCESS;
    }

    SG::ReadCondHandle<MuonMDT_CablingMap> readHandle{m_readKey};
    const MuonMDT_CablingMap* readCdo{*readHandle};
    if (!readCdo) {
        WARNING_WITH_LINE("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    }
    auto& msg = msgStream();
    while (!csmReadOut.is_EOB()) {
        while ((!csmReadOut.is_BOL()) && (!csmReadOut.is_EOB())) {
            wordPos += 1;
            if (wordPos >= size) {
                WARNING_WITH_LINE("failure: data corrupted");
                return StatusCode::FAILURE;
            }
            csmReadOut.decodeWord(vint[wordPos]);
        }

        if (csmReadOut.is_BOL()) {
            ATH_MSG_DEBUG("Found the Beginnning of Link ");
        } else if (csmReadOut.is_EOB()) {
            WARNING_WITH_LINE(" failure: collection not found");
            return StatusCode::FAILURE;
        }

        // uint16_t subdetId = 0x61;
        // uint16_t mrodId = csmReadOut.mrodId();

        cabling_data.csm = csmReadOut.csmId();

        // Get the offline identifier from the MDT cabling service
        // TDC and Tube identifier are dummy, only the chamber map is needed here

        ATH_MSG_DEBUG("subdetId : " << cabling_data.subdetectorId << " "
                                    << "mrodId : " << cabling_data.mrod << " "
                                    << "csmId : " << cabling_data.csm);

        cabling_data.tdcId = 0xff;
        cabling_data.channelId = 0xff;

        Identifier moduleId{0};
        IdentifierHash idHash{0};
        bool cab = readCdo->getOfflineId(cabling_data, msg) && readCdo->getMultiLayerCode(cabling_data, moduleId, idHash, msg);

        ATH_MSG_DEBUG("getOfflineIdfromOnlineID result: " << cabling_data);

        if (!cab) {
            ATH_MSG_WARNING("Cabling not understood");
            ATH_MSG_WARNING("Skip decoding of CSM link, subdetId : " << MSG::hex << std::showbase << cabling_data.subdetectorId << " "
                                                                     << "mrodId : " << cabling_data.mrod << " " << MSG::dec
                                                                     << "csmId : " << cabling_data.csm);
            // assert(false);
            // continue;  // Go to next link instead of crashing (here or later) E.P.

            // In case of failure this produce an infinite loop because
            // you do not decode the next word before doing "continue".
            // I commented it out.

            // The previous implementation was ok, the bad BOL was simply
            // skipped. This means that you skip decoding this chamber.

            // I'm not sure how you can recover this error. I think you
            // simple have to store the error condition in the
            // StatusCode word returned to the client.
            //    } else if (!isValid) {
            // ATH_MSG_DEBUG("Offline identifier returned by MdtIdHelper is not valid");
            // ATH_MSG_DEBUG("Skip decoding of CSM link, subdetId : "
            //		    << MSG::hex << std::showbase << subdetId << " "
            //		    << "mrodId : " << mrodId << " " << MSG::dec
            //		    << "csmId : " << csmId);

        } else {
            // EJWM Removed: if ( moduleId == csmOfflineId)
            // Can't see how to keep that in here now, and also not really sure what the point of it is
            // Might be wrong though - job for experts to look into.
            // FIXME

            const bool isHpTdc = m_idHelperSvc->hasHPTDC(moduleId);

            // Create MdtCsm and try to get it from the cache via the IDC_WriteHandle
            std::unique_ptr<MdtCsm> collection{nullptr};
            MdtCsmContainer::IDC_WriteHandle lock = rdoIDC.getWriteHandle(idHash);
            if (lock.alreadyPresent()) {
                ATH_MSG_DEBUG("collections already found, do not convert");
                ++m_nCache;
            } else {
                ATH_MSG_DEBUG(" Collection ID = " << m_idHelperSvc->toString(moduleId) << " does not exist, create it ");
                collection = std::make_unique<MdtCsm>(moduleId, idHash);
                ++m_nNotCache;
            }

            // Set values (keep Identifier and IdentifierHash the same though)
            if (collection)
                collection->set_values(collection->identify(), collection->identifyHash(), cabling_data.subdetectorId, cabling_data.mrod,
                                       cabling_data.csm);

            wordPos += 1;
            if (wordPos >= size) {
                WARNING_WITH_LINE("failure: data corrupted");
                return StatusCode::FAILURE;
            }
            csmReadOut.decodeWord(vint[wordPos]);
            if (!csmReadOut.is_TLP()) { ATH_MSG_DEBUG("failure: TDC Link Present not found "); }

            // Loop on the TDCs blocks, if present
            wordPos += 1;
            if (wordPos >= size) {
                WARNING_WITH_LINE("failure: data corrupted");
                return StatusCode::FAILURE;
            }
            isHpTdc ? hptdcReadOut.decodeWord(vint[wordPos]) : amtReadOut.decodeWord(vint[wordPos]);
            csmReadOut.decodeWord(vint[wordPos]);
            while (!csmReadOut.is_TWC()) {
                uint16_t tdcNum = isHpTdc ? hptdcReadOut.tdcId() : amtReadOut.tdcId();

                ATH_MSG_DEBUG(" Decoding data from TDC number : " << tdcNum);

                // increase word count by one
                wordPos += 1;
                if (wordPos >= size) {
                    WARNING_WITH_LINE("failure: data corrupted");
                    return StatusCode::FAILURE;
                }

                isHpTdc ? hptdcReadOut.decodeWord(vint[wordPos]) : amtReadOut.decodeWord(vint[wordPos]);

                std::unique_ptr<MdtAmtHit> amtHit{nullptr};
                // Loop on the TDC data words and create the corresponding
                // RDO's
                while (!((isHpTdc ? hptdcReadOut.is_EOT() : amtReadOut.is_EOT()) ||
                         (isHpTdc ? hptdcReadOut.is_BOT() : amtReadOut.is_BOT()) ||
                         (isHpTdc ? csmReadOut.is_TWC() : amtReadOut.is_TWC()))) {
                    isHpTdc ? hptdcReadOut.decodeWord(vint[wordPos]) : amtReadOut.decodeWord(vint[wordPos]);
                    int tdcCounts;
                    uint16_t chanNum;
                    // decode the tdc channel number

                    // Check whether this channel has already been
                    // created for this CSM

                    if ((isHpTdc ? hptdcReadOut.is_TSM() : amtReadOut.is_TSM()) &&
                        (isHpTdc ? hptdcReadOut.isLeading() : amtReadOut.isLeading()) && collection) {
                        chanNum = isHpTdc ? hptdcReadOut.channel() : amtReadOut.channel();

                        amtHit = std::make_unique<MdtAmtHit>(tdcNum, chanNum);
                        amtHit->setValues((isHpTdc ? hptdcReadOut.coarse() : amtReadOut.coarse()),
                                          (isHpTdc ? hptdcReadOut.fine() : amtReadOut.fine()), 0);
                        amtHit->addData(vint[wordPos]);
                        leadingHitMap.insert(std::make_pair(chanNum, std::move(amtHit)));
                    } else if ((isHpTdc ? hptdcReadOut.is_TSM() : amtReadOut.is_TSM()) &&
                               !(isHpTdc ? hptdcReadOut.isLeading() : amtReadOut.isLeading()) && collection) {
                        chanNum = isHpTdc ? hptdcReadOut.channel() : amtReadOut.channel();
                        leading_amt_map::iterator chanPosition = leadingHitMap.find(chanNum);

                        if (chanPosition != leadingHitMap.end()) {
                            // get the amt hit corresponding to the first data word
                            amtHit = std::move(chanPosition->second);
                            uint16_t coarse = amtHit->coarse();
                            uint16_t fine = amtHit->fine();
                            int tdcCountsFirst = coarse * 32 + fine;

                            // get the tdc counts of the current data word
                            tdcCounts =
                                isHpTdc ? hptdcReadOut.coarse() * 32 + hptdcReadOut.fine() : amtReadOut.coarse() * 32 + amtReadOut.fine();
                            int width = tdcCounts - tdcCountsFirst;

                            amtHit->setValues(coarse, fine, width);
                            amtHit->addData(vint[wordPos]);
                            collection->push_back(std::move(amtHit));

                            // delete the channel from the map
                            leadingHitMap.erase(chanPosition);
                        } else {
                            ATH_MSG_DEBUG("failure: corresponding leading edge not found for the trailing edge tdc: " << tdcNum << " chan: "
                                                                                                                    << chanNum);
                        }

                    } else if ((isHpTdc ? hptdcReadOut.is_TCM() : amtReadOut.is_TCM()) && collection) {
                        uint16_t chanNum = isHpTdc ? hptdcReadOut.channel() : amtReadOut.channel();

                        amtHit = std::make_unique<MdtAmtHit>(tdcNum, chanNum);
                        amtHit->setValues((isHpTdc ? hptdcReadOut.coarse() : amtReadOut.coarse()),
                                          (isHpTdc ? hptdcReadOut.fine() : amtReadOut.fine()),
                                          (isHpTdc ? hptdcReadOut.width() : amtReadOut.width()));
                        amtHit->addData(vint[wordPos]);
                        collection->push_back(std::move(amtHit));
                    }

                    // increase word count by one
                    wordPos += 1;
                    if (wordPos >= size) {
                        WARNING_WITH_LINE("failure: data corrupted");
                        return StatusCode::FAILURE;
                    }

                    isHpTdc ? hptdcReadOut.decodeWord(vint[wordPos]) : amtReadOut.decodeWord(vint[wordPos]);

                    if (isHpTdc) csmReadOut.decodeWord(vint[wordPos]);

                }  // End of loop on AMTs

                if (collection) {
                    for (auto& itHit : leadingHitMap) { collection->push_back(std::move(itHit.second)); }
                }
                leadingHitMap.clear();

                // increase the decoding position only if it's end of TDC
                // i.e. not operating in TDC trailer suppression mode
                if ((isHpTdc ? hptdcReadOut.is_EOT() : amtReadOut.is_EOT())) {
                    wordPos += 1;
                    if (wordPos >= size) {
                        WARNING_WITH_LINE("failure: data corrupted");
                        return StatusCode::FAILURE;
                    }
                }
                csmReadOut.decodeWord(vint[wordPos]);
                isHpTdc ? hptdcReadOut.decodeWord(vint[wordPos]) : amtReadOut.decodeWord(vint[wordPos]);
            }  // End of loop on TDCs
            if (collection) ATH_CHECK(lock.addOrDelete(std::move(collection)));
            // Collection has been found, go out
            // return;
        }  // Check for the chamber offline id = collection offline id
        wordPos += 1;
        if (wordPos >= size) {
            WARNING_WITH_LINE("Data corrupted");
            return StatusCode::FAILURE;
        }
        csmReadOut.decodeWord(vint[wordPos]);
    }
    return StatusCode::SUCCESS;
}
