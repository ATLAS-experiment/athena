/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentMarkerAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "xAODMuonViews/ContainerDecorator.h"


namespace MuonR4{
    StatusCode SegmentMarkerAlg::initialize() { 
        ATH_CHECK(m_muonKey.initialize());
        ATH_CHECK(m_readMarkKey.initialize());        
        ATH_CHECK(m_segKey.initialize()); 
        m_writeMarkKey = SG:: decorKeyFromKey(m_readMarkKey.key());
        ATH_CHECK(m_writeMarkKey.initialize());
        return StatusCode::SUCCESS; 
    }
    StatusCode SegmentMarkerAlg::execute(const EventContext& ctx) const{ 
        const xAOD::MuonContainer* muons{nullptr};
        ATH_CHECK(SG::get(muons, m_muonKey, ctx));
        
        SG::ReadDecorHandle<xAOD::MuonContainer, std::uint8_t> selHandle{m_readMarkKey, ctx};
        xAOD::ContainerDecorator decorHandle{m_writeMarkKey, ctx, std::uint8_t{0}};

        for (const xAOD::Muon* muon : *muons) {
            if (!selHandle(*muon)) {
               continue;
            }
            for (unsigned int s =0; s < muon->nMuonSegments(); ++s){
                const xAOD::MuonSegment* seg = muon->muonSegment(s);
                if (seg->container() != decorHandle.container()) {
                    ATH_MSG_FATAL("The segment "<<seg<<" does not live in container "<<m_segKey.fullKey());
                    return StatusCode::FAILURE;
                }
                decorHandle(*seg)= selHandle(*muon);
            }
        }        
        return StatusCode::SUCCESS; 
    }

}