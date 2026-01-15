/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SegmentFittingAlg.h"

#include <GeoPrimitives/GeoPrimitivesHelpers.h>

#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>

#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>

#include <format>

using namespace Acts;
namespace MuonR4 {
    using namespace SegmentFit;
    using namespace MuonValR4;

 
    using PrimitiveVec = MuonValR4::IPatternVisualizationTool::PrimitiveVec;

    SegmentFittingAlg::~SegmentFittingAlg() = default;
    StatusCode SegmentFittingAlg::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_seedKey.initialize());
        ATH_CHECK(m_outSegments.initialize());  
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
        SegmentAmbiSolver::Config cfg{};
        m_ambiSolver = std::make_unique<SegmentAmbiSolver>(name(), std::move(cfg));

        SegmentLineFitter::Config fitCfg{};
        fitCfg.calibrator = m_calibTool.get();
        fitCfg.visionTool = m_visionTool.get();
        fitCfg.idHelperSvc = m_idHelperSvc.get();
        fitCfg.fitT0 = m_doT0Fit;
        fitCfg.recalibrate = m_recalibInFit;
        fitCfg.useFastFitter = m_useFastFitter;
        fitCfg.fastPreFitter = m_fastPreFitter;
        fitCfg.useHessian = m_hessianResidual;

        fitCfg.doBeamSpot = m_doBeamspotConstraint;
        fitCfg.beamSpotRadius = m_beamSpotR;
        fitCfg.beamSpotLength = m_beamSpotL;

        fitCfg.outlierRemovalCut = m_outlierRemovalCut;
        fitCfg.recoveryPull = m_recoveryPull;
        fitCfg.nPrecHitCut = m_precHitCut;
        fitCfg.maxIter = m_maxIter;

        m_fitter = std::make_unique<SegmentFit::SegmentLineFitter>(name(), std::move(fitCfg));
  
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentFittingAlg::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        const SegmentSeedContainer* segmentSeeds=nullptr; 
        ATH_CHECK(SG::get(segmentSeeds, m_seedKey, ctx));
    
        SG::WriteHandle writeSegments{m_outSegments, ctx};
        ATH_CHECK(writeSegments.record(std::make_unique<SegmentContainer>()));
        std::vector<std::unique_ptr<Segment>> allSegments{};
        for (const SegmentSeed* seed : *segmentSeeds) {
            std::vector<std::unique_ptr<Segment>> segments = fitSegmentSeed(ctx, *gctx, seed);
             if (m_visionTool.isEnabled() && segments.size() > 1) {
                auto drawFinalReco = [this, &segments, &gctx, &ctx,&seed](const std::string& nameTag) {
                    PrimitiveVec segmentLines{};
                    double yLegend{0.85};
                    segmentLines.push_back(drawLabel(std::format("# segments: {:d}",  segments.size()), 0.2, yLegend, 14));
                    yLegend-=0.04;
                    for (const std::unique_ptr<Segment>& seg : segments) {
                        const Parameters pars = localSegmentPars(*gctx, *seg);
                        const auto [pos, dir] = makeLine(pars);
                        segmentLines.emplace_back(drawLine(pars, -Gaudi::Units::m, Gaudi::Units::m, kRed));
                        std::stringstream signStream{};
                        signStream<<std::format("#chi^{{2}}/nDoF: {:.2f} ({:}), ", seg->chi2() / seg->nDoF(), seg->nDoF());
                        signStream<<std::format("y_{{0}}={:.2f}",pars[toUnderlying(ParamDefs::y0)])<<", ";
                        signStream<<std::format("#theta={:.2f}^{{#circ}}", pars[toUnderlying(ParamDefs::theta)]/ Gaudi::Units::deg )<<", ";
                        for (const Segment::MeasType& m : seg->measurements()) {
                            if (m->type() == xAOD::UncalibMeasType::MdtDriftCircleType && m->fitState() == CalibratedSpacePoint::State::Valid) {
                                signStream<<(SeedingAux::strawSign(pos, dir, *m) == -1 ? "L" : "R");
                            }
                        }
                        segmentLines.push_back(drawLabel(signStream.str(), 0.2, yLegend, 13));
                        yLegend-=0.03;
                    }

                    m_visionTool->visualizeBucket(ctx, *seed->parentBucket(), nameTag, std::move(segmentLines));
                };
                drawFinalReco("all segments");
                const unsigned int nBeforeAmbi = segments.size();
                segments = m_ambiSolver->resolveAmbiguity(*gctx, std::move(segments));
                if (nBeforeAmbi != segments.size()) {
                    drawFinalReco("post ambiguity");
                }
            } else if (m_visionTool.isEnabled() && segments.empty() &&
                      std::ranges::count_if(seed->getHitsInMax(),[this](const SpacePoint* hit){
                            return  m_visionTool->isLabeled(*hit);
                      })) {
                m_visionTool->visualizeSeed(ctx, *seed, "Failed fit");
            }
            allSegments.insert(allSegments.end(), std::make_move_iterator(segments.begin()),
                                                  std::make_move_iterator(segments.end()));
        }
        resolveAmbiguities(*gctx, allSegments);
        writeSegments->insert(writeSegments->end(),
                                  std::make_move_iterator(allSegments.begin()),
                                  std::make_move_iterator(allSegments.end()));
        ATH_MSG_VERBOSE("Found in total "<<writeSegments->size()<<" segments. ");
        return StatusCode::SUCCESS; 
    }

    std::vector<std::unique_ptr<Segment>>
         SegmentFittingAlg::fitSegmentSeed(const EventContext& ctx,
                                           const ActsTrk::GeometryContext& gctx,
                                           const SegmentSeed* patternSeed) const {

        const Amg::Transform3D& locToGlob{patternSeed->msSector()->localToGlobalTransform(gctx)};
        std::vector<std::unique_ptr<Segment>> segments{};

        MdtSegmentSeedGenerator::Config genCfg{};
        genCfg.hitPullCut = m_seedHitChi2;
        genCfg.recalibSeedCircles = m_recalibSeed;
        genCfg.calibrator = m_calibTool.get();
        genCfg.startWithPattern = m_tryPatternPars;
   
        /// At very high inclanation angles, the muon may traverse 3 hits in the same layer (E.g. BEE)
        genCfg.busyLayerLimit = 2 + 2*(patternSeed->parameters()[toUnderlying(ParamDefs::theta)] > 50 * Gaudi::Units::deg);
        /** Draw the pattern with all possible seeds */
         if (m_visionTool.isEnabled()) { 
            PrimitiveVec seedLines{};
            MdtSegmentSeedGenerator drawMe{name(), patternSeed, genCfg};
            while(auto s = drawMe.nextSeed(ctx)) {
                seedLines.push_back(drawLine(s->parameters, -Gaudi::Units::m, Gaudi::Units::m, kViolet));
            }
            seedLines.push_back(drawLabel(std::format("possible seeds: {:d}",  drawMe.numGenerated()), 0.2, 0.85, 14));
            m_visionTool->visualizeSeed(ctx, *patternSeed, "pattern", std::move(seedLines));
        }

        MdtSegmentSeedGenerator seedGen{name(), patternSeed, std::move(genCfg)};
        ATH_MSG_VERBOSE("fitSegmentHits() - Start segment seed search");
        while (auto seed = seedGen.nextSeed(ctx)) {
            auto segment = m_fitter->fitSegment(ctx, patternSeed, seed->parameters,
                                                locToGlob, std::move(seed->measurements));
            if (segment) {
                segments.push_back(std::move(segment));
            }
        }
        ATH_MSG_VERBOSE("fitSegmentHits() - In total "<<segments.size()<<" segment were constructed ");
        return segments;
    }
   
    void SegmentFittingAlg::resolveAmbiguities(const ActsTrk::GeometryContext& gctx,
                                               std::vector<std::unique_ptr<Segment>>& segmentCandidates) const {
        if (segmentCandidates.empty()) {
            return;
        }
        using SegmentVec = std::vector<std::unique_ptr<Segment>>;
        ATH_MSG_VERBOSE("Resolve ambiguities amongst "<<segmentCandidates.size()<<" segment candidates. ");
        std::unordered_map<const MuonGMR4::SpectrometerSector*, SegmentVec> candidatesPerChamber{};
        
        for (std::unique_ptr<Segment>& sortMe : segmentCandidates) {
            const MuonGMR4::SpectrometerSector* chamb = sortMe->msSector();
            candidatesPerChamber[chamb].push_back(std::move(sortMe));
        }
        segmentCandidates.clear();
        for (auto& [chamber, resolveMe] : candidatesPerChamber) {
            SegmentVec resolvedSegments = m_ambiSolver->resolveAmbiguity(gctx, std::move(resolveMe));
            segmentCandidates.insert(segmentCandidates.end(), 
                                     std::make_move_iterator(resolvedSegments.begin()),
                                     std::make_move_iterator(resolvedSegments.end()));
        }
    }
}
