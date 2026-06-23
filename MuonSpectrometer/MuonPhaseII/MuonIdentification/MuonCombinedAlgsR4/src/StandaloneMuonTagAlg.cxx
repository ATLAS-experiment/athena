/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StandaloneMuonTagAlg.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "xAODMuonViews/FillContainer.h"
#include "ActsEvent/Decoration.h"

namespace{
    using TagFillCont_t = xAOD::FillContainer<MuonR4::MuonTagContainer, void*>;
}

namespace MuonCombinedR4{
    StatusCode StandaloneMuonTagAlg::initialize(){
        ATH_CHECK(m_msTrackKey.initialize());
        ATH_CHECK(m_tagKey.initialize());
        ATH_CHECK(m_summaryTool.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode StandaloneMuonTagAlg::execute(const EventContext& ctx) const{
        const xAOD::TrackParticleContainer* msTracks{nullptr};
        ATH_CHECK(SG::get(msTracks, m_msTrackKey, ctx));

        TagFillCont_t outputTags{};
        
        for (const xAOD::TrackParticle* trackPart : *msTracks) {
            auto msTag = outputTags->push_back(std::make_unique<MuonR4::MuonTag>());
            msTag->setAuthor(xAOD::Muon::Author::MuidSA);
            msTag->setMsTrack(trackPart);
            auto track = ActsTrk::getActsTrack(*trackPart);
            if (!track) {
                ATH_MSG_ERROR("No Acts track object is associated");
                return StatusCode::FAILURE;
            }
            msTag->setSummary(m_summaryTool->makeSummary(ctx, *track));
            msTag->setSegments((*track).component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks"));
        }
        ATH_CHECK(outputTags.record(m_tagKey, ctx));
        return StatusCode::SUCCESS;
    }
  
}