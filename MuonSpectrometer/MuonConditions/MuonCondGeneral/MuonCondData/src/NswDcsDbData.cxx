/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/NswDcsDbData.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "Identifier/Identifier.h"

#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"


#include "MuonNSWCommonDecode/MapperSTG.h"
#include "MuonNSWCommonDecode/MapperMMG.h"




// general functions ---------------------------------

NswDcsDbData::NswDcsDbData(const MmIdHelper& mmIdHelper, const sTgcIdHelper& stgcIdHelper, const MuonGM::MuonDetectorManager* muonGeoMgr):
    m_mmIdHelper(mmIdHelper),
    m_stgcIdHelper(stgcIdHelper),
    m_muonGeoMgr(muonGeoMgr)
{
}

std::ostream& operator<<(std::ostream& ostr, const NswDcsDbData::TDaqConstants& obj) {
    ostr  << " timeSince " << obj.timeSince << " timeUntil " << obj.timeUntil << " elink " << obj.elink;
    return ostr;
}

unsigned int NswDcsDbData::identToModuleIdx(const Identifier& chan_id) const{
    if (m_mmIdHelper.is_mm(chan_id)) {
        IdentifierHash hash{0};
        if (m_mmIdHelper.get_detectorElement_hash(chan_id, hash) || hash >= m_mmIdHelper.detectorElement_hash_max()){
            throw std::runtime_error("NswDcsDbData() - Failed to retrieve valid  micromega hash ");
        }
        return static_cast<unsigned int>(hash)*(m_mmIdHelper.gasGapMax()) + (m_mmIdHelper.gasGap(chan_id) -1);
    } else if (m_stgcIdHelper.is_stgc(chan_id)) {        
        IdentifierHash hash{0};
        if (m_stgcIdHelper.get_detectorElement_hash(chan_id, hash) || hash >= m_stgcIdHelper.detectorElement_hash_max()){
            throw std::runtime_error("NswDcsDbData() - Failed to retrieve valid stgc hash ");
        }
        return static_cast<unsigned int>(hash)*(m_stgcIdHelper.gasGapMax()) + (m_stgcIdHelper.gasGap(chan_id) -1);
    }
    throw std::runtime_error("NswDcsDbData() - No NSW identifier");
    return -1;
 }

// setting functions ---------------------------------

// setDataHv
void
NswDcsDbData::setDataHv(const DcsTechType tech, const Identifier& chnlId, DcsConstants constants) {
    if(tech == DcsTechType::MMG || tech == DcsTechType::MMD) {
        ChannelDcsMap& dcsMap = tech == DcsTechType::MMG ? m_data_hv_mmg : m_data_hv_mmd;
        const unsigned int array_idx = identToModuleIdx(chnlId);
        if (array_idx >= dcsMap.size()) dcsMap.resize(array_idx + 1);
        DcsModule& dcs_mod = dcsMap[array_idx];
        const unsigned int channel = m_mmIdHelper.channel(chnlId)-1;
        if(dcs_mod.channels.empty())
            dcs_mod.layer_id = m_mmIdHelper.channelID(chnlId, m_mmIdHelper.multilayer(chnlId), m_mmIdHelper.gasGap(chnlId), 1);
        if(dcs_mod.channels.size() <= channel) dcs_mod.channels.resize(channel +1);
        if(dcs_mod.channels[channel]) {
            throw std::runtime_error("NswDcsDbData::setData() -- Cannot overwrite channel");
            return;
        }
        dcs_mod.channels[channel] = std::make_unique<DcsConstants>(std::move(constants));
    } else if(tech == DcsTechType::STG) {
        ChannelDcsMap& dcsMap = m_data_hv_stg;
        const unsigned int array_idx = identToModuleIdx(chnlId); 
        if (array_idx >= dcsMap.size()) dcsMap.resize(array_idx + 1);
        DcsModule& dcs_mod = dcsMap.at(array_idx);
        const unsigned int channel = m_stgcIdHelper.channel(chnlId)-1;
        if(dcs_mod.channels.empty()) {
            dcs_mod.layer_id = m_stgcIdHelper.channelID(chnlId, m_stgcIdHelper.multilayer(chnlId), m_stgcIdHelper.gasGap(chnlId), m_stgcIdHelper.channelType(chnlId), 1);
        }
        if(dcs_mod.channels.size() <= channel) dcs_mod.channels.resize(channel +1);
        if(dcs_mod.channels[channel]) {
            throw std::runtime_error("setData() -- Cannot overwrite channel");
            return;
        }
        dcs_mod.channels[channel] = std::make_unique<DcsConstants>(std::move(constants));
    }
}

