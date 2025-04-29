/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentViewAlg.h"
#include <AthContainers/ConstDataVector.h>
namespace MuonR4 {

    StatusCode SegmentViewAlg::initialize(){
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_writeKey.initialize());
        if (m_readKeys.empty()) {
            ATH_MSG_FATAL("Please configure at least one read key");
            return StatusCode::FAILURE;
        }

        return StatusCode::SUCCESS;
    }
    StatusCode SegmentViewAlg::execute(const EventContext& ctx) const {
        
        ConstDataVector<xAOD::MuonSegmentContainer> viewCont{SG::VIEW_ELEMENTS};
        for (const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& key : m_readKeys) {
            const xAOD::MuonSegmentContainer* segCont{nullptr};
            ATH_CHECK(SG::get(segCont, key, ctx));            
            viewCont.insert(viewCont.end(), segCont->begin(), segCont->end());
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSegmentContainer>(*viewCont.asDataVector())));
        return StatusCode::SUCCESS;
    }
}