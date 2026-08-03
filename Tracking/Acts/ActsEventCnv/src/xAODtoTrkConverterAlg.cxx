/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  
#include "xAODtoTrkConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandle.h"

#include "ActsEvent/Decoration.h"

namespace ActsTrk {
    StatusCode xAODtoTrkConverterAlg::initialize() {
        ATH_CHECK(m_trackKey.initialize());
        ATH_CHECK(m_linkKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_ATLASConverterTool.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode xAODtoTrkConverterAlg::execute(const EventContext& ctx) const {
        const xAOD::TrackParticleContainer* inputTracks{nullptr};
        ATH_CHECK(SG::get(inputTracks, m_trackKey, ctx));
        auto convertedTracks = std::make_unique<TrackCollection>();

        using Link_t = ElementLink<TrackCollection>;
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, Link_t> dec_link{m_linkKey, ctx};
        for (const xAOD::TrackParticle* inTrack: *inputTracks) {
            // Create the dummy link
            Link_t& link = dec_link(*inTrack);
            // Fetch the Acts track from the decorator
            auto actsTrk = getActsTrack(*inTrack);
            if (!actsTrk) {
                ATH_MSG_ERROR("Track particle without ACTS track decoration");
                return StatusCode::FAILURE;
            }
            // convert into trk track
            auto track = m_ATLASConverterTool->convertTrack(ctx, *actsTrk);
            if (!track) {
                ATH_MSG_ERROR("Track conversion failed.");
                return StatusCode::FAILURE;
            }
            link = Link_t{*convertedTracks, convertedTracks->size()};
            convertedTracks->push_back(std::move(track));
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(convertedTracks)));
        return StatusCode::SUCCESS;
    }
 
}