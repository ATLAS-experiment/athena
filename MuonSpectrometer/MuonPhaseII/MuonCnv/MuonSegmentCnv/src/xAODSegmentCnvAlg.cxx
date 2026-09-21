/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODSegmentCnvAlg.h"

using namespace Acts::UnitLiterals;

namespace MuonR4{

    StatusCode xAODSegmentCnvAlg::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_readKeys.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_prdLinkKey.initialize());
        ATH_CHECK(m_localSegParKey.initialize());
        ATH_CHECK(m_localSegCovKey.initialize());
        ATH_CHECK(m_parentSegKey.initialize());
        ATH_CHECK(m_combMeasKey.initialize());
        ATH_CHECK(m_prdStateKey.initialize());
        ATH_CHECK(m_auxMeasProv.initialize(m_writeKey.key(), m_convertBeamSpot));
        ATH_CHECK(m_segmentCnvTool.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode xAODSegmentCnvAlg::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        IxAODSegmentCnvTool::DataShip ship{};
        ATH_CHECK(ship.segmentContainer.record(m_writeKey, ctx));
        ATH_CHECK(ship.setupLocalParameters(m_localSegParKey, m_localSegCovKey, ctx));
        ATH_CHECK(ship.setupMeasurementLink(m_combMeasKey, m_prdLinkKey, m_prdStateKey, ctx));
        if (m_convertBeamSpot) {
            ATH_CHECK(ship.setupBeamSpotMeasurement(m_auxMeasProv, ctx, *gctx));
        }      
        
        /** @brief Abrivation of the link to the reco segment container  */
        using SegLink_t = ElementLink<MuonR4::SegmentContainer>;
        SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegLink_t> dec_parentLink{m_parentSegKey, ctx};
 
        for (const SG::ReadHandleKey<SegmentContainer>& key : m_readKeys) {
            const SegmentContainer* segmentContainer{nullptr};
            ATH_CHECK(SG::get(segmentContainer, key, ctx));
         
            /// Counter for the reco segment link
            unsigned recoSegIdx{0};
            ship.segmentContainer->reserve(ship.segmentContainer->size() + segmentContainer->size());

            for (const Segment* inSegment : *segmentContainer) {
                xAOD::MuonSegment* convertedSeg = m_segmentCnvTool->convertSegment(ctx, *inSegment, ship);
                if (!convertedSeg) {
                    ATH_MSG_ERROR("Failed to convert segment");
                    return StatusCode::FAILURE;
                }

                dec_parentLink(*convertedSeg) = SegLink_t{key.key(), recoSegIdx, ctx};
                ++recoSegIdx;
            }
        }  
        return StatusCode::SUCCESS;
    }
}
