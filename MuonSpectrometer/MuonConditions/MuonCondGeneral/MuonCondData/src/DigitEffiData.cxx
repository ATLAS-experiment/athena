/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondData/DigitEffiData.h"
#include "GeoModelHelpers/throwExcept.h"

namespace Muon{
    DigitEffiData::DigitEffiData(const Muon::IMuonIdHelperSvc* idHelperSvc, double defaultEffi):
        AthMessaging{"DigitEffiData"},
        m_idHelperSvc{idHelperSvc},
        m_defaultEffi{defaultEffi}{}

    Identifier DigitEffiData::getLookUpId(const Identifier& channelId) const {
        Identifier lookUpId{};
        using TechIndex = Muon::MuonStationIndex::TechnologyIndex;
        switch (m_idHelperSvc->technologyIndex(channelId)){
            case TechIndex::MDT:
                lookUpId = channelId;
                break;
            case TechIndex::RPC:
            case TechIndex::TGC:
            case TechIndex::CSCI:
                lookUpId = m_idHelperSvc->gasGapId(channelId);
                break;
            case TechIndex::MM:
                lookUpId = m_idHelperSvc->mmIdHelper().pcbID(channelId);
                break;
            case TechIndex::STGC:
                lookUpId = m_idHelperSvc->stgcIdHelper().febID(channelId);
                break;
            default:
                THROW_EXCEPTION("Invalid muon technology");
        };
        return lookUpId;
    }
    double DigitEffiData::getEfficiency(const Identifier& channelId) const {
        EffiMap::const_iterator effi_itr = m_effiData.find(getLookUpId(channelId));
        if (effi_itr != m_effiData.end()) {
            ATH_MSG_VERBOSE("Channel "<<m_idHelperSvc->toString(channelId) <<" has an efficiency of "<<effi_itr->second);
            return effi_itr->second;
        }
        ATH_MSG_VERBOSE("Efficiency of channel "<<m_idHelperSvc->toString(channelId)<<" is unknown. Return 1.");
        return 1.;
    }

    StatusCode DigitEffiData::setEfficiency(const Identifier& channelId, const double effi){
        const Identifier gasGapId = getLookUpId(channelId);
        auto insert_itr = m_effiData.insert(std::make_pair(gasGapId, effi));
        if (!insert_itr.second) {
            ATH_MSG_ERROR("An efficiency for gasGap "<<m_idHelperSvc->toStringGasGap(gasGapId)
            <<" has already been stored "<<m_effiData[gasGapId]<<" vs. "<<effi);
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
}