// setDataTDaq
void
NswDcsDbData::setDataTDaq(const DcsTechType tech, const Identifier& chnlId, uint64_t timeSince, uint64_t timeUntil, unsigned int elink, bool permanentlyDisabled) {
    ChannelTDaqMap& data = tech == DcsTechType::MMG ? m_data_tdaq_mmg : m_data_tdaq_stg;
    const unsigned int array_idx = identToModuleIdx(chnlId);
    if (array_idx >= data.size()) data.resize(array_idx + 1);
    TDaqConstants x;
    x.timeSince = timeSince;
    x.timeUntil = timeUntil;
    x.elink = elink;
    x.permanentlyDisabled = permanentlyDisabled;
    data[array_idx][chnlId].insert(x);
}

void
NswDcsDbData::setDataEltx(const DcsTechType tech, const Identifier& channelId) {
    ChannelEltxMap& data = tech == DcsTechType::MMG ? m_data_eltx_mmg : m_data_eltx_stg;
    const uint array_idx = identToModuleIdx(channelId);
    if (array_idx >= data.size()) data.resize(array_idx + 1);
    data[array_idx].insert(channelId);
}



// retrieval functions -------------------------------

// getChannelIds
std::vector<Identifier>
NswDcsDbData::getChannelIdsHv(const DcsTechType tech, const std::string& side) const {
    std::vector<Identifier> chnls;
    if(tech == DcsTechType::MMG || tech == DcsTechType::MMD){
        const ChannelDcsMap& dcsMap = tech == DcsTechType::MMG ? m_data_hv_mmg : m_data_hv_mmd;
        chnls.reserve(dcsMap.size());
        for(const DcsModule& module : dcsMap) {
            if(module.channels.empty()) continue;
            if(side == "A" && m_mmIdHelper.stationEta(module.layer_id) < 0) continue;
            if(side == "C" && m_mmIdHelper.stationEta(module.layer_id) > 0) continue;
            for(unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if(!module.channels[chn -1]) continue;
                chnls.push_back(m_mmIdHelper.channelID(module.layer_id, m_mmIdHelper.multilayer(module.layer_id), m_mmIdHelper.gasGap(module.layer_id), chn ));
            }
        }
    } else if(tech == DcsTechType::STG){
        const ChannelDcsMap& dcsMap = m_data_hv_stg;
        chnls.reserve(dcsMap.size());
        for(const DcsModule& module : dcsMap) {
            if(module.channels.empty()) continue;
            if(side == "A" && m_stgcIdHelper.stationEta(module.layer_id) < 0) continue;
            if(side == "C" && m_stgcIdHelper.stationEta(module.layer_id) > 0) continue;
            for(unsigned int chn = 1 ; chn <= module.channels.size() ; ++chn) {
                if(!module.channels[chn -1]) continue;
                chnls.push_back(m_stgcIdHelper.channelID(module.layer_id, m_stgcIdHelper.multilayer(module.layer_id), 
                                        m_stgcIdHelper.gasGap(module.layer_id),  m_stgcIdHelper.channelType(module.layer_id), chn ));
            }
        }
    }
    return chnls;
}

