/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonAlignmentData/sTGCAsBuiltData2.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

sTGCAsBuiltData2::sTGCAsBuiltData2(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"sTGCAsBuiltData2"},
    m_idHelperSvc{idHelperSvc} {}



Amg::Vector2D sTGCAsBuiltData2::correctPosition(const Identifier& channelId, const Amg::Vector2D& pos) const {
    ParMap::const_iterator par_itr = m_asBuiltData.find(m_idHelperSvc->gasGapId(channelId));
    if(par_itr == m_asBuiltData.end()){
        ATH_MSG_WARNING("Missing as built parameters for gas gap " << m_idHelperSvc->toString(channelId));
        return pos;
    }
    Amg::Vector2D correctedPos = pos;
    Parameters pars = par_itr->second;
    constexpr double convScale = 1.e-3; // parameters are stored in um and mrad therefore dividing by 1000
    correctedPos.x() = pos.x() + (pars.offset * convScale  + pars.rotation *convScale * pos.y() + pars.scale*convScale*pos.x() + pars.nonPara * convScale * convScale * pos.x()*pos.y()); 
    return correctedPos;
}


StatusCode sTGCAsBuiltData2::setParameters(const Identifier& channelId, const Parameters& pars){
    const Identifier gasGapId = m_idHelperSvc->gasGapId(channelId);
    auto insert_itr = m_asBuiltData.insert(std::make_pair(gasGapId, pars));
    if (!insert_itr.second) {
        ATH_MSG_ERROR("As built parameters for gasGap "<<m_idHelperSvc->toStringGasGap(gasGapId)
            <<" has already been stored ");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
    