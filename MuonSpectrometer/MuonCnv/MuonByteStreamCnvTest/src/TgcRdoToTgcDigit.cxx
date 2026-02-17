/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcRdoToTgcDigit.h"


StatusCode TgcRdoToTgcDigit::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_tgcRdoDecoderTool.retrieve());
    ATH_CHECK(m_tgcRdoKey.initialize());
    ATH_CHECK(m_tgcDigitKey.initialize());
    ATH_CHECK(m_cablingKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode TgcRdoToTgcDigit::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("in execute()");
    // retrieve the collection of RDO
    const TgcRdoContainer* rdoContainer{};
    ATH_CHECK(SG::get(rdoContainer, m_tgcRdoKey, ctx));
    ATH_MSG_DEBUG("Retrieved " << rdoContainer->size() << " TGC RDOs.");

    SG::WriteHandle wh_tgcDigit(m_tgcDigitKey, ctx);
    ATH_CHECK(wh_tgcDigit.record(std::make_unique<TgcDigitContainer>(m_idHelperSvc->tgcIdHelper().module_hash_max())));
    ATH_MSG_DEBUG("Decoding TGC RDO into TGC Digit");

  
    TmpDigitContainer_t outDigitContainer{};
    for (const TgcRdo* rdoColl : *rdoContainer) {
        if (!rdoColl->empty()) { 
            ATH_CHECK(decodeTgc(ctx, *rdoColl, outDigitContainer)); 
        }
    }
    for (auto& coll : outDigitContainer){
        if (!coll) {
            continue;
        }
        const IdentifierHash hash = coll->identifierHash();
        ATH_CHECK(wh_tgcDigit->addCollection(coll.release(), hash));
    }

    return StatusCode::SUCCESS;
}