const NswDcsDbData::DcsConstants* 
NswDcsDbData::getDataForChannelHv(const DcsTechType tech, const Identifier& channelId, bool issTgcQ1OuterHv) const {
    if(tech == DcsTechType::MMG){
        if(!m_mmIdHelper.is_mm(channelId)) return nullptr;
        Identifier dcsChannelIdStripHv = m_mmIdHelper.pcbID(channelId);
        const ChannelDcsMap& dcsMap = m_data_hv_mmg; // later add something like: type == DcsDataType::HV ? m_data_hv : m_data_lv;
        const unsigned int array_idx = identToModuleIdx(dcsChannelIdStripHv);
        const unsigned int channel = m_mmIdHelper.channel(dcsChannelIdStripHv) -1;
        if (dcsMap.size() > array_idx && dcsMap.at(array_idx).channels.size() > channel && dcsMap[array_idx].channels[channel]) return dcsMap[array_idx].channels[channel].get();
    } else if (tech == DcsTechType::MMD) {
        if(!m_mmIdHelper.is_mm(channelId)) return nullptr;
        Identifier dcsChannelIdDriftHv = m_mmIdHelper.multilayerID(channelId);
        const ChannelDcsMap& dcsMap = m_data_hv_mmd;
        const unsigned int array_idx = identToModuleIdx(dcsChannelIdDriftHv);
        const unsigned int channel = m_mmIdHelper.channel(dcsChannelIdDriftHv) -1;
        if (dcsMap.size() > array_idx && dcsMap.at(array_idx).channels.size() > channel && dcsMap[array_idx].channels[channel]) return dcsMap[array_idx].channels[channel].get();
    } else if(tech == DcsTechType::STG){
        if(!m_stgcIdHelper.is_stgc(channelId)) return nullptr;
        
        // the parameter issTgcQ1OuterHv is only relevant for the Q1s of the stgcs. So set it to false if we are not in Q1, just in case
        if(std::abs(m_stgcIdHelper.stationEta(channelId))!= 1) {issTgcQ1OuterHv=false;}
        Identifier dcsChannelId = m_stgcIdHelper.hvID(channelId, !issTgcQ1OuterHv /* the function take isInnerQ1 therefore invert the isOuterQ1 variable*/);
        const ChannelDcsMap& dcsMap = m_data_hv_stg;
        const unsigned int array_idx = identToModuleIdx(dcsChannelId);
        const unsigned int channel = m_stgcIdHelper.channel(dcsChannelId) -1;
        if (dcsMap.size() > array_idx && dcsMap.at(array_idx).channels.size() > channel && dcsMap[array_idx].channels[channel]) return dcsMap[array_idx].channels[channel].get();
    }
    return nullptr;
}


bool NswDcsDbData::isGood(const EventContext& ctx, const Identifier& channelId, bool issTgcQ1OuterHv) const {
    // here we will check the different DCS components that need to be good to declare a detector region as good
    // for now we only we only have the HV data
    if(!isGoodHv(channelId, issTgcQ1OuterHv)) return false;
    //isGoodEltx and isGoodTdaq are still under validation, but since they are switched off in the NswDcsAlg their data will be empty so they do not reject hits for now.
    bool permanentlyDisabled{false};
    if(!isGoodTDaq(ctx, channelId, permanentlyDisabled)) return false;
    if(!isGoodEltx(channelId)) return false;
    if(!isConnectedChannel(channelId))  return false;
    return true;

}


bool NswDcsDbData::isGoodHv(const Identifier& channelId, bool issTgcQ1OuterHv) const {
    if (m_stgcIdHelper.is_stgc(channelId)){
        const NswDcsDbData::DcsConstants* dcs = getDataForChannelHv(DcsTechType::STG, channelId, issTgcQ1OuterHv);
        /// For the moment do not kill the hit if there's no dcs data
        return !dcs || dcs->fsmState == DcsFsmState::ON;
    } else if (m_stgcIdHelper.is_mm(channelId)){
        const NswDcsDbData::DcsConstants* dcsDrift = getDataForChannelHv(DcsTechType::MMD, channelId, issTgcQ1OuterHv);
        bool driftHvIsGood = (!dcsDrift || dcsDrift->fsmState == DcsFsmState::ON);

        const NswDcsDbData::DcsConstants* dcsStrips = getDataForChannelHv(DcsTechType::MMG, channelId, issTgcQ1OuterHv);
        bool stripHvIsGood = (!dcsStrips || dcsStrips->fsmState == DcsFsmState::ON);
        
        return  driftHvIsGood && stripHvIsGood; 
    }
    return false;
}

