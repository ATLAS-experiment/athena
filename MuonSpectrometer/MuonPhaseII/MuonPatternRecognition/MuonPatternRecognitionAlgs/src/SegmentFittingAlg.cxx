/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentFittingAlg.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

#include "MuonPatternHelpers/MdtSegmentSeedGenerator.h"
#include "MuonSpacePoint/SpacePointPerLayerSplitter.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "ActsCalibBase/CalibrationContext.h"
#include "ActsInterop/Logger.h"

#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "Acts/Definitions/Units.hpp"

#include <format>

using namespace Acts;
using namespace Acts::UnitLiterals;

namespace MuonR4 {
    using namespace SegmentFit;
    using namespace MuonValR4;

    /** brief checks if the trajectory passes through the given chamber in local y,z plane if phiExtension is available checks also the x,z plane
     *  @param loc the chamber location
     *  @param seed the segment seed
     *  @param etaEdgeTolerance the tolerance in the eta projection
     *  @param phiEdgeTolerance the tolerance in the phi projection
     *  @return true if the trajectory passes through the chamber, false otherwise
     */
    inline bool trajectoryPassesThroughChamber( const SpacePointBucket::chamberLocation& loc,
                                            const SegmentSeed& seed,
                                            double etaEdgeTolerance = 25. * Gaudi::Units::mm,
                                            double phiEdgeTolerance = 0.){       

        // eta projection
        const double z = loc.location().z();
        const double yCross = seed.interceptY() + (z * seed.tanBeta());
        const bool passEta = loc.minY() + etaEdgeTolerance < yCross && yCross < loc.maxY() - etaEdgeTolerance;

        if (!passEta) { 
            return false; 
        }

        // No reliable second coordinate available.
        if (!seed.hasPhiExtension()) {
            return true;
        }

        // phi projection
        const double xCross = seed.interceptX() + (z * seed.tanAlpha());

        // Chamber half-width in x depends on y.
        const double halfWidthX = loc.width(yCross);
        
        return ((loc.location().x() - halfWidthX) + phiEdgeTolerance) < xCross && xCross < ((loc.location().x() + halfWidthX) - phiEdgeTolerance);

    }
 
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
        fitCfg.ignoreFailedPreFit = m_ignoreFailedPreFit;
        fitCfg.useHessian = m_hessianResidual;

        fitCfg.doBeamSpot = m_doBeamspotConstraint;
        fitCfg.beamSpotRadius = m_beamSpotR;
        fitCfg.beamSpotLength = m_beamSpotL;

        if (m_doBeamspotConstraint) {
            for (const auto* prop :{&m_seedMaxBsR, &m_seedMaxBsL}) {
                if (prop->value().size() != s_stIdxMax || 
                    std::ranges::min(prop->value()) < 0.) {
                    ATH_MSG_ERROR("Invalid configuratnon of "<<(prop));
                    return StatusCode::FAILURE;
                }
                ATH_MSG_DEBUG("Configured "<<(*prop));
            }
        }
        fitCfg.outlierRemovalCut = m_outlierRemovalCut;
        fitCfg.recoveryPull = m_recoveryPull;
        fitCfg.nPrecHitCut = m_precHitCut;
        fitCfg.maxIter = m_maxIter;
        ATH_MSG_DEBUG("Fitter configuration: \n -- "<<m_doT0Fit<<"\n -- "<<m_recalibInFit<<"\n -- "<<m_useFastFitter
                <<"\n -- "<<m_fastPreFitter<<"\n -- "<<m_ignoreFailedPreFit<<"\n -- "<<m_hessianResidual
                <<"\n -- "<<m_doBeamspotConstraint<<"\n -- "<<m_beamSpotR<<"\n -- "<<m_beamSpotL<<"\n -- "<<m_outlierRemovalCut
                <<"\n -- "<<m_recoveryPull<<"\n -- "<<m_precHitCut<<"\n -- "<<m_maxIter);

        m_fitter = std::make_unique<SegmentFit::SegmentLineFitter>(name(), std::move(fitCfg));

