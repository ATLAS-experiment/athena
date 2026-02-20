/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcRDO_Decoder.h"

#include "Identifier/Identifier.h"
#include "MuonDigitContainer/TgcDigit.h"
#include "MuonRDO/TgcRawData.h"

namespace Muon {
// constructor
StatusCode TgcRDO_Decoder::initialize() {
    ATH_MSG_DEBUG("TgcRDO_Decoder::initialize");

    // try to configure the cabling service
    ATH_CHECK(m_cablingKey.initialize());

    return StatusCode::SUCCESS;
}

void TgcRDO_Decoder::applyPatch(bool patch) {
    m_applyPatch = patch;
}

std::unique_ptr<TgcDigit> TgcRDO_Decoder::getDigit(const EventContext& ctx,
                                                   const TgcRawData& rawData,
                                                   bool orFlag) const {
    // MuonTGC_CablingSvc should be configured at initialise
    const TgcCablingMap* cabling{nullptr};
    if (!SG::get(cabling, m_cablingKey, ctx).isSuccess()) {
        ATH_MSG_ERROR(
            "Cabling is not available in "
            "TgcRDO_Decoder::getDigit()");
        return nullptr;
    }

    int offset = 0, offsetORed = 0;

    const uint16_t sswId = rawData.sswId();
    const uint16_t slbId = rawData.slbId();
    const uint16_t bitpos = rawData.bitpos();

    if (m_applyPatch) {
        if (sswId == 9) {
            if ((slbId % 4 == 3 && bitpos >= 77 && bitpos <= 84) ||
                (slbId % 4 != 3 && bitpos >= 77 && bitpos <= 111)) {
                offset = 1;
            }
        } else if (sswId >= 3 && sswId <= 8) {
            if ((slbId == 1 && bitpos >= 66 && bitpos <= 73) ||
                (slbId == 2 && bitpos >= 42 && bitpos <= 63)) {
                offset = 36;
            }
            if ((slbId == 1 && bitpos >= 102 && bitpos <= 109) ||
                (slbId == 2 && bitpos >= 78 && bitpos <= 99)) {
                offset = -36;
            }
            if ((slbId == 1 && bitpos >= 74 && bitpos <= 75) ||
                (slbId == 2 && bitpos >= 40 && bitpos <= 41)) {
                offsetORed = 36;
            }
            if ((slbId == 1 && bitpos >= 110 && bitpos <= 111) ||
                (slbId == 2 && bitpos >= 76 && bitpos <= 77)) {
                offsetORed = -36;
            }
        }
    }
    int corr = orFlag ? offsetORed : offset;

    // get official channel ID
    Identifier chanId;
    bool c_found = cabling->getOfflineIDfromReadoutID(
        chanId, rawData.subDetectorId(), rawData.rodId(), sswId, slbId,
        bitpos + corr, orFlag);

    if (!c_found) {
        return nullptr;
    }

    return std::make_unique<TgcDigit>(chanId, rawData.bcTag());
}

Identifier TgcRDO_Decoder::getOfflineData(const EventContext& ctx,
                                          const TgcRawData& rawData,
                                          bool orFlag, uint16_t& bctag) const {
    Identifier chanId{};

    // ITGCcablingSvc should be configured at initialise
    const TgcCablingMap* cabling{nullptr};
    if (!SG::get(cabling, m_cablingKey, ctx).isSuccess()) {
        ATH_MSG_ERROR(
            "Cabling is not available in "
            "TgcRDO_Decoder::getOfflineData()");
        return chanId;
    }

    bctag = TgcDigit::BC_UNDEFINED;

    const uint16_t sswId = rawData.sswId();
    const uint16_t slbId = rawData.slbId();
    const uint16_t bitpos = rawData.bitpos();

    int offset = 0, offsetORed = 0;
    if (m_applyPatch) {
        if (sswId == 9) {
            if ((slbId % 4 == 3 && bitpos >= 77 && bitpos <= 84) ||
                (slbId % 4 != 3 && bitpos >= 77 && bitpos <= 111)) {
                offset = 1;
            }
        } else if (sswId >= 3 && sswId <= 8) {
            if ((slbId == 1 && bitpos >= 66 && bitpos <= 73) ||
                (slbId == 2 && bitpos >= 42 && bitpos <= 63)) {
                offset = 36;
            }
            if ((slbId == 1 && bitpos >= 102 && bitpos <= 109) ||
                (slbId == 2 && bitpos >= 78 && bitpos <= 99)) {
                offset = -36;
            }
            if ((slbId == 1 && bitpos >= 74 && bitpos <= 75) ||
                (slbId == 2 && bitpos >= 40 && bitpos <= 41)) {
                offsetORed = 36;
            }
            if ((slbId == 1 && bitpos >= 110 && bitpos <= 111) ||
                (slbId == 2 && bitpos >= 76 && bitpos <= 77)) {
                offsetORed = -36;
            }
        }
    }
    int corr = orFlag ? offsetORed : offset;

    // get official channel ID
    bool c_found = cabling->getOfflineIDfromReadoutID(
        chanId, rawData.subDetectorId(), rawData.rodId(), sswId, slbId,
        bitpos + corr, orFlag);

    if (!c_found) {
        return chanId;
    }

    bctag = rawData.bcTag();
    return chanId;
}
}  // namespace Muon