/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastReconstructionAlg.h"

namespace MuonR4{
using namespace FastReco;
using LayerIndex = GlobalPatternFinder::LayerIndex;

StatusCode FastReconstructionAlg::initialize() {
    if (m_outSpacePoints.size() != m_inSpacePoints.size()) {
        m_outSpacePoints.clear();
        for (const auto& key: m_inSpacePoints) {
            m_outSpacePoints.emplace_back(key.key() + m_outSpacePointSuffix);
        }
    }
    ATH_CHECK(m_inSpacePoints.initialize());
    ATH_CHECK(m_outSpacePoints.initialize());
    ATH_CHECK(m_outPatterns.initialize());  

    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));

    GlobalPatternFinder::Config patCfg{};
    patCfg.useMdtHits = m_useMdtHits;
    patCfg.seedFromMdt = m_seedFromMdt;
    patCfg.thetaSearchWindow = m_thetaSearchWindow;
    patCfg.baseRWindow = m_baseRWindow;
    patCfg.phiTolerance = m_phiTolerance;
    patCfg.minTriggerLayers = m_minTriggerLayers;
    patCfg.minPrecisionLayers = m_minPrecisionLayers;
    patCfg.minPhiLayers = m_minPhiLayers;
    patCfg.minStationLayers = m_minStationLayers;
    patCfg.meanNormRes2Cut = m_meanNormRes2Cut;
    patCfg.maxSeedAttempts = m_maxSeedAttempts;
    patCfg.maxMissLayersInStation = m_maxMissLayersInStation;
    patCfg.minLayerSeparation = m_minLayerSeparation;

    if (m_seedFromInner) {
        patCfg.layerSeedings.push_back(LayerIndex::Inner);
    }
    patCfg.visionTool = m_visionTool.get();
    patCfg.idHelperSvc = m_idHelperSvc.get();
    m_globPatFinder = std::make_unique<GlobalPatternFinder>(name(), std::move(patCfg));

    //Print Configuration
    ATH_MSG_DEBUG(" Configuration:\n"
            << " Theta search window [rad]: " << m_thetaSearchWindow << "\n"
            << " Base R window [mm]: " << m_baseRWindow << "\n"
            << " Max missed layer hits in station: " << m_maxMissLayersInStation << "\n"
            << " Min layer separation [mm]: " << m_minLayerSeparation << "\n"
            << " Phi tolerance [rad]: " << m_phiTolerance << "\n"
            << " Min trigger layers: " << m_minTriggerLayers << "\n"
            << " Min precision layers: " << m_minPrecisionLayers << "\n"
            << " Min phi layers: " << m_minPhiLayers << "\n"
            << " Min station layers: " << m_minStationLayers << "\n"
            << " Mean norm residual^2 cut: " << m_meanNormRes2Cut << "\n"
            << " Seed from inner: " << m_seedFromInner << "\n"
            << " Use MDT hits: " << m_useMdtHits << "\n"
            << " Seed from MDT: " << m_seedFromMdt << "\n"
            << " Max seed attempts: " << m_maxSeedAttempts << "\n");

    return StatusCode::SUCCESS;
}

StatusCode FastReconstructionAlg::execute(const EventContext& ctx) const {
    SpacePointContainerVec inSpacePoints{};
    inSpacePoints.reserve(m_inSpacePoints.size());
    for (const auto& key: m_inSpacePoints) {
        auto& spc = inSpacePoints.emplace_back(nullptr);
        ATH_CHECK(SG::get(spc, key, ctx));
        ATH_MSG_DEBUG("Reading " << spc->size() << " SP buckets from collection: " << key);
    }

    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    using BucketPerContainer = GlobalPatternFinder::BucketPerContainer;
    BucketPerContainer outBuckets{};
    for (const SpacePointContainer* inSpc : inSpacePoints) {
        outBuckets.emplace(inSpc, BucketPerContainer::mapped_type{});
    }
    PatternVec patterns {m_globPatFinder->findPatterns(*gctx, inSpacePoints, outBuckets)};

    /** Write out the filtered spacepoint containers, i.e. those having a pattern */
    for (size_t idx = 0; idx < inSpacePoints.size(); ++idx) {
        const SpacePointContainer* inSpc = inSpacePoints.at(idx);
        const auto& outBucketVec = outBuckets.at(inSpc);

        SG::WriteHandle<SpacePointContainer> writeHandle{m_outSpacePoints.at(idx), ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<SpacePointContainer>()));

        for (const SpacePointBucket* bucket : outBucketVec) {
            writeHandle->push_back(std::make_unique<SpacePointBucket>(*bucket));
        }

        ATH_MSG_DEBUG("Written " << writeHandle->size()
                        << " SP Buckets into StoreGate with key "
                        << m_outSpacePoints.at(idx));
    }
    /** Write out the global patterns. Make this optional in the future */
    SG::WriteHandle<GlobalPatternContainer> writeHandle{m_outPatterns, ctx};
    ATH_CHECK(writeHandle.record(std::make_unique<GlobalPatternContainer>()));
    for (const GlobalPattern& pat : patterns) {
        writeHandle->push_back(std::make_unique<GlobalPattern>(pat));
    }
    ATH_MSG_DEBUG("Written "<<writeHandle->size()<<" GlobalPatterns into StoreGate.");

    /* Consume the patterns to build muon candidates. WORK IN PROGRESS */
    return StatusCode::SUCCESS;
}


}