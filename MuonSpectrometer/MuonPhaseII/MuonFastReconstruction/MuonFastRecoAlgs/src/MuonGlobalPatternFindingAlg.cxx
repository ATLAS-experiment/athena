/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGlobalPatternFindingAlg.h"

#include <ActsInterop/Logger.h>

namespace MuonR4 {
using namespace FastReco;
using LayerIndex = GlobalPatternFinder::LayerIndex;

StatusCode MuonGlobalPatternFindingAlg::initialize() {
    ATH_CHECK(m_inSpacePoints.initialize());
    ATH_CHECK(m_outPatterns.initialize());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());

    GlobalPatternFinder::Config patCfg{};
    patCfg.useMdtHits = m_useMdtHits;
    patCfg.seedFromMdt = m_seedFromMdt;
    patCfg.thetaSearchWindow = m_thetaSearchWindow;
    patCfg.nResidualSigma = m_nResidualSigma;
    patCfg.lowConfidenceResSigma = m_lowConfidenceResSigma;
    patCfg.nPhiSigma = m_nPhiSigma;
    patCfg.minTriggerLayers = m_minTriggerLayers;
    patCfg.minPrecisionLayers = m_minPrecisionLayers;
    patCfg.minPhiLayers = m_minPhiLayers;
    patCfg.minStationLayers = m_minStationLayers;
    patCfg.meanNormRes2Cut = m_meanNormRes2Cut;
    patCfg.maxSeedAttempts = m_maxSeedAttempts;
    patCfg.maxMissLayersInStation = m_maxMissLayersInStation;
    patCfg.minHitDistance4Line = m_minHitDistance4Line;
    patCfg.beamSpotRadius = m_beamSpotRadius;
    patCfg.beamSpotLength = m_beamSpotLength;

    if (m_seedFromInner) {
        patCfg.layerSeedings.push_back(LayerIndex::Inner);
    }
    patCfg.visionTool = m_patVisionTool.get();
    patCfg.idHelperSvc = m_idHelperSvc.get();
    m_globPatFinder = std::make_unique<GlobalPatternFinder>(std::move(patCfg), makeActsAthenaLogger(this, name()));

    //Print Configuration
    ATH_MSG_DEBUG("Global Pattern Finder Configuration:\n"
            << " Theta search window [rad]: " << m_thetaSearchWindow << "\n"
            << " Number of residual standard deviations: " << m_nResidualSigma << "\n"
            << " Low confidence residual sigma [mm]: " << m_lowConfidenceResSigma << "\n"
            << " Max missed layer hits in station: " << m_maxMissLayersInStation << "\n"
            << " Min hit distance for line [mm]: " << m_minHitDistance4Line << "\n"
            << " Number of phi standard deviations: " << m_nPhiSigma << "\n"
            << " Min trigger layers: " << m_minTriggerLayers << "\n"
            << " Min precision layers: " << m_minPrecisionLayers << "\n"
            << " Min phi layers: " << m_minPhiLayers << "\n"
            << " Min station layers: " << m_minStationLayers << "\n"
            << " Mean norm residual^2 cut: " << m_meanNormRes2Cut << "\n"
            << " Seed from inner: " << m_seedFromInner << "\n"
            << " Use MDT hits: " << m_useMdtHits << "\n"
            << " Seed from MDT: " << m_seedFromMdt << "\n"
            << " Max seed attempts: " << m_maxSeedAttempts << "\n"
            << " Beam spot radius: " << m_beamSpotRadius << "\n"
            << " Beam spot length: " << m_beamSpotLength << "\n");

    return StatusCode::SUCCESS;
}

StatusCode MuonGlobalPatternFindingAlg::execute(const EventContext& ctx) const {
    std::vector<const SpacePointContainer*> inSpacePoints{};
    inSpacePoints.reserve(m_inSpacePoints.size());
    for (const auto& key: m_inSpacePoints) {
        auto& spc = inSpacePoints.emplace_back(nullptr);
        ATH_CHECK(SG::get(spc, key, ctx));
        ATH_MSG_DEBUG("Reading " << spc->size() << " SP buckets from collection: " << key);
    }

    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    std::vector<GlobalPattern> patterns {m_globPatFinder->findPatterns(*gctx, inSpacePoints)};

    /** Write out the global patterns. */
    SG::WriteHandle<GlobalPatternContainer> patWriteHandle{m_outPatterns, ctx};
    ATH_CHECK(patWriteHandle.record(std::make_unique<GlobalPatternContainer>()));
    for (GlobalPattern& pat : patterns) {
        patWriteHandle->push_back(std::make_unique<GlobalPattern>(std::move(pat)));
    }
    ATH_MSG_DEBUG("Written "<<patWriteHandle->size()<<" GlobalPatterns into StoreGate.");
    if (msgLvl(MSG::DEBUG)) {
        for (const auto& pat : *patWriteHandle) {
            ATH_MSG_DEBUG(*pat);
        }
    }

    return StatusCode::SUCCESS;
}

}