StatusCode TgcRdoToTgcDigit::decodeTgc(const EventContext& ctx,
                                       const TgcRdo& rdoColl, 
                                       TmpDigitContainer_t& outDigitContainer) const {
   
    const Muon::TgcCablingMap* cabling{nullptr};
    ATH_CHECK(SG::get(cabling,  m_cablingKey, ctx));
    ATH_MSG_DEBUG("Number of RawData in this rdo " << rdoColl.size());
    // for each Rdo, loop over RawData, converter RawData to digit
    // retrieve/create digit collection, and insert digit into collection

    std::map<std::vector<uint16_t>, uint16_t> stripMap;

    // TGC2 Endcap Strip OR channel treatement preparation start
    // Signals are ORed as follows:
    // |stationEta|=5, T9, E1 - slbId=16, bit0 of stripSlbBits in this code
    //                        /
    // |stationEta|=4, T8, E2 - slbId=17, bit1 of stripSlbBits in this code
    //                        /
    // |stationEta|=3, T7, E3 - slbId=18, bit2 of stripSlbBits in this code
    //                        /
    // |stationEta|=2, T6, E4 - slbId=19, bit3 of stripSlbBits in this code
    //                        /
    // |stationEta|=1, T4, E5 - slbId=20, bit4 of stripSlbBits in this code
    for (const TgcRawData* rawData : rdoColl) {
        if (rawData->isCoincidence()) continue;                                  // Require hits
        if (rawData->slbType() != TgcRawData::SLB_TYPE_DOUBLET_STRIP) continue;  // Require TGC2 or TGC3
        if (rawData->sswId() == 7) continue;                                     // Exclude Forward
        if (rawData->bitpos() < 112 || rawData->bitpos() > 199) continue;        // Require C, D-input

        std::vector<uint16_t> stripId(5, 0);
        stripId.at(0) = rawData->subDetectorId();
        stripId.at(1) = rawData->rodId();
        stripId.at(2) = rawData->sswId();
        stripId.at(3) = rawData->bitpos();
        stripId.at(4) = rawData->bcTag();
        uint16_t stripSlbBits = 0x1 << (rawData->slbId() - 16);
        std::map<std::vector<uint16_t>, uint16_t>::iterator itMap = stripMap.find(stripId);  // Find correspond channel
        if (itMap == stripMap.end()) {                                                       // This is new one
            stripMap.insert(std::map<std::vector<uint16_t>, uint16_t>::value_type(stripId, stripSlbBits));
        } else {  // This already exists
            itMap->second |= stripSlbBits;
        }
    }
    // Covert to original hit patterns
    for (auto& jtPair : stripMap) {
        if (jtPair.second <= 31) {
            // x    : 5-bit variable
            // f(x) : OR function above, Digit->RDO conversion
            // g(x) : originalHitBits which satisfies f(g(f(x))) = f(x), RDO->Digit conversion
            static constexpr std::array<uint16_t, 32> originalHitBits{//  0   1   2   3   4   5   6   7
                                                         0, 1, 0, 3, 0, 0, 4, 7,
                                                         //  8   9  10  11  12  13  14  15
                                                         0, 0, 0, 0, 8, 9, 12, 15,
                                                         // 16  17  18  19  20  21  22  23
                                                         0, 0, 0, 0, 0, 0, 0, 0,
                                                         // 24  25  26  27  28  29  30  31
                                                         16, 17, 0, 19, 24, 25, 28, 31};
            jtPair.second = originalHitBits[jtPair.second];
        } else {
            jtPair.second = 0;
        }
    }
    // TGC2 Endcap Strip OR channel treatement preparation end

    for (const TgcRawData* rawData : rdoColl) {
        // check Hit or Coincidence
        if (rawData->isCoincidence()) continue;

        // TGC2 Endcap Strip OR channel treatement start
        if (rawData->slbType() == TgcRawData::SLB_TYPE_DOUBLET_STRIP &&  // Require TGC2 or TGC3
            rawData->sswId() != 7 &&                                     // Exclude Forward
            rawData->bitpos() >= 112 && rawData->bitpos() <= 199         // Require C, D-input
        ) {
            std::vector<uint16_t> stripId(5, 0);
            stripId.at(0) = rawData->subDetectorId();
            stripId.at(1) = rawData->rodId();
            stripId.at(2) = rawData->sswId();
            stripId.at(3) = rawData->bitpos();
            stripId.at(4) = rawData->bcTag();
            std::map<std::vector<uint16_t>, uint16_t>::iterator itMap = stripMap.find(stripId);  // Find correspond hit
            if (itMap != stripMap.end()) {
                uint16_t stripSlbBits = 0x1 << (rawData->slbId() - 16);
                if (!(itMap->second & stripSlbBits)) continue;  // This hit is additional.
            }
        }
        // TGC2 Endcap Strip OR channel treatement end

        // repeat two times for ORed channel
        for (int iOr = 0; iOr < 2; ++iOr) {
            // TGC2 Endcap Strip OR channel is not converted.
            if (iOr && rawData->slbType() == TgcRawData::SLB_TYPE_DOUBLET_STRIP) continue;

            bool orFlag = false;

            // check if this channel has ORed partner only when 2nd time
            if (iOr != 0) {
                const bool o_found = cabling->isOredChannel(rawData->subDetectorId(), 
                                                            rawData->rodId(), 
                                                            rawData->sswId(),
                                                            rawData->slbId(), 
                                                            rawData->bitpos());
                // set OR flag
                if (o_found)
                    orFlag = true;
                else
                    continue;
            }

            // get element ID
            Identifier elementId;
            const bool e_found = cabling->getElementIDfromReadoutID(elementId, rawData->subDetectorId(), rawData->rodId(),
                                                                    rawData->sswId(), rawData->slbId(), rawData->bitpos(), 
                                                                    orFlag);

            if (!e_found) {
                bool show_warning_level = true;

                /* One invalid channel in TGC sector A09:
                   sub=103 rod=9 ssw=6 slb=20 bitpos=151 orFlag=0
                   was always seen in 2008 data, at least run 79772 - 91800.
                   bug #48828 */
                if (rawData->subDetectorId() == 103 && rawData->rodId() == 9 && rawData->sswId() == 6 && rawData->slbId() == 20 &&
                    rawData->bitpos() == 151) {
                    show_warning_level = m_show_warning_level_invalid_TGC_A09_SSW6_hit;
                }

                if (show_warning_level || msgLvl(MSG::DEBUG)) {
                    ATH_MSG_WARNING("ElementID not found for "
                                    << " sub=" << rawData->subDetectorId() << " rod=" << rawData->rodId() << " ssw=" << rawData->sswId()
                                    << " slb=" << rawData->slbId() << " bitpos=" << rawData->bitpos() << " orFlag=" << orFlag);
                }
                continue;
            }

            // convert RawData to Digit
            std::unique_ptr<TgcDigit> newDigit(m_tgcRdoDecoderTool->getDigit(ctx, *rawData, orFlag));

            // check if converted correctly
            if (!newDigit) continue;

            // check new element or not
            
            const IdentifierHash coll_hash = m_idHelperSvc->moduleHash(elementId);
            if (coll_hash >= outDigitContainer.size()) {
                outDigitContainer.resize(coll_hash +1u);
            }
            std::unique_ptr<TgcDigitCollection>& collection = outDigitContainer[coll_hash];
            if (!collection) {
                collection = std::make_unique<TgcDigitCollection>(m_idHelperSvc->chamberId(elementId),
                                                                  coll_hash);
            }
            
            // check duplicate digits
            if (!std::ranges::any_of(*collection, [&newDigit](const TgcDigit* digit){
                return newDigit->identify() == digit->identify() && newDigit->bcTag() == digit->bcTag();
            })) {
                collection->push_back(std::move(newDigit));
            }
        }
    }
    return StatusCode::SUCCESS;
}