        MdtSegmentSeedGenerator::Config genCfg{};
        genCfg.hitPullCut = m_seedHitChi2;
        genCfg.busyLayerLimit = m_busyLayerLimit;
        genCfg.startWithPattern = m_tryPatternPars;
        ATH_MSG_DEBUG("Seeder configuration: \n - "<<m_tryPatternPars
                    <<"\n - "<<m_seedHitChi2
                    <<"\n - "<<m_busyLayerLimit);  
        m_seeder = std::make_unique<SegmentFit::MdtSegmentSeedGenerator>(genCfg, makeActsAthenaLogger(this, name()));

        //Cache station indices for quick access
        const auto& mdtHelper = m_idHelperSvc->mdtIdHelper();
        m_bilStation = mdtHelper.stationNameIndex("BIL");
        m_bimStation = mdtHelper.stationNameIndex("BIM");
        m_birStation = mdtHelper.stationNameIndex("BIR");
     
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentFittingAlg::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        const SegmentSeedContainer* segmentSeeds=nullptr; 
        ATH_CHECK(SG::get(segmentSeeds, m_seedKey, ctx));
    
        SG::WriteHandle writeSegments{m_outSegments, ctx};
        ATH_CHECK(writeSegments.record(std::make_unique<SegmentContainer>()));
        SegmentVec_t allSegments{};
        ATH_MSG_VERBOSE("execute() - Start processing " << segmentSeeds->size() << " pattern seeds.");
        for (const SegmentSeed* seed : *segmentSeeds) {
            SegmentVec_t segments = fitSegmentSeed(ctx, *gctx, seed);
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
                        const auto& cov = seg->covariance();
                        signStream<<std::format("#chi^{{2}}/nDoF: {:.2f} ({:}), ", seg->chi2() / seg->nDoF(), seg->nDoF());
                        signStream<<std::format("y_{{0}}={:.2f}#pm{:.2f}",
                                                pars[toUnderlying(ParamDefs::y0)],
                                                Amg::error(cov, toUnderlying(ParamDefs::y0)))<<", ";
                        signStream<<std::format("#theta={:.2f}#pm{:.2f}^{{#circ}}", 
                                                pars[toUnderlying(ParamDefs::theta)]/ 1._degree,
                                                Amg::error(cov, toUnderlying(ParamDefs::theta)) / 1._degree )<<", ";
                        for (const Segment::MeasType& m : seg->measurements()) {
                            if (m->type() == xAOD::UncalibMeasType::MdtDriftCircleType && 
                                m->fitState() == CalibratedSpacePoint::State::Valid) {
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
                      std::ranges::any_of(seed->getHitsInMax(),[this](const SpacePoint* hit){
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

    SegmentFittingAlg::SegmentVec_t
         SegmentFittingAlg::fitSegmentSeed(const EventContext& ctx,
                                           const ActsTrk::GeometryContext& gctx,
                                           const SegmentSeed* patternSeed) const {

        const Acts::Transform3& locToGlob{patternSeed->msSector()->localToGlobalTransform(gctx)};
        std::vector<std::unique_ptr<Segment>> segments{};

        const Acts::CalibrationContext cctx = ActsTrk::getCalibrationContext(ctx);

        using namespace Muon::MuonStationIndex;
        using State_t = MdtSegmentSeedGenerator::State_t;


        auto seedSelector = [&](const Amg::Vector3D& tangentSeedPos,
                               const Amg::Vector3D& tangentSeedDir) {
                                if (!m_doBeamspotConstraint) {
                                    return true;
                                }
                                const Amg::Vector3D globPos{locToGlob*tangentSeedPos};
                                const Amg::Vector3D globDir{locToGlob.linear()*tangentSeedDir};

                                /** This patch restores the efficiency for muons with pT< 10 GeV in
                                 *  the middle and outer endcap stations, for more details on the derivation of the exact window cuts see MR !91056 */
                                const StIndex stIdx = toStationIndex(patternSeed->msSector()->chamberIndex());
                                switch (stIdx) {
                                    using enum StIndex;
                                    case EM: {
                                        if (std::abs(globPos.eta()) > 2.35 && 
                                            std::abs(globPos.theta() - globDir.theta()) < 5.8_degree) {
                                            return true;
                                        }
                                        break;
                                    }
                                    case EO: {
                                         if (std::abs(globPos.eta()) > 2.35 && 
                                            std::abs(globPos.theta() - globDir.theta()) < 3._degree) {
                                            return true;
                                        }
                                        break;
                                    }
                                    default:
                                        break;
                                }
                                using namespace Acts::detail::LineHelper;
                                const Acts::Intersection3D bsExtp = lineIntersect<3>(Amg::Vector3D::Zero(),
                                                                                     Amg::Vector3D::UnitZ(),
                                                                                     globPos, globDir);
                                const Amg::Vector3D closePoint = bsExtp.position();
                                const auto rawStIdx = Acts::toUnderlying(stIdx);
                                //coverity[NEGATIVE_RETURNS]
                                if (closePoint.perp() > m_seedMaxBsR.value().at(rawStIdx) * m_beamSpotR ||
                                    std::abs(closePoint.z()) >  m_seedMaxBsL.value().at(rawStIdx) * m_beamSpotL){
                                    ATH_MSG_DEBUG("fitSegmentSeed() - Reject parameters "<<Amg::toString(tangentSeedPos)
                                                  <<" + "<<Amg::toString(tangentSeedDir)
                                                  <<" as extrapolation to beamspot is too far "<<Amg::toString(closePoint)
                                                  <<", r: "<<closePoint.perp());
                                    return false;
                                }
                                return true;
        }; 

        // Make sure patternSeed->parameters() are in ACTS units!
        State_t seedState{patternSeed->parameters(), patternSeed, m_calibTool.get(), m_recalibSeed, seedSelector};
        const auto* seeder =  m_seeder.get();


        auto visualizeSegmentSeeds = [&](State_t& state, const std::string_view label) {
            if (m_visionTool.isEnabled()) { 
                PrimitiveVec seedLines{};
                State_t drawMe{state};
                while(auto s = seeder->nextSeed(cctx, drawMe)) {
                    seedLines.push_back(drawLine(s->parameters, -Gaudi::Units::m, Gaudi::Units::m, kViolet));
                }
                seedLines.push_back(drawLabel(std::format("{} possible seeds: {:d}", label, drawMe.nGenSeeds()), 0.2, 0.85, 14));
                m_visionTool->visualizeSeed(ctx, *patternSeed, std::format("pattern_{:}{:}{:}",
                    patternSeed->msSector()->chamberIndex(),
                    patternSeed->msSector()->side() ? 'A' : 'C' ,
                    patternSeed->msSector()->sector()), std::move(seedLines));
            }
        };

        auto runSeeder = [&](State_t&& state) {
            while (auto seed = seeder->nextSeed(cctx, state)) {
            
                seed->parameters[toUnderlying(ParamDefs::t0)] = ActsTrk::timeToAthena( seed->parameters[toUnderlying(ParamDefs::t0)]);
                    
                auto segment = m_fitter->fitSegment( ctx, patternSeed, seed->parameters, locToGlob, std::move(seed->hits));
                    
                if (segment) {
                    segments.push_back(std::move(segment));
                }
            }
        };

        visualizeSegmentSeeds(seedState, "Nominal Seeds");
        ATH_MSG_VERBOSE("fitSegmentHits() - Start segment seed search");
        runSeeder(std::move(seedState));

        //----------------------------------------------------------------------------------------
        /** MDT-multilayer fallback. Only try this when standard segment reconstruction failed.  */
        /* Trying to recover certain cases where we either have only one multilayer or where the chambers are not overlapping and there are no hits in the other (e.g. BIM & BIR)*/
        //----------------------------------------------------------------------------------------
        if( m_allowSegmentSeedingFallback && segments.empty() && (patternSeed->msSector()->chamberIndex() == ChIndex::BIL) ) {
            // Implement fallback logic here
            ATH_MSG_VERBOSE( "fitSegmentSeed() - Standard MDT seeding failed. " "Trying ML fallback.");

            //For recovery seeding we will have smaller hit collection
            std::optional<SpacePointPerLayerSplitter::HitVec> recoveryHits{};

            // First try the very restrictive BIL single-ML recovery.
            recoveryHits = makeBilRecoveryHits(*patternSeed);

            if (!recoveryHits) {
                    ATH_MSG_VERBOSE(__func__<< ":" <<__LINE__<< "- BIL recovery did not succeed. Try BIM/BIR.");
                    recoveryHits = makeBimBirRecoveryHits(*patternSeed);
            }

             // Both recovery modes ultimately use exactly the same nominal MDT seeder. Only their input hits differ.
             if (recoveryHits) {
                ATH_MSG_VERBOSE( "fitSegmentSeed() - ML seeding fallback has " << recoveryHits->size() << " hits.");
                State_t fallbackState{ patternSeed->parameters(), patternSeed, m_calibTool.get(), m_recalibSeed, *recoveryHits, seedSelector};
                visualizeSegmentSeeds(fallbackState , "Fallback Seeds");
                runSeeder(std::move(fallbackState));
             }

        }

        ATH_MSG_VERBOSE("fitSegmentHits() - In total "<<segments.size()<<" segment were constructed ");
        return segments;

    }
   
    void SegmentFittingAlg::resolveAmbiguities(const ActsTrk::GeometryContext& gctx,
                                               SegmentVec_t& segmentCandidates) const {
        if (segmentCandidates.empty()) {
            return;
        }
        ATH_MSG_VERBOSE("resolveAmbiguities() - Resolve ambiguities amongst "
                        <<segmentCandidates.size()<<" segment candidates.");
        std::map<const MuonGMR4::SpectrometerSector*, SegmentVec_t,
                 MuonGMR4::MuonDetectorManager::MSEnvelopeSorter> candidatesPerChamber{};
        
        for (std::unique_ptr<Segment>& sortMe : segmentCandidates) {
            const MuonGMR4::SpectrometerSector* chamb = sortMe->msSector();
            candidatesPerChamber[chamb].push_back(std::move(sortMe));
        }
        segmentCandidates.clear();
        for (auto& [chamber, resolveMe] : candidatesPerChamber) {
            const std::size_t nBefore = resolveMe.size();
            SegmentVec_t resolvedSegments = m_ambiSolver->resolveAmbiguity(gctx, std::move(resolveMe));
            ATH_MSG_DEBUG("resolveAmbiguities()  - "<<resolvedSegments.size()<<"/"<<nBefore
                <<" segments survived ambiguity solving in "<<chamber->identString()<<".");
            segmentCandidates.insert(segmentCandidates.end(), 
                                     std::make_move_iterator(resolvedSegments.begin()),
                                     std::make_move_iterator(resolvedSegments.end()));
        }
        ATH_MSG_VERBOSE("Ambiguity solving done "<<segmentCandidates.size()<<" survived.");
    }


std::optional<SpacePointPerLayerSplitter::HitVec>SegmentFittingAlg::makeBilRecoveryHits(const SegmentSeed& seed) const {

    const auto& mdtHelper = m_idHelperSvc->mdtIdHelper();

    const MuonGMR4::MuonReadoutElement* crossedRE{nullptr};

    // Find BIL readout elements crossed by the Hough trajectory.
    for (const auto& chamber : seed.parentBucket()->chamberLocations()) {

        const auto* readoutEle = chamber.readoutEle();

        if (readoutEle->detectorType() != ActsTrk::DetectorType::Mdt) {
            continue;
        }

        const Identifier id = readoutEle->identify();
        //For now do this only for BIL stations (technically could be even more specific, BIL eta 3 or 4), choose int comparison instead?
        if (mdtHelper.stationName(id) != m_bilStation) {
            continue;
        }

        if (!trajectoryPassesThroughChamber(chamber, seed, m_singleMlEdgeTolerance, m_singleMlPhiEdgeTolerance)) {
            continue;
        }

        crossedRE = readoutEle;

        ATH_MSG_VERBOSE( "BIL recovery: Hough trajectory crosses " << m_idHelperSvc->toStringDetEl(id));
    }

    // BIL recovery is strictly a single crossed-ML recovery.
    if (!crossedRE) {
        ATH_MSG_VERBOSE( "BIL recovery rejected: trajectory crosses no BIL readout elements.");
        return std::nullopt;
    }

    SpacePointPerLayerSplitter::HitVec recoveryHits{};
    recoveryHits.reserve(seed.getHitsInMax().size());

    std::size_t nMdtHits{0};

    for (const SpacePoint* hit : seed.getHitsInMax()) {

        // Preserve non-MDT hits.
        if (hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
            recoveryHits.push_back(hit);
            continue;
        }

        // Only MDT hits from the single geometrically crossed BIL RE.
        if (hit->primaryMeasurement()->readoutElement() != crossedRE) {
            continue;
        }

        recoveryHits.push_back(hit);
        ++nMdtHits;
    }

    ATH_MSG_VERBOSE( "BIL recovery: keeping " << nMdtHits << " MDT hits from " << m_idHelperSvc->toStringDetEl( crossedRE->identify()));
    return recoveryHits;
}


std::optional<SpacePointPerLayerSplitter::HitVec> SegmentFittingAlg::makeBimBirRecoveryHits( const SegmentSeed& seed) const {

    // For BIM/BIR we rely on the phi extension to distinguish
    // readout elements which overlap in the eta projection.
    if (!seed.hasPhiExtension()) {
        ATH_MSG_VERBOSE(
            "BIM/BIR recovery rejected: seed has no phi extension.");
        return std::nullopt;
    }

    const auto& mdtHelper = m_idHelperSvc->mdtIdHelper();

    /*
     * Use the geometrical crossing only to identify the recovery
     * station family (BIM/BIR) and stationPhi.
     *
     * Do NOT use the crossed REs themselves for hit filtering:
     * the Hough trajectory can be biased in eta by high occupancy.
     */
    std::optional<int> recoveryStation{};

    for (const auto& chamber : seed.parentBucket()->chamberLocations()) {

        const auto* readoutEle = chamber.readoutEle();

        if (readoutEle->detectorType() != ActsTrk::DetectorType::Mdt) {
            continue;
        }

        const Identifier id = readoutEle->identify();
        const int station = mdtHelper.stationName(id);

        //Don't care about other stations for now
        if (station != m_bimStation && station != m_birStation) {
            continue;
        }

        //Don't need any tolerance here
        if (!trajectoryPassesThroughChamber(chamber, seed, 0., 0.)) {
            continue;
        }

        ATH_MSG_VERBOSE( "BIM/BIR recovery: Hough trajectory crosses " << m_idHelperSvc->toStringDetEl(id));

        // First crossed BIM/BIR RE defines the recovery region.
        if (!recoveryStation) {
            recoveryStation = station;
            continue;
        }

        /*
         * All geometrically crossed BIM/BIR REs must be compatible
         * with the same station family
         * Different eta and multilayer are explicitly allowed.
         */
        if (*recoveryStation != station){ 
            ATH_MSG_VERBOSE( "BIM/BIR recovery rejected: crossed readout " "elements belong to different station/phi regions.");
            return std::nullopt;
        }
    }

    // No geometrically compatible BIM/BIR region.
    if (!recoveryStation) {
        return std::nullopt;
    }

    /*
     * Now construct the recovery collection.
     * Selection:
     *   - same BIM/BIR station family
     *   - ANY stationEta
     *   - ANY multilayer
     * Only hits already belonging to this Hough seed are considered.
     */
    SpacePointPerLayerSplitter::HitVec recoveryHits{};
    recoveryHits.reserve(seed.getHitsInMax().size());

    std::size_t nMdtHits{0};

    for (const SpacePoint* hit : seed.getHitsInMax()) {

        //Preserve the non-MDT content of the original seed.
        if (hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
            recoveryHits.push_back(hit);
            continue;
        }

        const Identifier hitId = hit->identify();

        if (mdtHelper.stationName(hitId) != *recoveryStation) {
            continue;
        }

        recoveryHits.push_back(hit);
        ++nMdtHits;

        ATH_MSG_VERBOSE( "BIM/BIR recovery keeps MDT hit " << m_idHelperSvc->toString(hitId));
    }

    ATH_MSG_VERBOSE( "BIM/BIR recovery selected " << nMdtHits << " MDT hits from station " << mdtHelper.stationNameString(*recoveryStation));

    if (nMdtHits == 0) {
        return std::nullopt;
    }

    return recoveryHits;
}


}

