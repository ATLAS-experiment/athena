/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SegmentFittingAlg.h"

#include <TrkEventPrimitives/ParticleHypothesis.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>

#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonSpacePoint/UtilFunctions.h>

#include <xAODMuonPrepData/RpcMeasurement.h>
#include <xAODMuonPrepData/TgcStrip.h>
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include <GaudiKernel/PhysicalConstants.h>
#include <Minuit2/Minuit2Minimizer.h>
#include <Math/Minimizer.h>

#include <MuonVisualizationHelpersR4/VisualizationHelpers.h>

namespace MuonR4 {
    using namespace SegmentFit;
    using namespace MuonValR4;

    MuonR4::SegmentFitResult::HitVec copy(const MuonR4::SegmentFitResult::HitVec& hits) {
        MuonR4::SegmentFitResult::HitVec copied{};
        copied.reserve(hits.size());
        std::ranges::transform(hits, std::back_inserter(copied), 
                               [](const auto& hit) { return std::make_unique<MuonR4::CalibratedSpacePoint>(*hit);});
        return copied;
    }
    MuonR4::SegmentFitResult copy(const MuonR4::SegmentFitResult& toCopy) {
        MuonR4::SegmentFitResult toRet{};
        toRet.segmentPars = toCopy.segmentPars;
        toRet.calibMeasurements = copy(toCopy.calibMeasurements);
        toRet.chi2 = toCopy.chi2;
        toRet.nDoF = toCopy.nDoF;
        toRet.timeFit = toCopy.timeFit;
        return toRet;
    }
    bool removeBeamSpot(MuonR4::SegmentFitResult::HitVec& hits){
        std::size_t before = hits.size();
        hits.erase(std::remove_if(hits.begin(), hits.end(),
                [](const auto& a){
                    return a->type() == xAOD::UncalibMeasType::Other;
                }), hits.end());
        return before != hits.size();
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
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentFittingAlg::execute(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));
        const SegmentSeedContainer* segmentSeeds=nullptr; 
        ATH_CHECK(retrieveContainer(ctx, m_seedKey, segmentSeeds));
    
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
                        const auto [locPos, locDir] = makeLine(pars);

