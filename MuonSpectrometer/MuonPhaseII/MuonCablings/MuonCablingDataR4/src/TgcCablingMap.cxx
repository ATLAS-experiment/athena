#include "MuonCablingDataR4/TgcCablingMap.h"
#include "MuonCablingDataR4/TgcCablingData.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonIdHelpers/TgcIdHelper.h"

#include <format>

namespace MuonR4 {

TgcCablingMap::TgcCablingMap(const Muon::IMuonIdHelperSvc* idHelperSvc) :
    m_tgcIdHelper{idHelperSvc->tgcIdHelper()} {}

TgcCablingMap::~TgcCablingMap() = default;

std::ostream& operator<<(std::ostream& ostr,
                         const TgcCablingMap::JsonEntry& obj) {
    ostr << std::format("stationName: {} ", obj.stationName)
         << std::format("stationNameIndex: {} ",
                        obj.stationNameIndex)
         << std::format("eta: {:2d} ", obj.stationEta)
         << std::format("phi: {:2d} ", obj.stationPhi)
         << std::format("gasGap: {:1d} ", obj.gasGap)
         << std::format("isStrip: {:1d} ", obj.isStrip)
         << std::format("ASDstartChannel: {:2d} ",
                        obj.ASDstartChannel)
         << std::format("range: [{:2d}, {:2d}] ",
                        obj.channelRangeStart,
                        obj.channelRangeEnd)
         << std::format("reversed: {:1d} ", obj.reversed)
         << std::format("SLID: {:2d} ", obj.SLID)
         << std::format("cell1: {:4d} ", obj.cellAddress1)
         << std::format("cell2: {:4d} ", obj.cellAddress2)
         << std::format("hasCell2: {:1d}",
                        obj.hasSecondCellAddress);
    return ostr;
}

bool TgcCablingMap::convert(const TgcCablingData& translator,
                            Identifier& id,
                            bool checkValid) const {
    bool valid{!checkValid};

    id = checkValid
       ? m_tgcIdHelper.channelID(translator.stationName,
                                 translator.stationEta,
                                 translator.stationPhi,
                                 translator.gasGap,
                                 translator.isStrip,
                                 translator.channel,
                                 valid)
       : m_tgcIdHelper.channelID(translator.stationName,
                                 translator.stationEta,
                                 translator.stationPhi,
                                 translator.gasGap,
                                 translator.isStrip,
                                 translator.channel);

    return valid;
}

bool TgcCablingMap::convert(const Identifier& channelId,
                            TgcCablingData& translator) const {
    if (!m_tgcIdHelper.is_tgc(channelId)) {
        return false;
    }

    translator.stationName =
        m_tgcIdHelper.stationName(channelId);
    translator.stationEta =
        m_tgcIdHelper.stationEta(channelId);
    translator.stationPhi =
        m_tgcIdHelper.stationPhi(channelId);
    translator.gasGap =
        m_tgcIdHelper.gasGap(channelId);
    translator.isStrip =
        m_tgcIdHelper.isStrip(channelId);
    translator.channel =
        m_tgcIdHelper.channel(channelId);

    return true;
}

bool TgcCablingMap::getReadoutId(TgcCablingData& translatorCache,
                                 MsgStream& log) const {
    const TgcCablingOfflineID& offId{translatorCache};

    const auto range = m_offToReadout.equal_range(offId);

    if (range.first == range.second) {
        log << MSG::DEBUG
            << "TgcCablingMap::" << __func__
            << ": unknown offline identifier " << offId
            << endmsg;
        return false;
    }

    const OfflineToReadoutAssociation& assoc = range.first->second;
    static_cast<TgcCablingReadoutID&>(translatorCache) = assoc.readoutID;

    return true;
}

bool TgcCablingMap::getOfflineId(TgcCablingData& translatorCache,
                                 MsgStream& log) const {
    const TgcCablingReadoutID& readoutId{translatorCache};

    const auto range = m_readoutToOff.equal_range(readoutId);

    if (range.first == range.second) {
        log << MSG::DEBUG
            << "TgcCablingMap::" << __func__
            << ": unknown readout identifier " << readoutId
            << endmsg;
        return false;
    }

    const ReadoutToOfflineAssociation& assoc = range.first->second;
    static_cast<TgcCablingOfflineID&>(translatorCache) = assoc.offlineID;

    return true;
}

bool TgcCablingMap::insertChannels(const JsonEntry& entry,
                                   MsgStream& log) {
    if (entry.channelRangeStart < 1 ||
        entry.channelRangeEnd > 16 ||
        entry.channelRangeStart > entry.channelRangeEnd) {
        log << MSG::ERROR
            << "Invalid channelRangeInASD in " << entry
            << endmsg;
        return false;
    }

    if (entry.cellAddress1 < 0) {
        log << MSG::ERROR
            << "Invalid cellAddress1 in " << entry
            << endmsg;
        return false;
    }

    if (entry.hasSecondCellAddress && entry.cellAddress2 < 0) {
        log << MSG::ERROR
            << "Invalid cellAddress2 in " << entry
            << endmsg;
        return false;
    }

    for (int asdChannel = entry.channelRangeStart;
         asdChannel <= entry.channelRangeEnd;
         ++asdChannel) {
        const int offset = asdChannel - entry.channelRangeStart;
        const int offlineChannel = entry.ASDstartChannel + offset;

        int bitPosition = asdChannel - 1;

        if (entry.reversed) {
            bitPosition = 16 - asdChannel;
        }

        if (bitPosition < 0 || bitPosition >= 16) {
            log << MSG::ERROR
                << "bitPosition out of 16-bit range: "
                << bitPosition << " in " << entry
                << endmsg;
            return false;
        }

        const auto hitBitmap = static_cast<int16_t>(static_cast<uint16_t>(1u << bitPosition));
        bool valid{false};

        Identifier channelId =
            m_tgcIdHelper.channelID(entry.stationNameIndex,
                            entry.stationEta,
                            entry.stationPhi,
                            entry.gasGap,
                            entry.isStrip,
                            offlineChannel,
                            valid);

        if (!valid) {
            log << MSG::ERROR
                << "Invalid TGC identifier from " << entry
                << ", offlineChannel: " << offlineChannel
                << endmsg;
            return false;
        }

        TgcCablingOfflineID offCh{};
        offCh.stationName = m_tgcIdHelper.stationName(channelId);
        offCh.stationEta = m_tgcIdHelper.stationEta(channelId);
        offCh.stationPhi = m_tgcIdHelper.stationPhi(channelId);
        offCh.gasGap = m_tgcIdHelper.gasGap(channelId);
        offCh.isStrip = m_tgcIdHelper.isStrip(channelId);
        offCh.channel = m_tgcIdHelper.channel(channelId);

        TgcCablingReadoutID readoutCh{};
        readoutCh.SLID = entry.SLID;
        readoutCh.cellAddress1 = entry.cellAddress1;
        readoutCh.hitBitmap1 = hitBitmap;

        readoutCh.cellAddress2 = entry.hasSecondCellAddress
                               ? entry.cellAddress2
                               : static_cast<int16_t>(-1);

        readoutCh.hitBitmap2 = entry.hasSecondCellAddress
                             ? hitBitmap
                             : static_cast<int16_t>(0);

        OfflineToReadoutAssociation offToRead{};
        offToRead.readoutID = readoutCh;
        offToRead.ASDstartChannel = entry.ASDstartChannel;
        offToRead.channelRangeStart = entry.channelRangeStart;
        offToRead.channelRangeEnd = entry.channelRangeEnd;
        offToRead.reversed = entry.reversed;

        m_offToReadout.emplace(offCh, offToRead);

        ReadoutToOfflineAssociation readToOff{};
        readToOff.offlineID = offCh;
        readToOff.ASDstartChannel = entry.ASDstartChannel;
        readToOff.channelRangeStart = entry.channelRangeStart;
        readToOff.channelRangeEnd = entry.channelRangeEnd;
        readToOff.reversed = entry.reversed;

        m_readoutToOff.emplace(readoutCh, readToOff);
    }

    return true;
}

bool TgcCablingMap::finalize(MsgStream& log) {
    if (m_offToReadout.empty()) {
        log << MSG::ERROR
            << "TgcCablingMap::finalize() -- No offline-to-readout data has been loaded"
            << endmsg;
        return false;
    }

    if (m_readoutToOff.empty()) {
        log << MSG::ERROR
            << "TgcCablingMap::finalize() -- No readout-to-offline data has been loaded"
            << endmsg;
        return false;
    }

    log << MSG::INFO
        << "TgcCablingMap loaded "
        << m_offToReadout.size()
        << " offline-to-readout entries and "
        << m_readoutToOff.size()
        << " readout-to-offline entries"
        << endmsg;

    return true;
}

}  // namespace Muon
