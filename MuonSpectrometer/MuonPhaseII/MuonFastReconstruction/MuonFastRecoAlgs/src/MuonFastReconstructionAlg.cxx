/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastReconstructionAlg.h"

namespace {
    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
    std::string printMuon(const xAOD::Muon& muon) {
        SG::Accessor<ElementLink<MuonR4::GlobalPatternContainer>> acc{"globalPatternLink"};
        
        std::ostringstream oss;
        oss<<"FastMuonSA, Eta: "<<muon.eta() << ", Phi: "<<inDeg(muon.phi())<< ", Pt: "<<muon.pt() / Gaudi::Units::GeV<<" GeV, Charge: "<<muon.charge() << std::endl
            <<"Associated GlobalPattern: "<< **acc(muon);
        return oss.str();
    }
    //defined here to avoid repeated string construction in loop
    const std::string patternLinkStr{"globalPatternLink"};
}

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
    ATH_CHECK(m_outMuons.initialize());

    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_patVisionTool.retrieve(EnableTool{!m_patVisionTool.empty()}));
    ATH_CHECK(m_segVisionTool.retrieve(EnableTool{!m_segVisionTool.empty()}));
    ATH_CHECK(m_seedingTool.retrieve());

    GlobalPatternFinder::Config patCfg{};
    patCfg.useMdtHits = m_useMdtHits;
    patCfg.seedFromMdt = m_seedFromMdt;
    patCfg.thetaSearchWindow = m_thetaSearchWindow;
    patCfg.baseResidualSigma = m_baseResidualSigma;
    patCfg.phiTolerance = m_phiTolerance;
    patCfg.minTriggerLayers = m_minTriggerLayers;
    patCfg.minPrecisionLayers = m_minPrecisionLayers;
    patCfg.minPhiLayers = m_minPhiLayers;
    patCfg.minStationLayers = m_minStationLayers;
    patCfg.meanNormRes2Cut = m_meanNormRes2Cut;
    patCfg.maxSeedAttempts = m_maxSeedAttempts;
    patCfg.maxMissLayersInStation = m_maxMissLayersInStation;
    patCfg.minHitDistance4Line = m_minHitDistance4Line;

    if (m_seedFromInner) {
        patCfg.layerSeedings.push_back(LayerIndex::Inner);
    }
    patCfg.visionTool = m_patVisionTool.get();
    patCfg.idHelperSvc = m_idHelperSvc.get();
    m_globPatFinder = std::make_unique<GlobalPatternFinder>(name(), std::move(patCfg));

    FastMuonSABuilder::Config saCfg{};
    saCfg.recalibInFit = m_recalibInFit;
    saCfg.useHessianResidual = m_hessianResidual;
    saCfg.seedHitChi2 = m_seedHitChi2;
    saCfg.recalibSeed = m_recalibSeed;
    saCfg.goodSegmentCut = m_goodSegmentCut;
    saCfg.outlierRemovalCut = m_outlierRemovalCut;
    saCfg.recoveryPull = m_recoveryPull;
    saCfg.precHitCut= m_precHitCut;
    saCfg.useFastFitter = m_useFastFitter;
    saCfg.fastPreFitter = m_fastPreFitter;
    saCfg.ignoreFailedPreFit = m_ignoreFailedPreFit;
    saCfg.maxIter = m_maxIter;

    saCfg.calibrator = m_calibTool.get();
    saCfg.visionTool = m_segVisionTool.get();
    saCfg.idHelperSvc = m_idHelperSvc.get();
    saCfg.trackSeeder = m_seedingTool.get();
    m_saBuilder = std::make_unique<FastMuonSABuilder>(name(), std::move(saCfg));

    //Print Configuration
    ATH_MSG_DEBUG("Global Pattern Finder Configuration:\n"
            << " Theta search window [rad]: " << m_thetaSearchWindow << "\n"
            << " Base residual sigma [mm]: " << m_baseResidualSigma << "\n"
            << " Max missed layer hits in station: " << m_maxMissLayersInStation << "\n"
            << " Min hit distance for line [mm]: " << m_minHitDistance4Line << "\n"
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
    
    ATH_MSG_DEBUG("FastMuonSABuilder Configuration:\n"
            << " Recalibrate in fit: " << m_recalibInFit << "\n"
            << " Use Hessian in residual: " << m_hessianResidual << "\n"
            << " Seed hit chi2: " << m_seedHitChi2 << "\n"
            << " Recalibrate seed: " << m_recalibSeed << "\n"
            << " Good segment cut (reduced chi2): " << m_goodSegmentCut << "\n"
            << " Outlier removal cut (chi2/nDoF): " << m_outlierRemovalCut << "\n"
            << " Recovery pull: " << m_recoveryPull << "\n"
            << " Precision hit cut: " << m_precHitCut << "\n"
            << " Use fast fitter: " << m_useFastFitter << "\n"
            << " Fast fitter as pre-fitter: " << m_fastPreFitter << "\n"
            << " Ignore failed pre-fits: " << m_ignoreFailedPreFit << "\n"
            << " Max iterations: " << m_maxIter);

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
    /** Write out the global patterns. */
    SG::WriteHandle<GlobalPatternContainer> patWriteHandle{m_outPatterns, ctx};
    ATH_CHECK(patWriteHandle.record(std::make_unique<GlobalPatternContainer>()));
    for (GlobalPattern& pat : patterns) {
        patWriteHandle->push_back(std::make_unique<GlobalPattern>(std::move(pat)));
    }
    ATH_MSG_DEBUG("Written "<<patWriteHandle->size()<<" GlobalPatterns into StoreGate.");
 
    /* Build Fast Reco SA muons */
    FastMuonSABuilder::MuonCont_t fillMuons;
    ATH_CHECK(fillMuons.record(m_outMuons, ctx));
    std::size_t patIdx{0};
    using Patternlink = ElementLink<GlobalPatternContainer>;
    SG::Accessor<Patternlink> acc{patternLinkStr};
    for (const GlobalPattern* pat : *patWriteHandle) {

        xAOD::Muon* newMuon {m_saBuilder->buildMuonCandidate(ctx, *gctx, *pat, fillMuons)};
        if (!newMuon) {
            ATH_MSG_DEBUG("No muon candidate could be built for pattern " << *pat);
            patIdx++;
            continue;
        }
        // Add the link to the global pattern to the muon
        acc(*newMuon) = Patternlink{*patWriteHandle, patIdx};
        ATH_MSG_DEBUG("Add new muon candidate: " << printMuon(*newMuon));
        patIdx++;
    }
    ATH_MSG_DEBUG("Written "<<fillMuons->size()<<" FastMuonSA into StoreGate.");
    return StatusCode::SUCCESS;
}


}