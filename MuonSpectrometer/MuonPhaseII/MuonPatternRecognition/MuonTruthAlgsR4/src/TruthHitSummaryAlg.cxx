/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthHitSummaryAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "MuonTrackEvent/HitSummary.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "xAODMuon/versions/MuonTrackSummaryAccessors_v1.h"
namespace MuonR4{
    StatusCode TruthHitSummaryAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_segLinkKey.initialize());
        ATH_CHECK(m_summaryTool.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode TruthHitSummaryAlg::execute(const EventContext& ctx) const {
        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons, m_readKey, ctx));
        for (const xAOD::TruthParticle* truth : *truthMuons){
            m_summaryTool->copySummary(m_summaryTool->makeSummary(ctx,
                                            getTruthSegments(*truth)), *truth);
        }
        return StatusCode::SUCCESS;
    }

}
