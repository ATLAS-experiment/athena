/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondData/NswT0Data.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "Identifier/Identifier.h"
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

unsigned int NswT0Data::identToModuleIdx(const Identifier& chan_id) const{
    const IdentifierHash hash = m_idHelperSvc->detElementHash(chan_id);
    if (m_idHelperSvc->isMM(chan_id)) {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        return static_cast<unsigned int>(hash)*(idHelper.gasGapMax()) + (idHelper.gasGap(chan_id) -1);
    } else if (m_idHelperSvc->issTgc(chan_id)) {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        return static_cast<unsigned int>(hash)*(idHelper.gasGapMax() * 3 /*3 channel types*/) + 
               (idHelper.gasGap(chan_id) -1  + idHelper.gasGapMax() * idHelper.channelType(chan_id));
    }
    THROW_EXCEPTION("NswT0Data() - No MM or sTGC identifier");
    return -1;
}

void NswT0Data::setData(const Identifier& id, const float value){
    uint idx = identToModuleIdx(id);
    ChannelArray& data = m_idHelperSvc->isMM(id)  ? m_data_mmg : m_data_stg;
    uint channelIdx{0};
    if(m_idHelperSvc->isMM(id)){
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        if(data.at(idx).empty())  data.at(idx).resize(idHelper.channelMax(id), 0.f);
        channelIdx = idHelper.channel(id) -1;
    } else {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        if(data.at(idx).empty())  data.at(idx).resize(idHelper.channelMax(id), 0.f);
        channelIdx = idHelper.channel(id) -1;
    }
    data.at(idx).at(channelIdx) = value;
}

bool NswT0Data::getT0(const Identifier& id, float& value) const {
    uint idx = identToModuleIdx(id);
    if (m_idHelperSvc->isMM(id)) {
        if (m_data_mmg.size() <= idx) return false;
        uint channelId = m_idHelperSvc->mmIdHelper().channel(id) -1;
        value = m_data_mmg[idx].at(channelId);
        return true;
    }
    uint channelId = m_idHelperSvc->stgcIdHelper().channel(id)-1;
    if(m_data_stg.size() <= idx) return false;
    if(m_data_stg[idx].size() <= idx) return false;
    value = m_data_stg[idx].at(channelId);
    return true;
}