bool NswDcsDbData::isGoodTDaq(const EventContext& ctx, const Identifier& channelId, bool &permanentlyDisabled) const {
    const ChannelTDaqMap & data = m_stgcIdHelper.is_mm(channelId) ? m_data_tdaq_mmg : m_data_tdaq_stg;
    const unsigned int array_idx = identToModuleIdx(channelId);
    if(data.size()<=array_idx || data[array_idx].empty()) return true; // for this ro element no bad elink have been recorded 
    const std::map<Identifier, std::set<TDaqConstants>>& dataInRoElement = data[array_idx];
    Identifier mapIdentifier{0};
    uint elink{0};

    if(m_stgcIdHelper.is_stgc(channelId)){
        mapIdentifier = m_stgcIdHelper.febID(channelId);
        auto mapper = Muon::nsw::MapperSTG();
        mapper.elink_info(m_stgcIdHelper.channelType(channelId), !m_stgcIdHelper.isSmall(channelId), std::abs(m_stgcIdHelper.stationEta(channelId))-1, 4*(m_stgcIdHelper.multilayer(channelId)-1) + m_stgcIdHelper.gasGap(channelId) -1, m_stgcIdHelper.channel(channelId), elink);
    } else {
        mapIdentifier = m_mmIdHelper.febID(channelId);
        auto mapper = Muon::nsw::MapperMMG();
        mapper.elink_info(std::abs(m_mmIdHelper.stationEta(channelId))-1, m_mmIdHelper.channel(channelId), elink); 
    }
    
    auto elm = dataInRoElement.find(mapIdentifier);
    if(elm == dataInRoElement.end()) return true; // channel in question was not deactivated at all
    TDaqConstants x;
    uint64_t evtTime = ctx.eventID().time_stamp()*1e9; // go from seconds to nanoseconds
    evtTime += ctx.eventID().time_stamp_ns_offset();
    x.timeSince = evtTime;
    x.timeUntil = evtTime;
    x.elink = elink;

    auto disabledLink = elm->second.find(x);
    if(disabledLink != elm->second.end()){// elink was deactivated for this time period
        permanentlyDisabled = disabledLink->permanentlyDisabled;
        return false;
    } 
    return true; // checked the channel in question, not deactivated for given run and lumi block combination, all good
}

bool NswDcsDbData::isGoodEltx(const Identifier& channelId) const{
    const ChannelTDaqMap & data = m_stgcIdHelper.is_mm(channelId) ? m_data_tdaq_mmg : m_data_tdaq_stg;
    const unsigned int array_idx = identToModuleIdx(channelId);
    if(data.size()<=array_idx || data[array_idx].empty()) return true; // for this ro element no bad elink have been recorded 
    if(data[array_idx].find(channelId) != data[array_idx].end()) return false;
    return true;
}


bool NswDcsDbData::isConnectedChannel(const Identifier& channelId) const {
    // for stgc we do not have unconnected channels
    if(m_stgcIdHelper.is_stgc(channelId)) return true;

    if(!m_mmIdHelper.is_mm(channelId)) throw std::runtime_error("the check for unconnected channels was called with an identifier that is in MM and not sTGC");

    const MuonGM::MMReadoutElement* detectorReadoutElement = m_muonGeoMgr->getMMReadoutElement(channelId);
    if(!detectorReadoutElement) {
       throw std::runtime_error("failed to retrieve MMReadoutElement");
    }
    const MuonGM::MuonChannelDesign* channelDesign = detectorReadoutElement->getDesign(channelId);
    if(!channelDesign) {
      throw std::runtime_error("failed to retrieve MuonChannelDesign");
    }

    int channel_number = m_mmIdHelper.channel(channelId);
    if(m_mmIdHelper.isStereo(channelId)){
      if(channel_number <= channelDesign->nMissedBottomStereo  || channel_number >= channelDesign->totalStrips - channelDesign->nMissedTopStereo) {
         return false;
       } 
    } else {
      if(channel_number <= channelDesign->nMissedBottomEta  || channel_number >= channelDesign->totalStrips - channelDesign->nMissedTopEta) {
        return false;
      } 
    }
    return true; 

}