                        segmentLines.emplace_back(drawLine(pars, -Gaudi::Units::m, Gaudi::Units::m, kRed));
                        std::stringstream signStream{};
                        signStream<<std::format("#chi^{{2}}/nDoF: {:.2f} ({:}), ", seg->chi2() / seg->nDoF(), seg->nDoF());
                        signStream<<std::format("y_{{0}}={:.2f}",pars[toInt(ParamDefs::y0)])<<", ";
                        signStream<<std::format("#theta={:.2f}^{{#circ}}", pars[toInt(ParamDefs::theta)]/ Gaudi::Units::deg )<<", ";
                        for (const Segment::MeasType& m : seg->measurements()) {
                            if (m->type() == xAOD::UncalibMeasType::MdtDriftCircleType && m->fitState() == CalibratedSpacePoint::State::Valid) {
                                signStream<<(SegmentFitHelpers::driftSign(locPos, locDir, *m, msgStream()) == -1 ? "L" : "R");
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
            } else  if (m_visionTool.isEnabled() && segments.empty() &&
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

    template <class ContainerType>
        StatusCode SegmentFittingAlg::retrieveContainer(const EventContext& ctx, 
                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                        const ContainerType*& contToPush) const {
            contToPush = nullptr;
            if (key.empty()) {
                ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
                return StatusCode::SUCCESS;
            }
            SG::ReadHandle readHandle{key, ctx};
            ATH_CHECK(readHandle.isPresent());
            contToPush = readHandle.cptr();
            return StatusCode::SUCCESS;
        }

    SegmentFitResult SegmentFittingAlg::fitSegmentHits(const EventContext& ctx,
                                                       const ActsGeometryContext& gctx,
                                                       const Parameters& startPars,
                                                       SegmentFitResult::HitVec&& calibHits) const {
        SegmentFitResult data{};
        if (calibHits.empty()) {
            ATH_MSG_WARNING("No hits, no segment");
            return data;
        }

        /** Add beamspot if neccessary */
        if (m_doBeamspotConstraint) {
            unsigned int numPhi = std::accumulate(calibHits.begin(), calibHits.end(),0,
                                                [](unsigned int n, const HitVec::value_type& hit){
                                                    return n + (hit->fitState() == CalibratedSpacePoint::State::Valid &&
                                                                hit->measuresPhi());
                                                });
            if (numPhi) {
                const Amg::Transform3D globToLoc{calibHits[0]->spacePoint()->msSector()->globalToLocalTrans(gctx)};
                Amg::Vector3D beamSpot{globToLoc.translation()};
                AmgSymMatrix(3) covariance{AmgSymMatrix(3)::Identity()}; 
                /// placeholder for a very generous beam spot: 300mm in X,Y (tracking volume), 20000 along Z
                covariance(0,0) = std::pow(m_beamSpotR, 2);
                covariance(1,1) = std::pow(m_beamSpotR, 2);
                covariance(2,2) = std::pow(m_beamSpotL, 2);
                AmgSymMatrix(3) jacobian =  globToLoc.linear();
                covariance = jacobian * covariance * jacobian.transpose(); 
                AmgSymMatrix(2) beamSpotCov{covariance.block<2,2>(0,0)};
                auto beamSpotSP = std::make_unique<CalibratedSpacePoint>(nullptr, std::move(beamSpot), Amg::Vector3D::Zero());
                beamSpotSP->setCovariance<2>(std::move(beamSpotCov));
                ATH_MSG_VERBOSE("Beam spot constraint "<<Amg::toString(beamSpotSP->positionInChamber())<<", "<<toString(beamSpotSP->covariance()));
                calibHits.push_back(std::move(beamSpotSP));
            }
        }

        const Amg::Transform3D& locToGlob{calibHits[0]->spacePoint()->msSector()->localToGlobalTrans(gctx)};

        MdtSegmentFitter::Config fitCfg{};
        fitCfg.calibrator = m_calibTool.get();
        fitCfg.doTimeFit = m_doT0Fit;
        fitCfg.reCalibrate = m_recalibInFit;

        MdtSegmentFitter fitter{name(), std::move(fitCfg)};
        return fitter.fitSegment(ctx, std::move(calibHits), startPars, locToGlob);
    }
    std::vector<std::unique_ptr<Segment>>
         SegmentFittingAlg::fitSegmentSeed(const EventContext& ctx,
                                           const ActsGeometryContext& gctx,
                                           const SegmentSeed* patternSeed) const {

        const Amg::Transform3D& locToGlob{patternSeed->msSector()->localToGlobalTrans(gctx)};
        std::vector<std::unique_ptr<Segment>> segments{};

        MdtSegmentSeedGenerator::Config genCfg{};
        genCfg.hitPullCut = m_seedHitChi2;
        genCfg.recalibSeedCircles = m_recalibSeed;
        genCfg.fastSeedFit = m_refineSeed;
        genCfg.fastSegFitWithT0 = m_doT0Fit;
        genCfg.calibrator = m_calibTool.get();
        

        /// At very high inclanation angles, the muon may traverse 3 hits in the same layer (E.g. BEE)
        genCfg.busyLayerLimit = 2 + 2*(patternSeed->parameters()[toInt(ParamDefs::theta)] > 50 * Gaudi::Units::deg);
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
        while (auto seed = seedGen.nextSeed(ctx)) {
            SegmentFitResult data{};
            data.segmentPars = seed->parameters;
            data.calibMeasurements = std::move(seed->measurements);            
            /// Draw the prefit
             if (m_visionTool.isEnabled()) {
                auto seedCopy = convertToSegment(locToGlob, patternSeed, copy(data));
                m_visionTool->visualizeSegment(ctx, *seedCopy, std::format("Pre fit {:d}", seedGen.numGenerated()));
            } 
            data = fitSegmentHits(ctx, gctx, seed->parameters, std::move(data.calibMeasurements));
            data.nIter +=  seed->nIter;
             if (m_visionTool.isEnabled() && data.converged) {
                auto seedCopy = convertToSegment(locToGlob, patternSeed, copy(data));
                m_visionTool->visualizeSegment(ctx, *seedCopy, std::format("Intermediate fit {:d}", seedGen.numGenerated()));
            }
            if (!removeOutliers(ctx, gctx, *patternSeed, data)) {
                continue;
            }          
            if (!plugHoles(ctx, gctx, *patternSeed, data)) {
                continue;
            }
             if (m_visionTool.isEnabled()) {
                auto seedCopy = convertToSegment(locToGlob, patternSeed, copy(data));
                m_visionTool->visualizeSegment(ctx, *seedCopy, std::format("Final fit {:d}", seedGen.numGenerated()));
            }
            segments.push_back(convertToSegment(locToGlob, patternSeed, std::move(data)));
        }
        return segments;
    }
     std::unique_ptr<Segment> SegmentFittingAlg::convertToSegment(const Amg::Transform3D& locToGlob, 
                                                                  const SegmentSeed* patternSeed,
                                                                  SegmentFitResult&& data) {
        const auto [locPos, locDir] = data.makeLine();
        Amg::Vector3D globPos = locToGlob * locPos;
        Amg::Vector3D globDir = locToGlob.linear()* locDir;


        auto finalSeg = std::make_unique<Segment>(std::move(globPos), std::move(globDir),
                                                  patternSeed, std::move(data.calibMeasurements),
                                                  data.chi2, data.nDoF);
        finalSeg->setCallsToConverge(data.nIter);
        finalSeg->setParUncertainties(std::move(data.segmentParErrs));
        if (data.timeFit) {
            finalSeg->setSegmentT0(data.segmentPars[toInt(ParamDefs::time)]);
        }
        return finalSeg;
    }

    bool SegmentFittingAlg::removeOutliers(const EventContext& ctx,
                                           const ActsGeometryContext& gctx,
                                           const SegmentSeed& seed,
                                           SegmentFitResult& data) const {
        
        /** If no degree of freedom is in the segment fit then try to plug the holes  */
        if (data.nDoF<=0 || data.calibMeasurements.empty()) {
            ATH_MSG_VERBOSE("No degree of freedom available. What shall be removed?!. nDoF: "
                            <<data.nDoF<<", n-meas: "<<data.calibMeasurements);
            return false;
        }

        const auto [segPos, segDir] = data.makeLine();

        if (data.converged && data.chi2 / data.nDoF < m_outlierRemovalCut) {
            ATH_MSG_VERBOSE("The segment "<<Amg::toString(segPos)<<" + "<<Amg::toString(segDir)
                            <<" is already of good quality "<<data.chi2 / std::max(data.nDoF, 1)
                            <<". Don't remove outliers");
            return true;
        }
        ATH_MSG_VERBOSE("Segment "<<toString(data.segmentPars)<<" is of badish quality.");
        /** Remove a priori the beamspot constaint as it never should pose any problem and
         *  another one will be added anyway in the next iteration */        
        if (m_doBeamspotConstraint && removeBeamSpot(data.calibMeasurements)) {
            data.nDoF-=2;
            data.nPhiMeas-=1;
        }

        /** Next sort the measurements by chi2 */
        std::sort(data.calibMeasurements.begin(), data.calibMeasurements.end(),
                  [&, this](const HitVec::value_type& a, const HitVec::value_type& b){
                    return SegmentFitHelpers::chiSqTerm(segPos, segDir, data.segmentPars[toInt(ParamDefs::time)], std::nullopt, *a, msgStream()) <
                           SegmentFitHelpers::chiSqTerm(segPos, segDir, data.segmentPars[toInt(ParamDefs::time)], std::nullopt, *b, msgStream());
                  });
        
        /** Declare the hit with the largest chi2 as outlier. */
        data.calibMeasurements.back()->setFitState(CalibratedSpacePoint::State::Outlier);
        data.nDoF -= data.calibMeasurements.back()->measuresEta();
        data.nDoF -= data.calibMeasurements.back()->measuresPhi();
        if (m_doT0Fit && data.calibMeasurements.back()->measuresTime() && 
                         data.calibMeasurements.back()->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
            --data.nDoF;
        }
        /** Remove the last measurement as it has the largest discrepancy */
        std::vector<HoughHitType> uncalib{};
        for (const HitVec::value_type& calib : data.calibMeasurements) {
           uncalib.push_back(calib->spacePoint());
        }
        SegmentFitResult newAttempt = fitSegmentHits(ctx, gctx, data.segmentPars, std::move(data.calibMeasurements));
        if (newAttempt.converged) {
            newAttempt.nIter+=data.nIter;
            data = std::move(newAttempt);
             if (m_visionTool.isEnabled()) {
                auto seedCopy = convertToSegment(seed.msSector()->localToGlobalTrans(gctx), &seed, copy(data));
                m_visionTool->visualizeSegment(ctx, *seedCopy, "Bad fit recovery");
            }
        } else {
            data.calibMeasurements = std::move(newAttempt.calibMeasurements);
        }
        return removeOutliers(ctx, gctx, seed, data);
    }

    bool SegmentFittingAlg::plugHoles(const EventContext& ctx,
                                      const ActsGeometryContext& gctx,
                                      const SegmentSeed& seed,
                                      SegmentFitResult& beforeRecov) const {
        /** We've the first estimator of the segment fit */
        ATH_MSG_VERBOSE("plugHoles() -- segment "<<toString(beforeRecov.segmentPars)
                        <<", chi2: "<<beforeRecov.chi2<<", nDoF: "<<beforeRecov.nDoF<<", "
                        <<beforeRecov.chi2 /std::max(beforeRecov.nDoF, 1) );
        /** Setup a map to replace space points if they better suite */
        std::unordered_set<const SpacePoint*> usedSpacePoint{};
        for (const HitVec::value_type& hit : beforeRecov.calibMeasurements) {
            usedSpacePoint.insert(hit->spacePoint());
        }

        HitVec candidateHits{};
        SpacePointPerLayerSplitter hitLayers{*seed.parentBucket()};
        bool hasCandidate{false};
        const auto [locPos, locDir] = beforeRecov.makeLine();
        for (const std::vector<HoughHitType>& mdtLayer : hitLayers.mdtHits()) {
            for (const SpacePoint* mdtHit: mdtLayer) {
                /// Hit is already used in the segment fit
                if (usedSpacePoint.count(mdtHit)) {
                    continue;
                }
                const double dist = Amg::lineDistance(locPos, locDir, mdtHit->positionInChamber(), mdtHit->directionInChamber());
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(mdtHit->primaryMeasurement());
                if (dist >= dc->readoutElement()->innerTubeRadius()) {
                    continue;
                }
                HitVec::value_type calibHit{m_calibTool->calibrate(ctx, mdtHit, locPos, locDir, beforeRecov.segmentPars[toInt(ParamDefs::time)])};
                const double pull = std::sqrt(SegmentFitHelpers::chiSqTermMdt(locPos, locDir, *calibHit, msgStream()));
                ATH_MSG_VERBOSE(__func__<<"() :"<<__LINE__<<" Candidate hit for recovery "<<m_idHelperSvc->toString(mdtHit->identify())<<", chi2: "<<pull);
                if (pull <= m_recoveryPull) {
                    hasCandidate |= calibHit->fitState() == CalibratedSpacePoint::State::Valid;
                    candidateHits.push_back(std::move(calibHit));
                } else {
                    calibHit->setFitState(CalibratedSpacePoint::State::Outlier);
                    candidateHits.push_back(std::move(calibHit));
                }
            }
        }
        if (!hasCandidate) {
            ATH_MSG_VERBOSE("No space point candidates for recovery were found");
            beforeRecov.calibMeasurements.insert(beforeRecov.calibMeasurements.end(), 
                                                 std::make_move_iterator(candidateHits.begin()),
                                                 std::make_move_iterator(candidateHits.end()));
            eraseWrongHits(beforeRecov);
            return beforeRecov.nDoF > 0;
        }
   
        HitVec copied = copy(beforeRecov.calibMeasurements), copiedCandidates = copy(candidateHits);
        /// Remove the beamspot constraint measurement
        if (m_doBeamspotConstraint) {
            removeBeamSpot(copied);
        }

        candidateHits.insert(candidateHits.end(), std::make_move_iterator(copied.begin()), std::make_move_iterator(copied.end()));

        SegmentFitResult recovered = fitSegmentHits(ctx, gctx, beforeRecov.segmentPars, std::move(candidateHits));
        if (!recovered.converged) {
            return false;
        }
        /** Nothing has been recovered. Just bail out */
        if (recovered.nDoF + recovered.timeFit <= beforeRecov.nDoF + beforeRecov.timeFit) {
            for (HitVec::value_type& hit : copiedCandidates) {
                hit->setFitState(CalibratedSpacePoint::State::Outlier);
                beforeRecov.calibMeasurements.push_back(std::move(hit));
            }
            eraseWrongHits(beforeRecov);
            return true;
        }
        ATH_MSG_VERBOSE("Chi2, nDOF before:"<<beforeRecov.chi2<<", "<<beforeRecov.nDoF<<" after recovery: "<<recovered.chi2<<", "<<recovered.nDoF);
        double redChi2 = recovered.chi2 / std::max(recovered.nDoF,1);
        /// If the chi2 is less than 5, no outlier rejection is launched. So also accept any recovered segment below that threshold
        if (redChi2 < m_outlierRemovalCut || (beforeRecov.nDoF == 0) || redChi2 < beforeRecov.chi2 / beforeRecov.nDoF) {
            ATH_MSG_VERBOSE("Accept segment with recovered "<<(recovered.nDoF + recovered.timeFit) - (beforeRecov.nDoF + beforeRecov.timeFit)<<" hits.");
            recovered.nIter += beforeRecov.nIter;
            beforeRecov = std::move(recovered);
            /** Next check whether the recovery made measurements marked as outlier feasable for the hole recovery*/
            while (true) {
                bool runAnotherTrial = false;
                copied = copy(beforeRecov.calibMeasurements);
                if (m_doBeamspotConstraint) {
                    removeBeamSpot(copied);
                }
                const auto [beforePos, beforeDir] = beforeRecov.makeLine();
                for (HitVec::value_type& copyHit : copied) {
                    if (copyHit->fitState() != CalibratedSpacePoint::State::Outlier) {
                        continue;
                    }
                    copyHit->setFitState(CalibratedSpacePoint::State::Valid);
                    bool append = std::sqrt(SegmentFitHelpers::chiSqTerm(beforePos, beforeDir, 
                                                                        beforeRecov.segmentPars[toInt(ParamDefs::time)], 
                                                                        std::nullopt, *copyHit, msgStream())) < m_recoveryPull;
                    if (append) {
                        runAnotherTrial = true;    
                    } else {
                        copyHit->setFitState(CalibratedSpacePoint::State::Outlier);
                    }
                }
                if (!runAnotherTrial) {
                    break;
                }
                recovered = fitSegmentHits(ctx, gctx, beforeRecov.segmentPars, std::move(copied));
                if (!recovered.converged) {
                    break;
                }
                if (recovered.nDoF + recovered.timeFit <= beforeRecov.nDoF + beforeRecov.timeFit) {
                    break;
                }
                redChi2 = recovered.chi2 / std::max(recovered.nDoF, 1);
                if (redChi2 < m_outlierRemovalCut || redChi2 < beforeRecov.chi2 / beforeRecov.nDoF) {
                    recovered.nIter += beforeRecov.nIter;
                    beforeRecov = std::move(recovered);
                } else {
                    break;
                }
            }
            /** Finally remove all hits from the calib measurements which are obvious outliers */
            eraseWrongHits(beforeRecov);
        } else{
            for (HitVec::value_type& hit : copiedCandidates) {
                hit->setFitState(CalibratedSpacePoint::State::Outlier);
                beforeRecov.calibMeasurements.push_back(std::move(hit));
            }
        }
        return true;
    }
    void SegmentFittingAlg::eraseWrongHits(SegmentFitResult& candidate) const {
        auto [segPos, segDir] = makeLine(candidate.segmentPars);
        candidate.calibMeasurements.erase(std::remove_if(candidate.calibMeasurements.begin(), candidate.calibMeasurements.end(),
                                                [&segPos, &segDir](const HitVec::value_type& hit){
                                                if (hit->fitState() != CalibratedSpacePoint::State::Outlier) {
                                                    return false;
                                                }
                                                /** The segment has never crossed the tube */
                                                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                                                    const double dist = Amg::lineDistance(segPos, segDir, hit->positionInChamber(), hit->directionInChamber());
                                                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->spacePoint()->primaryMeasurement());
                                                    return dist >= dc->readoutElement()->innerTubeRadius();
                                                }
                                                return true;
                                                }), candidate.calibMeasurements.end());

        std::ranges::sort(candidate.calibMeasurements, [](const Segment::MeasType& a, const Segment::MeasType& b){
            return a->positionInChamber().z() < b->positionInChamber().z();
        });
    }
    void SegmentFittingAlg::resolveAmbiguities(const ActsGeometryContext& gctx,
                                               std::vector<std::unique_ptr<Segment>>& segmentCandidates) const {
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
