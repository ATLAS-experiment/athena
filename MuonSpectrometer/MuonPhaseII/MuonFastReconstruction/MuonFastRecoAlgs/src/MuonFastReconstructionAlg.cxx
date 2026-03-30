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
    patCfg.thetaSearchWindow = m_thetaSearchWindow;
    patCfg.maxMissedLayerHits = m_maxMissedLayerHits;
    patCfg.baseRWindow = m_baseRWindow;
    patCfg.minZDiff4Line = m_minZDiff4Line;
    patCfg.minRDiff4Line = m_minRDiff4Line;
    patCfg.phiTolerance = m_phiTolerance;
    patCfg.minBendingTriggerHits = m_minBendingTriggerHits;
    patCfg.minBendingPrecisionHits = m_minBendingPrecisionHits;
    patCfg.useMdtHits = m_useMdtHits;
    patCfg.seedFromMdt = m_seedFromMdt;
    patCfg.maxSeedAttempts = m_maxSeedAttempts;
    if (m_seedFromInner) {
        patCfg.layerSeedings.push_back(LayerIndex::Inner);
    }
    patCfg.visionTool = m_visionTool.get();
    patCfg.idHelperSvc = m_idHelperSvc.get();
    m_globPatFinder = std::make_unique<GlobalPatternFinder>(name(), std::move(patCfg));

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

    /** Write out the global patterns */
    SG::WriteHandle<GlobalPatternContainer> writeHandle{m_outPatterns, ctx};
    ATH_CHECK(writeHandle.record(std::make_unique<GlobalPatternContainer>()));
    for (GlobalPattern& pat : patterns) {
        writeHandle->push_back(std::make_unique<GlobalPattern>(std::move(pat)));
    }
    ATH_MSG_DEBUG("Written "<<writeHandle->size()<<" GlobalPatterns into StoreGate.");
    return StatusCode::SUCCESS;
}


}