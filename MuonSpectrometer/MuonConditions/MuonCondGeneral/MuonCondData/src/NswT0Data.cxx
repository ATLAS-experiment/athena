/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondData/NswT0Data.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "GeoModelKernel/throwExcept.h"


NswT0Data::NswT0Data(const Muon::IMuonIdHelperSvc* idHelperSvc): 
    m_idHelperSvc{idHelperSvc} {
    if (m_idHelperSvc->hasMM()) {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};    
        m_data_mmg.resize((idHelper.detectorElement_hash_max()+1)*idHelper.gasGapMax());
    }
    if (m_idHelperSvc->hasSTGC()) {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        m_data_stg.resize((idHelper.detectorElement_hash_max() +1)*(idHelper.gasGapMax()*3 /*3 channel types*/));

    }
}

std::size_t NswT0Data::identToModuleIdx(const Identifier& chan_id) const{
    const IdentifierHash hash = m_idHelperSvc->detElementHash(chan_id);
    if (m_idHelperSvc->isMM(chan_id)) {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        return static_cast<std::size_t>(hash)*(idHelper.gasGapMax()) + (idHelper.gasGap(chan_id) -1);
    } else if (m_idHelperSvc->issTgc(chan_id)) {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        return static_cast<std::size_t>(hash)*(idHelper.gasGapMax() * 3 /*3 channel types*/) + 
               (idHelper.gasGap(chan_id) -1  + idHelper.gasGapMax() * idHelper.channelType(chan_id));
    }
    THROW_EXCEPTION("NswT0Data() - No MM or sTGC identifier");
    return -1;
}

void NswT0Data::setData(const Identifier& id, const float value){
    std::size_t idx = identToModuleIdx(id);
    ChannelArray& data = m_idHelperSvc->isMM(id)  ? m_data_mmg : m_data_stg;
    std::size_t channelIdx{0};
    if(m_idHelperSvc->isMM(id)){
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        if(data.at(idx).empty()) {
            data.at(idx).resize(idHelper.channelMax(id));
        }
        channelIdx = idHelper.channel(id) -1;
    } else {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        if(data.at(idx).empty())  {
            data.at(idx).resize(idHelper.channelMax(id));
        }
        channelIdx = idHelper.channel(id) -1;
    }
    data.at(idx).at(channelIdx) = value;
}

std::optional<float> NswT0Data::getT0(const Identifier& id) const {
    std::size_t idx = identToModuleIdx(id);
    
    if (m_idHelperSvc->isMM(id)) {
        std::size_t channelId = m_idHelperSvc->mmIdHelper().channel(id) -1;        
        if (m_data_mmg.size() <= idx || m_data_mmg[idx].size() <= channelId){
            return std::nullopt;
        } 
        return m_data_mmg[idx].at(channelId);
    }
    std::size_t channelId = m_idHelperSvc->stgcIdHelper().channel(id) - 1;
    if(m_data_stg.size() <= idx || m_data_stg[idx].size() <= channelId) {
        return std::nullopt;
    }
    return m_data_stg[idx].at(channelId);
}
