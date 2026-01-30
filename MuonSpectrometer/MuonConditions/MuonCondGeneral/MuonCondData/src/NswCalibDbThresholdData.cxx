/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "Identifier/Identifier.h"

using namespace Muon::MuonStationIndex;
// general functions ---------------------------------
NswCalibDbThresholdData::NswCalibDbThresholdData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"NswCalibDbThresholdData"},
    m_idHelperSvc{idHelperSvc} {}


// setting functions ---------------------------------

// setData
void NswCalibDbThresholdData::setData(const Identifier& chnlId, const float threshold) {
    
    auto insert_itr = m_data.insert(std::make_pair(chnlId, threshold));
    if (!insert_itr.second) {
        ATH_MSG_WARNING("Threshold data for channel "<<m_idHelperSvc->toString(chnlId)
                <<" already set to "<<insert_itr.first->second<<". Cannot apply "<<threshold);
    }
}

// setZero
void NswCalibDbThresholdData::setZero(ThrsldTechType tech, const float threshold) {
    m_zero[toInt(tech)] = threshold;
}


// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier>
NswCalibDbThresholdData::getChannelIds(const std::string tech, const std::string side) const {
    std::vector<Identifier> keys;
    keys.reserve(m_data.size());
    for (const auto& p : m_data) {
        keys.emplace_back(p.first);
    }

    if(tech.empty() && side.empty()) {
        return keys;
    }
    std::vector<Identifier> chnls;
    std::ranges::copy_if(keys, std::back_inserter(chnls), [&](const Identifier& copyMe){
        const int eta = m_idHelperSvc->stationEta(copyMe);
        if (!side.empty() && ((eta < 0 && side == "A") || (eta > 0 && side == "C"))){
            return false;
        }
        return tech.empty() || technologyName(m_idHelperSvc->technologyIndex(copyMe)) == tech;
    });
     return chnls;
}

// getThreshold
std::optional<float> NswCalibDbThresholdData::getThreshold(const Identifier& chnlId) const {
    ChannelMap::const_iterator chan_itr = m_data.find(chnlId);
    /// For the moment require that there is only one channel per identifier
    if(chan_itr != m_data.end()) {
        return chan_itr->second;
    }
    // if channelId doesn't exist in buffer, use 0 channel data ("all channels"), if exists
    const ThrsldTechType tech = (m_idHelperSvc->issTgc(chnlId))? ThrsldTechType::STGC : ThrsldTechType::MM;
    return m_zero[toInt(tech)];
}



