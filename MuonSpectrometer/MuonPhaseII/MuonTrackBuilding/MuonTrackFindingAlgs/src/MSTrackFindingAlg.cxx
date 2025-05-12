/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSTrackFindingAlg.h"

#include "AthContainers/ConstDataVector.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"


namespace MuonR4{
    StatusCode MSTrackFindingAlg::initialize() {
        ATH_CHECK(m_segmentKeys.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_magFieldKey.initialize());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_msTrkSeedKey.initialize());
        ATH_CHECK(m_visualizationTool.retrieve(EnableTool{!m_visualizationTool.empty()}));
        return StatusCode::SUCCESS;
    }

    MSTrackFindingAlg::~MSTrackFindingAlg() = default;


    StatusCode MSTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
        
        ConstDataVector<xAOD::MuonSegmentContainer> allEventSegs{SG::VIEW_ELEMENTS};
        for (const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& key : m_segmentKeys) {
            const xAOD::MuonSegmentContainer* partSegments{nullptr};
            ATH_CHECK(SG::get(partSegments, key, ctx));
            allEventSegs.insert(allEventSegs.end(), partSegments->begin(), partSegments->end());
        }
        auto seedContainer = findTrackSeeds(ctx, *allEventSegs.asDataVector());

        SG::WriteHandle writeHandle{m_msTrkSeedKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(seedContainer)));
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<MsTrackSeedContainer>  
        MSTrackFindingAlg::findTrackSeeds(const EventContext& ctx,
                                          const xAOD::MuonSegmentContainer& segments) const {

        MsTrackSeeder::Config seederCfg{};
        seederCfg.seedHalfLength = m_seedHalfLength;
        seederCfg.selector = m_segSelector.get();

        MsTrackSeeder seeder{name(), std::move(seederCfg)};

        auto seedContainer = seeder.findTrackSeeds(ctx, segments);

        if (!m_visualizationTool.empty()) {
            m_visualizationTool->displaySeeds(ctx, seeder, segments, *seedContainer, "all seeds");
        }
        return seedContainer;
    }
}