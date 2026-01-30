/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "Identifier/IdentifierHash.h"
#include "GeoModelKernel/throwExcept.h"
#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& ostr, const NswCalibDbTimeChargeData::CalibConstants& obj) {
    ostr<<"slope: "<<std::setprecision(15)<<obj.slope;//<<" pm "<<std::setprecision(15)<<obj.slopeError;
    ostr<<" intercept: "<<std::setprecision(15)<<obj.intercept;//<<" pm "<<std::setprecision(15)<<obj.interceptError;
    return ostr;
}

// general functions ---------------------------------
NswCalibDbTimeChargeData::NswCalibDbTimeChargeData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"NswCalibDbTimeChargeData"},
    m_idHelperSvc{idHelperSvc} {
        m_pdo_data.resize(m_nStgcElements + m_nMmElements);
        m_tdo_data.resize(m_nStgcElements + m_nMmElements);
}

int NswCalibDbTimeChargeData::identToModuleIdx(const Identifier& chan_id) const{
    const IdentifierHash hash = m_idHelperSvc->detElementHash(chan_id);
    if (m_idHelperSvc->isMM(chan_id)) {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        return static_cast<unsigned>(hash)*(idHelper.gasGapMax()) + (idHelper.gasGap(chan_id) -1);
    } else if (m_idHelperSvc->issTgc(chan_id)) {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        return static_cast<unsigned>(hash)*(idHelper.gasGapMax() * 3 /*3 channel types*/) +
               (idHelper.gasGap(chan_id) -1  + idHelper.gasGapMax() * idHelper.channelType(chan_id)) + m_nMmElements;
    }
    return -1;
}

// setting functions ---------------------------------

// setData
void
NswCalibDbTimeChargeData::setData(CalibDataType type, 
                                  const Identifier& chnlId,  
                                  CalibConstants constants) {
    ChannelCalibMap& calibMap =  type == CalibDataType::PDO ? m_pdo_data : m_tdo_data; 
    
    
    const int array_idx = identToModuleIdx(chnlId);   
    ATH_MSG_VERBOSE("Set "<<(type == CalibDataType::PDO  ? "PDO" : "TDO")<<" calibration constants for channel "
                <<m_idHelperSvc->toString(chnlId)<<", slot: "<< array_idx<<", "<<constants);
    CalibModule& calib_mod = calibMap.at(array_idx);
    const unsigned channel = (m_idHelperSvc->isMM(chnlId) ?
                                    m_idHelperSvc->mmIdHelper().channel(chnlId) :
                                    m_idHelperSvc->stgcIdHelper().channel(chnlId)) -1;
    if (calib_mod.channels.empty()) {
        if (m_idHelperSvc->isMM(chnlId)) {
            const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
            calib_mod.layer_id = idHelper.channelID(chnlId, idHelper.multilayer(chnlId), idHelper.gasGap(chnlId), 1);
        } else if (m_idHelperSvc->issTgc(chnlId)) {
            const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
            calib_mod.layer_id = idHelper.channelID(chnlId, idHelper.multilayer(chnlId), idHelper.gasGap(chnlId), idHelper.channelType(chnlId), 1);
        }
    }
    if (calib_mod.channels.size() <= channel) calib_mod.channels.resize(channel +1);
    if (calib_mod.channels[channel]) {
        THROW_EXCEPTION("setData() -- Cannot overwrite channel "<<m_idHelperSvc->toString(chnlId)
                        <<"Layer ID: "<<m_idHelperSvc->toString(calib_mod.layer_id)<<
                        " "<<(*calib_mod.channels[channel] ));
        return;
    }    
    calib_mod.channels[channel] = std::make_unique<CalibConstants>(std::move(constants));    
}

// setZeroData
void
NswCalibDbTimeChargeData::setZero(CalibDataType type, MuonCond::CalibTechType tech,  CalibConstants constants) {
    using namespace Muon::MuonStationIndex;
    m_zero[toInt(tech)][toInt(type)] = std::move(constants);
}



// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier>
NswCalibDbTimeChargeData::getChannelIds(const CalibDataType type, const std::string& tech, const std::string& side) const {
    std::vector<Identifier> chnls;
    const ChannelCalibMap& calibMap = type == CalibDataType::PDO ? m_pdo_data : m_tdo_data;    
    chnls.reserve(calibMap.size());
    for (const CalibModule& module : calibMap) {
        /// No calibration constants saved here
        if (module.channels.empty()) continue;
        if (side == "A" && m_idHelperSvc->stationEta(module.layer_id) < 0) continue;
        if (side == "C" && m_idHelperSvc->stationEta(module.layer_id) > 0) continue;

        if (m_idHelperSvc->isMM(module.layer_id)) {
            if (tech == "STGC") continue;
            const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
            for (unsigned chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if (!module.channels[chn -1]) continue;
                
                chnls.push_back(idHelper.channelID(module.layer_id, 
                                                   idHelper.multilayer(module.layer_id), 
                                                   idHelper.gasGap(module.layer_id), chn ));
            }
        } else if (m_idHelperSvc->issTgc(module.layer_id)) {
            if (tech == "MM") break;
            const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
            for (unsigned chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if (!module.channels[chn -1]) continue;
                chnls.push_back(idHelper.channelID(module.layer_id, 
                                                   idHelper.multilayer(module.layer_id), 
                                                   idHelper.gasGap(module.layer_id),  
                                                   idHelper.channelType(module.layer_id), chn ));
            }
        }    
    }

    return chnls;
}
const NswCalibDbTimeChargeData::CalibConstants* 
NswCalibDbTimeChargeData::getCalibForChannel(const CalibDataType type, const Identifier& channelId) const {
    const ChannelCalibMap& calibMap =  type == CalibDataType::PDO ? m_pdo_data : m_tdo_data;    
    const int array_idx = identToModuleIdx(channelId);
    if (array_idx < 0){
      ATH_MSG_ERROR("NswCalibDbTimeChargeData::getCalibForChannel: array index is negative.");
      return nullptr;
    }
    const unsigned channel = (m_idHelperSvc->isMM(channelId) ? 
                                  m_idHelperSvc->mmIdHelper().channel(channelId) : 
                                  m_idHelperSvc->stgcIdHelper().channel(channelId)) -1;
    if (calibMap.at(array_idx).channels.size() > channel && calibMap[array_idx].channels[channel]) {
        return calibMap[array_idx].channels[channel].get();
    }
    // search for data for channel zero
    const MuonCond::CalibTechType tech = (m_idHelperSvc->issTgc(channelId))? MuonCond::CalibTechType::STGC : MuonCond::CalibTechType::MM;
    return getZeroCalibChannel(type, tech);        

}
const NswCalibDbTimeChargeData::CalibConstants* 
    NswCalibDbTimeChargeData::getZeroCalibChannel(const CalibDataType type, const MuonCond::CalibTechType tech) const{   
    using namespace Muon::MuonStationIndex;
    return &m_zero[toInt(tech)][toInt(type)];
}


