/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCablingData/TwinTubeMap.h>

namespace Muon{
    TwinTubeMap::TwinTubeMap(const IMuonIdHelperSvc* idHelperSvc):
        AthMessaging{"TwinTubeMap"},
        m_idHelperSvc{idHelperSvc} {
        m_twinTubesPerRE.resize(m_idHelperSvc->mdtIdHelper().module_hash_max());
    }
    bool TwinTubeMap::isTwinTubeLayer(const Identifier& channelId) const {
        ATH_MSG_VERBOSE("Check whether "<<m_idHelperSvc->toStringDetEl(channelId)<<" is a twin tube layer");
        const IdentifierHash detHash{m_idHelperSvc->moduleHash(channelId)};
        return !m_twinTubesPerRE[detHash].empty();
    }
           
    Identifier TwinTubeMap::twinId(const Identifier& channelId) const {
        ATH_MSG_VERBOSE("Fetch the twin tube id for "<<m_idHelperSvc->toString(channelId));
        const IdentifierHash detHash{m_idHelperSvc->moduleHash(channelId)};
        const Storage& lookUp{m_twinTubesPerRE[detHash]};
        Storage::const_iterator twinItr = lookUp.find(channelId);
        if (twinItr != lookUp.end()) {
            ATH_MSG_VERBOSE("The matching twin tube is "<<m_idHelperSvc->toString(twinItr->second));
            return twinItr->second;
        }
        return channelId;
    }
     StatusCode TwinTubeMap::addTwinPair(const Identifier& sibling1, const Identifier& sibling2) {
        ATH_MSG_DEBUG("Add new twin tube pair "<<m_idHelperSvc->toString(sibling1)<<", "
                    <<m_idHelperSvc->toString(sibling2));
        const IdentifierHash hashSib1{m_idHelperSvc->detElementHash(sibling1)};
        const IdentifierHash hashSib2{m_idHelperSvc->detElementHash(sibling2)};
        if (hashSib1 != hashSib2) {
            ATH_MSG_ERROR("The two identifiers are not from the same readout element "
                        <<m_idHelperSvc->toString(sibling1)<<", "<<m_idHelperSvc->toString(sibling2));
            return StatusCode::FAILURE;
        }
        Storage& insertMe{m_twinTubesPerRE[m_idHelperSvc->moduleHash(sibling1)]};
        if (!insertMe.insert(std::make_pair(sibling1, sibling2)).second ||
           !insertMe.insert(std::make_pair(sibling2, sibling1)).second) {
           ATH_MSG_ERROR("Failed to establish twin tube connection between "
                        <<m_idHelperSvc->toString(sibling1)<<", "<<m_idHelperSvc->toString(sibling2));
           return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    double TwinTubeMap::hvDelayTime(const Identifier& channelId) const {
        const IdentifierHash chambHash = m_idHelperSvc->moduleHash(channelId);
        const HVDelayMap& delayMap{m_hvDelayTimes[chambHash]};
        HVDelayMap::const_iterator itr = delayMap.find(channelId);
        if (itr != delayMap.end()) {
            ATH_MSG_VERBOSE("Non stadnard delay was found for "<<m_idHelperSvc->toString(channelId)
                          <<" to be"<<itr->second);
            return itr->second;
        }
        if (!delayMap.empty() && (itr = delayMap.find(twinId(channelId))) != delayMap.end()) {
           ATH_MSG_VERBOSE("Non standard delay was found for "<<m_idHelperSvc->toString(itr->first)
                          <<" to be"<<itr->second);
            return itr->second;
        }
        return m_defaultHVDelay;
    }

    StatusCode TwinTubeMap::setHVDelay(const Identifier& chId, double hvDelay) {
        const IdentifierHash chambHash = m_idHelperSvc->moduleHash(chId);
        HVDelayMap& delayMap{m_hvDelayTimes[chambHash]};
        if (!delayMap.insert(std::make_pair(chId, hvDelay)).second ||
            std::abs(hvDelayTime(twinId(chId)) - hvDelay) > std::numeric_limits<double>::epsilon()){
            ATH_MSG_FATAL("Failed to insert hv delay "<<hvDelay <<" for "<<m_idHelperSvc->toString(chId)
                        <<" as "<<delayMap[chId]<<" has been added before");
        }
        return StatusCode::SUCCESS;
    }
           
    void TwinTubeMap::setDefaultHVDelay(const double hvDelay) {
        ATH_MSG_DEBUG("Set the default HV delay to "<<hvDelay<<" ns.");
        m_defaultHVDelay = hvDelay;
    }

}

     