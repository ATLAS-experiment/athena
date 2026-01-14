/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCablingData/MdtTdcMap.h"

#include "MuonIdHelpers/MdtIdHelper.h"

// constructor
MdtTdcMap::MdtTdcMap(MezzCardPtr mezType, const MdtCablingData& cabling_data)
    : m_statId{cabling_data}, m_mezzCard{std::move(mezType)} {
    for (uint8_t globTube : m_mezzCard->tdcToTubeMap()) {
        if (globTube == NOTSET)
            continue;
        int8_t tube = globTube % m_mezzCard->numTubesPerLayer();
        m_minTube = std::min(m_minTube, tube);
        m_maxTube = std::max(m_maxTube, tube);
    }
    m_minTube += tubeZero();
    m_maxTube += tubeZero();
}

// get the offlineId
bool MdtTdcMap::offlineId(MdtCablingData& cabling_map, MsgStream& log) const {
    if (onlineId() != cabling_map) {
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << cabling_map
                << " does not have the same online Id " << onlineId() << endmsg;
            return false;
        }
    }
    using OfflineCh = MdtMezzanineCard::OfflineCh;
    OfflineCh tubeLayer = m_mezzCard->offlineTube(cabling_map.channelId, log);
    static_cast<MdtCablingOffData&>(cabling_map) = offId();
    cabling_map.tube = tubeZero() + tubeLayer.tube;
    cabling_map.layer = tubeLayer.layer;
    return tubeLayer.isValid;
}
bool MdtTdcMap::onlineId(MdtCablingData& cabling_data, MsgStream& log) const {
    if (offId() != cabling_data) {
        log << MSG::WARNING << "The cabling data " << cabling_data
            << " does not share the same offline id as the Tdc " << offId()
            << ". " << endmsg;
        return false;
    }
    const bool debug = true || log.level() <= MSG::VERBOSE;
    const uint8_t tubeOffSet =
        (tubeZero() - 1) % m_mezzCard->numTubesPerLayer();
    if (debug) {
        log << MSG::VERBOSE << "Try to match " << cabling_data << endmsg;
    }
    if (cabling_data.tube < minTube() || cabling_data.tube > maxTube()) {
        if (debug) {
            log << MSG::VERBOSE << "The requested tube "
                << static_cast<int>(cabling_data.tube) << " is out of range  "
                << static_cast<int>(minTube()) << " -- "
                << static_cast<int>(maxTube()) << endmsg;
        }
        return false;
    }
    cabling_data.channelId = m_mezzCard->tdcChannel(
        cabling_data.layer, cabling_data.tube - tubeOffSet, log);
    static_cast<MdtCablingOnData&>(cabling_data) = m_statId;
    cabling_data.tdcId = moduleId();
    cabling_data.mezzanine_type = m_mezzCard->id();
    if (debug) {
        log << MSG::VERBOSE << "Mapped to " << m_statId << ", channel "
            << static_cast<int>(cabling_data.channelId) << endmsg;
    }
    return cabling_data.channelId != NOTSET;
}
