/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StandaloneMuonTagAlg.h"

namespace MuonCombinedR4{
    StatusCode StandaloneMuonTagAlg::initialize(){
        ATH_CHECK(m_msTrackKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_summaryTool.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode StandaloneMuonTagAlg::execute(const EventContext& ctx) const{
        const ActsTrk::TrackContainer* msTracks{nullptr};
        ATH_CHECK(SG::get(msTracks, m_msTrackKey, ctx));

        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<MuonR4::MuonTagContainer>()));
        return StatusCode::SUCCESS;
    }
  
}