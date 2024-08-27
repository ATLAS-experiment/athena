/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SegmentFittingAlg.h"

#include <TrkEventPrimitives/ParticleHypothesis.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>

#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/CalibSegmentChi2Minimizer.h>
#include <MuonPatternHelpers/SegmentAmbiSolver.h>
#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>


#include <xAODMuonPrepData/RpcMeasurement.h>
#include <xAODMuonPrepData/TgcStrip.h>

#include <GaudiKernel/PhysicalConstants.h>
#include <Minuit2/Minuit2Minimizer.h>
#include <Math/Minimizer.h>

#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include "TCanvas.h"
#include "TLine.h"
#include "TArrow.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLatex.h"
#include "TH1F.h"
#include "TBox.h"



namespace {
    constexpr double inv_c = 1./ Gaudi::Units::c_light;
    
    std::unique_ptr<TLine> drawLine(const MuonR4::SegmentFit::Parameters& pars,
                                    const double z1 = 300.*Gaudi::Units::mm,
                                    const double z2 = -300.*Gaudi::Units::mm,
                                    const int color = kViolet) {
        using namespace MuonR4::SegmentFit;
        const double y1 = pars[toInt(AxisDefs::y0)]  + z1 *pars[toInt(AxisDefs::tanTheta)];
        const double y2 = pars[toInt(AxisDefs::y0)]  + z2 *pars[toInt(AxisDefs::tanTheta)];
        auto seedLine = std::make_unique<TLine>(y1,z1, y2, z2);
        seedLine->SetLineColor(color);
        seedLine->SetLineWidth(2);
        seedLine->SetLineStyle(7);
        return seedLine;
    }
    std::unique_ptr<TLatex> drawLabel(const std::string& text, const double xPos, const double yPos,
                                      unsigned int fontSize = 18) {
        auto tl = std::make_unique<TLatex>(xPos, yPos, text.c_str());
        tl->SetNDC();
        tl->SetTextFont(53); 
        tl->SetTextSize(fontSize); 
        return tl;
    }
    MuonR4::SegmentFitResult::HitVec copy(const MuonR4::SegmentFitResult::HitVec& hits) {
        MuonR4::SegmentFitResult::HitVec copied{};
        copied.reserve(hits.size());
        std::ranges::transform(hits, std::back_inserter(copied), 
                               [](const auto& hit) { return std::make_unique<MuonR4::CalibratedSpacePoint>(*hit);});
        return copied;
    }
    bool removeBeamSpot(MuonR4::SegmentFitResult::HitVec& hits){
        std::size_t before = hits.size();
        hits.erase(std::remove_if(hits.begin(), hits.end(),
                [](const auto& a){
                    return a->type() == xAOD::UncalibMeasType::Other;
                }), hits.end());
        return before != hits.size();
    }  
}

namespace MuonR4{
    using namespace SegmentFit;
 
    SegmentFittingAlg::~SegmentFittingAlg() = default;
    SegmentFittingAlg::SegmentFittingAlg(const std::string& name, ISvcLocator* pSvcLocator): 
            AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode SegmentFittingAlg::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_seedKey.initialize());
        ATH_CHECK(m_outSegments.initialize());  
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_truthSegKey.initialize(!m_truthSegKey.empty()));
        m_allCan = std::make_unique<TCanvas>("all", "all", 800,600.);
        m_allCan->SaveAs( (m_allCanName +".pdf[").c_str());
        return StatusCode::SUCCESS;
    }
    StatusCode SegmentFittingAlg::finalize() {
        m_allCan->SaveAs( (m_allCanName +".pdf]").c_str());
        return StatusCode::SUCCESS;
    }

    StatusCode SegmentFittingAlg::execute(const EventContext& ctx) const {
    
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));
        const SegmentSeedContainer* segmentSeeds=nullptr; 
        ATH_CHECK(retrieveContainer(ctx, m_seedKey, segmentSeeds));
    
        SG::WriteHandle<SegmentContainer> writeSegments{m_outSegments, ctx};
        ATH_CHECK(writeSegments.record(std::make_unique<SegmentContainer>()));
        std::vector<std::unique_ptr<Segment>> allSegments{};
        for (const SegmentSeed* seed : *segmentSeeds) {
            std::vector<std::unique_ptr<Segment>> segments = fitSegmentSeed(ctx, *gctx, seed);
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
            SG::ReadHandle<ContainerType> readHandle{key, ctx};
            ATH_CHECK(readHandle.isPresent());
            contToPush = readHandle.cptr();
            return StatusCode::SUCCESS;
        }


   SegmentFitResult SegmentFittingAlg::fitSegmentHits(const EventContext& ctx,
                                                       const ActsGeometryContext& gctx,
                                                       const Parameters& startPars,
                                                       const std::vector<HoughHitType>& unCalibHits) const {
        const auto [seedPos, seedDir] = makeLine(startPars);
        return fitSegmentHits(ctx, gctx, startPars, m_calibTool->calibrate(ctx, unCalibHits, seedPos, seedDir, 
                                                                            startPars[toInt(AxisDefs::time)]));
        
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
            if (numPhi > 1 && numPhi < 3) {
                const Amg::Transform3D globToLoc{calibHits[0]->spacePoint()->chamber()->globalToLocalTrans(gctx)};
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
                calibHits.push_back(std::move(beamSpotSP));
            }
        }

        const Amg::Transform3D& locToGlob{calibHits[0]->spacePoint()->chamber()->localToGlobalTrans(gctx)};

        if (!m_useMinuit) {
            MdtSegmentFitter::Config fitCfg{};
            fitCfg.calibrator = m_calibTool.get();
            fitCfg.doTimeFit = m_doT0Fit;

            MdtSegmentFitter fitter{name(), std::move(fitCfg)};
            return fitter.fitSegment(ctx, std::move(calibHits), startPars, locToGlob);
        }

        data.segmentPars = startPars;

        CalibSegmentChi2Minimizer c2f{name(), ctx, locToGlob, copy(calibHits), m_calibTool.get(), m_doT0Fit};
        data.hasPhi = c2f.hasPhiMeas();
        data.timeFit = c2f.doTimeFit();
        data.nDoF = c2f.nDoF();
        if (data.nDoF <= 0) {
            ATH_MSG_DEBUG("Reject fit due to 0 degrees of freedom");
            return data;
        }


        ROOT::Minuit2::Minuit2Minimizer minimizer((name() + std::to_string(ctx.eventID().event_number())).c_str());

    
        /** Configure the minimizer */
        minimizer.SetMaxFunctionCalls(100000);
        minimizer.SetTolerance(0.0001);
        minimizer.SetPrintLevel(-1);
        minimizer.SetStrategy(1);

        minimizer.SetVariable(toInt(AxisDefs::y0), "y0", startPars[toInt(AxisDefs::y0)], 1.e-5);
        minimizer.SetVariable(toInt(AxisDefs::tanTheta), "tanTheta", startPars[toInt(AxisDefs::tanTheta)], 1.e-5);
        minimizer.SetVariableLimits(toInt(AxisDefs::y0), 
                                    startPars[toInt(AxisDefs::y0)] - 60. *Gaudi::Units::cm, 
                                    startPars[toInt(AxisDefs::y0)] + 60. *Gaudi::Units::cm);
        minimizer.SetVariableLimits(toInt(AxisDefs::tanTheta),
                                    startPars[toInt(AxisDefs::tanTheta)] - 0.6, 
                                    startPars[toInt(AxisDefs::tanTheta)] + 0.6);
        
        if (data.hasPhi) {
            minimizer.SetVariable(toInt(AxisDefs::x0), "x0", startPars[toInt(AxisDefs::x0)], 1.e-5);
            minimizer.SetVariable(toInt(AxisDefs::tanPhi), "tanPhi", startPars[toInt(AxisDefs::tanPhi)], 1.e-5);
            minimizer.SetVariableLimits(toInt(AxisDefs::x0), 
                                        startPars[toInt(AxisDefs::x0)] - 600, 
                                        startPars[toInt(AxisDefs::x0)] + 600);
            minimizer.SetVariableLimits(toInt(AxisDefs::tanPhi), 
                                        startPars[toInt(AxisDefs::tanPhi)] - 0.6, 
                                        startPars[toInt(AxisDefs::tanPhi)] + 0.6);
        } else {
            minimizer.SetFixedVariable(toInt(AxisDefs::x0), "x0", 0.);
            minimizer.SetFixedVariable(toInt(AxisDefs::tanPhi), "tanPhi", 0.);
        }
        /// Assumption that the particle travels at the speed of light
        if (data.timeFit) {
            minimizer.SetVariable(toInt(AxisDefs::time), "t0", startPars[toInt(AxisDefs::time)] , 1.e-5 );
            minimizer.SetVariableLimits(toInt(AxisDefs::time), 
                                        startPars[toInt(AxisDefs::time)] - 12.5,
                                        startPars[toInt(AxisDefs::time)] + 12.5);
        } else{
            minimizer.SetFixedVariable(toInt(AxisDefs::time), "t0", 0.);
        }
        minimizer.SetFunction(c2f);
        /// Execute fit
        if (!minimizer.Minimize() || !minimizer.Hesse()) {
            data.calibMeasurements = std::move(calibHits);
        } else {
            const double* xs = minimizer.X();
            const double* errs = minimizer.Errors();

            for (unsigned int p = 0; p < toInt(AxisDefs::nPars); ++p) {
                data.segmentPars[p] = xs[p];
                data.segmentParErrs(p,p) = errs[p];
            }
            std::optional<double> timeOfArrival{std::nullopt};
            const auto [locPos, locDir] = data.makeLine();
            if (data.timeFit) {
                timeOfArrival = std::make_optional<double>((locToGlob*locPos).mag() * inv_c);
            }
            data.nIter = minimizer.NCalls();
            data.calibMeasurements = c2f.release(xs);

            auto [chiPerMeas, finalChi2] = SegmentFitHelpers::postFitChi2PerMas(data.segmentPars, timeOfArrival, 
                                                                                data.calibMeasurements, msgStream());
            data.chi2PerMeasurement = std::move(chiPerMeas);
            data.chi2 = finalChi2;
            data.converged = true;
        }
        return data;
    }
    std::vector<std::unique_ptr<Segment>>
         SegmentFittingAlg::fitSegmentSeed(const EventContext& ctx,
                                           const ActsGeometryContext& gctx,
                                           const SegmentSeed* patternSeed) const {

    
        const Amg::Transform3D& locToGlob{patternSeed->chamber()->localToGlobalTrans(gctx)};
        std::vector<std::unique_ptr<Segment>> segments{};

        MdtSegmentSeedGenerator::Config genCfg{};
        genCfg.chi2PerHit = m_seedHitChi2;
        genCfg.interceptReso = m_seedY0Reso;
        genCfg.tanThetaReso = m_seedTanThetaReso;
        genCfg.calibrator = m_calibTool.get();
        /** Draw the pattern with all possible seeds */
        if (m_canvCounter < m_nDrawCanvases) {
            SegmentFitResult data{};
            data.segmentPars = patternSeed->parameters();
            const Amg::Vector3D seedPos{patternSeed->positionInChamber()};
            const Amg::Vector3D seedDir{patternSeed->directionInChamber()};

        
            data.calibMeasurements = m_calibTool->calibrate(ctx, patternSeed->getHitsInMax(), seedPos, seedDir, 
                                                            data.segmentPars[toInt(AxisDefs::time)]);

            for (const std::unique_ptr<CalibratedSpacePoint>& meas : data.calibMeasurements) {
                data.chi2PerMeasurement.push_back(SegmentFitHelpers::chiSqTerm(seedPos, seedDir, 0., std::nullopt, *meas, msgStream()));
            } 
            data.chi2 = std::accumulate(data.chi2PerMeasurement.begin(), data.chi2PerMeasurement.end(), 0.);
            data.nDoF = 1;

            // /** Draw the full pattern seed */
            std::vector<std::unique_ptr<TObject>> seedLines{};
            MdtSegmentSeedGenerator drawMe{name(), patternSeed, genCfg};
            while(auto s = drawMe.nextSeed(ctx)) {
                seedLines.push_back(drawLine(s->parameters()));
            }
            seedLines.push_back(drawLabel("possible seeds: "+std::to_string(drawMe.numGenerated()),0.15, 0.85, 14));
            visualizeFit(ctx, data, patternSeed->parentBucket(), "pattern", std::move(seedLines));
        }
        MdtSegmentSeedGenerator seedGen{name(),patternSeed, std::move(genCfg)};

        while (const std::optional<SegmentSeed> seed = seedGen.nextSeed(ctx)) {

            SegmentFitResult data{};
            data.segmentPars = seed->parameters();
            const auto [seedPos, seedDir] = SegmentFit::makeLine(data.segmentPars);
            /// Draw the prefit
            if (m_canvCounter < m_nDrawCanvases) {
                data.calibMeasurements = m_calibTool->calibrate(ctx, seed->getHitsInMax(), seedPos, seedDir, 
                                                                data.segmentPars[toInt(AxisDefs::time)]);
           

                for (const std::unique_ptr<CalibratedSpacePoint>& meas : data.calibMeasurements) {
                    data.chi2PerMeasurement.push_back(SegmentFitHelpers::chiSqTerm(seedPos, seedDir, 0., std::nullopt, *meas, msgStream()));
                }
                data.chi2 = std::accumulate(data.chi2PerMeasurement.begin(), data.chi2PerMeasurement.end(), 0.);

                std::vector<std::unique_ptr<TObject>> primitives{};
                primitives.push_back(drawLine(seed->parameters()));

                visualizeFit(ctx, data, patternSeed->parentBucket(), "prefit", std::move(primitives));
            }
            data = fitSegmentHits(ctx, gctx, seed->parameters(), seed->getHitsInMax());
            if (data.converged) {
                visualizeFit(ctx, data, patternSeed->parentBucket(), "Intermediate fit");
            }
            if (!removeOutliers(ctx, gctx, *patternSeed, data)) {
                continue;
            }          
            if (!plugHoles(ctx, gctx, *patternSeed, data)) {
                continue;
            }

            visualizeFit(ctx, data, patternSeed->parentBucket(), "Final fit");

            const auto [locPos, locDir] = data.makeLine();
            Amg::Vector3D globPos = locToGlob * locPos;
            Amg::Vector3D globDir = locToGlob.linear()* locDir;


            auto finalSeg = std::make_unique<Segment>(std::move(globPos), std::move(globDir),
                                                      patternSeed,
                                                      std::move(data.calibMeasurements),
                                                      data.chi2,
                                                      data.nDoF);
            finalSeg->setCallsToConverge(data.nIter);
            finalSeg->setChi2PerMeasurement(std::move(data.chi2PerMeasurement));
            finalSeg->setParUncertainties(std::move(data.segmentParErrs));
            if (data.timeFit) {
                finalSeg->setSegmentT0(data.segmentPars[toInt(AxisDefs::time)]);
            }
            segments.push_back(std::move(finalSeg));

        }
        return segments;
    }

    bool SegmentFittingAlg::removeOutliers(const EventContext& ctx,
                                           const ActsGeometryContext& gctx,
                                           const SegmentSeed& seed,
                                           SegmentFitResult& data) const {
        
        /** If no degree of freedom is in the segment fit then try to plug the holes  */
        if (data.nDoF<=0 || data.calibMeasurements.empty()) {
            ATH_MSG_VERBOSE("No degree of freedom available. What shall be removed?!");
            return false;
        }

        const auto [segPos, segDir] = data.makeLine();

        if (data.converged && data.chi2 / data.nDoF < m_outlierRemovalCut) {
            ATH_MSG_VERBOSE("The segment "<<Amg::toString(segPos)<<" + "<<Amg::toString(segDir)
                            <<" is already of good quality "<<data.chi2 / std::max(data.nDoF, 1)
                            <<". Don't remove outliers");
            return true;
        }
        /** Remove a priori the beamspot constaint as it never should pose any problem and
         *  another one will be added anyway in the next iteration */        
        if (m_doBeamspotConstraint && removeBeamSpot(data.calibMeasurements)) {
            data.nDoF-=2;
            data.nPhiMeas-=1;
        }

        /** Next sort the measurements by chi2 */
        std::sort(data.calibMeasurements.begin(), data.calibMeasurements.end(),
                  [&, this](const HitVec::value_type& a, const HitVec::value_type& b){
                    return SegmentFitHelpers::chiSqTerm(segPos, segDir, data.segmentPars[toInt(AxisDefs::time)], std::nullopt, *a, msgStream()) <
                           SegmentFitHelpers::chiSqTerm(segPos, segDir, data.segmentPars[toInt(AxisDefs::time)], std::nullopt, *b, msgStream());
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
            visualizeFit(ctx, data, seed.parentBucket(), "bad fit recovery");
        } else {
            data.calibMeasurements = std::move(newAttempt.calibMeasurements);
        }
        return removeOutliers(ctx, gctx, seed, data);
    }
    void SegmentFittingAlg::visualizeFit(const EventContext& ctx,
                                         const SegmentFitResult& fitResult,
                                         const SpacePointBucket* bucket,
                                         const std::string& extraLabel,
                                         std::vector<std::unique_ptr<TObject>> primitives) const {
        

        static std::mutex mutex{};
        if(m_canvCounter >= m_nDrawCanvases) return;
        std::lock_guard guard{mutex};
        if(m_canvCounter >= m_nDrawCanvases) return;
        
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        retrieveContainer(ctx, m_truthSegKey, truthSegs).ignore();
        
        
        std::unordered_set<const SpacePoint*> usedSpacePoint{};
        for (const HitVec::value_type& hit : fitResult.calibMeasurements) {
            usedSpacePoint.insert(hit->spacePoint());
        }
        std::unordered_set<const xAOD::MuonSimHit*> truthSpHits{getTruthMatchedHits(*bucket)};
        const xAOD::MuonSegment* truthMatched{nullptr};
        if (truthSegs) {
            unsigned int mostHits{0};
            for (const xAOD::MuonSegment* seg : *truthSegs) {
                std::unordered_set<const xAOD::MuonSimHit*> truthHits{getTruthMatchedHits(*seg)};
                const unsigned matchedHits = std::accumulate(truthHits.begin(), truthHits.end(),0 ,
                                                          [&truthSpHits](unsigned int n, const xAOD::MuonSimHit* simHit){
                                                            return n + truthSpHits.count(simHit);
                                                          });
                if (matchedHits > mostHits) {
                    mostHits = matchedHits;
                    truthMatched = seg;
                }
            }
            if (truthMatched) {
                truthSpHits =  getTruthMatchedHits(*truthMatched);
            } else {
                truthSpHits.clear();
                return;
            }
        }
        const auto [locPos, locDir] = fitResult.makeLine();

        ATH_MSG_VERBOSE("visualizeFit() -- segment "<<Amg::toString(locPos)<<", "<<Amg::toString(locDir)
                        <<", counter: "<<m_canvCounter<<", chi2: "<<fitResult.chi2<<", nDoF: "<<fitResult.nDoF<<", "
                        <<fitResult.chi2 /std::max(fitResult.nDoF, 1));

        double yMin{std::numeric_limits<double>::max()},
               yMax{-std::numeric_limits<double>::max()},
               zMin{std::numeric_limits<double>::max()},
               zMax{-std::numeric_limits<double>::max()};
        Identifier refId{};
        unsigned  updated{0};
        for (const SpacePointBucket::value_type& spInBucket : *bucket) {
            if (usedSpacePoint.count(spInBucket.get())) {
                refId = m_idHelperSvc->chamberId(spInBucket->identify());
            }
            const Amg::Vector3D& hitPos{spInBucket->positionInChamber()};
            if (refId == m_idHelperSvc->chamberId(spInBucket->identify())) {
                 yMin = std::min(yMin, hitPos.y());
                 yMax = std::max(yMax, hitPos.y());
                 zMax = std::max(zMax, hitPos.z());
                 zMin = std::min(zMin, hitPos.z());
                 ++updated;
            }
            switch (spInBucket->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    auto circle = std::make_unique<TEllipse>(hitPos.y(), hitPos.z(), spInBucket->driftRadius());
                    if (truthSpHits.count(getTruthMatchedHit(*spInBucket->primaryMeasurement()))){                        
                        circle->SetFillColor(kOrange +1); 
                        circle->SetLineColor(kOrange +1); 
                    } else {
                        circle->SetFillColor(kBlue); 
                        circle->SetLineColor(kBlue);
                    }
                    
                    circle->SetFillStyle(0);
                    if (usedSpacePoint.count(spInBucket.get())) {
                        circle->SetFillStyle(1001);
                        circle->SetFillColorAlpha(circle->GetFillColor(), 0.8);
                    }
                    primitives.insert(primitives.begin(), std::move(circle));
                    break;
                } case xAOD::UncalibMeasType::RpcStripType: {
                    const auto* prd = static_cast<const xAOD::RpcMeasurement*>(spInBucket->primaryMeasurement());
                    const double boxY = spInBucket->uncertainty().y();
                    const double boxZ = 0.5*prd->readoutElement()->gasGapPitch();
                    auto stripBox = std::make_unique<TBox>(hitPos.y() - boxY, hitPos.z() - boxZ,
                                                           hitPos.y() + boxY, hitPos.z() + boxZ);
                    stripBox->SetFillColor(kGreen +2);
                    stripBox->SetLineColor(kGreen +2);
                    stripBox->SetFillStyle(0);
                    if (usedSpacePoint.count(spInBucket.get())) {
                        stripBox->SetFillStyle(1001);
                        stripBox->SetFillColorAlpha(stripBox->GetFillColor(), 0.8);
                    }
                    primitives.insert(primitives.begin(), std::move(stripBox));
                    break;
                } case xAOD::UncalibMeasType::TgcStripType: {
                    const auto* prd = static_cast<const xAOD::TgcStrip*>(spInBucket->primaryMeasurement());
                    const double boxY = spInBucket->uncertainty().y();
                    const double boxZ = 0.5*prd->readoutElement()->gasGapPitch();
                    auto stripBox = std::make_unique<TBox>(hitPos.y() - boxY, hitPos.z() - boxZ,
                                                           hitPos.y() + boxY, hitPos.z() + boxZ);
                    stripBox->SetFillColor(kCyan +2);
                    stripBox->SetLineColor(kCyan +2);
                    stripBox->SetFillStyle(0);
                    if (usedSpacePoint.count(spInBucket.get())) {
                        stripBox->SetFillStyle(1001);
                        stripBox->SetFillColorAlpha(stripBox->GetFillColor(), 0.8);
                    }
                    primitives.insert(primitives.begin(), std::move(stripBox));
                    break;
                } default: 
                    continue;
            }
        }
        if (updated < 2) {
            --m_canvCounter;
            return;
        }
        auto  myCanvas = std::make_unique<TCanvas>("can","can",800,600); 
        myCanvas->cd();

        double width = (yMax - yMin)*1.3;
        double height = (zMax - zMin)*1.3;
        if (height > width) width = height; 
        else height = width;

        const double midPointY = 0.5 * (yMax + yMin);
        const double midPointZ = 0.5 * (zMax + zMin);
        const double y0 = midPointY - 0.5 * width;
        const double z0 = midPointZ - 0.5 * height;
        const double y1 = midPointY + 0.5 * width;
        const double z1 = midPointZ + 0.5 * height;
        auto frame = myCanvas->DrawFrame(y0,z0,y1,z1);
        frame->GetXaxis()->SetTitle("y [mm]");
        frame->GetYaxis()->SetTitle("z [mm]");

        primitives.push_back(drawLine(fitResult.segmentPars, z0, z1, kRed));
        if (truthMatched){
            static const SG::Accessor<xAOD::MeasVector<toInt(AxisDefs::nPars)>> acc_localSegPars{"localSegPars"};
            primitives.push_back(drawLine(xAOD::toEigen(xAOD::ConstVectorMap<toInt(AxisDefs::nPars)>{acc_localSegPars(*truthMatched).data()}), 
                                 z0, z1, kOrange + 1));

        }
        primitives.push_back(drawLine(fitResult.segmentPars, z0, z1, kRed));
  
        {
            std::stringstream legendLabel{};
            legendLabel<<"Event: "<<ctx.eventID().event_number()<<", chamber : "<<m_idHelperSvc->toStringChamber(bucket->at(0)->identify())
                       <<" #chi^{2} /nDoF: "<<fitResult.chi2/std::max(1, fitResult.nDoF)<<", nDoF: "<<fitResult.nDoF;
            if (!extraLabel.empty()) {
                legendLabel<<" ("<<extraLabel<<")";
            }
            primitives.push_back(drawLabel(legendLabel.str(), 0.1, 0.96));
        }
        primitives.push_back(drawLabel(makeLabel(fitResult.segmentPars),0.25,0.91));

      
        double legX{0.15};
        double legY{0.8};
        for (size_t ihit = 0; ihit < fitResult.calibMeasurements.size(); ++ihit) {
            const HitVec::value_type& hit{fitResult.calibMeasurements[ihit]};
            if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                std::stringstream legendstream{};
                const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
                legendstream<<"ML: "<<idHelper.multilayer(hit->spacePoint()->identify());
                legendstream<<", TL: "<<idHelper.tubeLayer(hit->spacePoint()->identify());
                legendstream<<", T: "<<idHelper.tube(hit->spacePoint()->identify());
                legendstream<<", #chi^{2}: "<<fitResult.chi2PerMeasurement[ihit];
                legendstream<<", "<<(SegmentFitHelpers::driftSign(locPos, locDir, hit->spacePoint(), msgStream()) == -1 ? "L" : "R");
                if (hit->fitState() == CalibratedSpacePoint::State::Valid) {
                    legendstream<<", OnTrk";
                }else if (hit->fitState() == CalibratedSpacePoint::State::Outlier) {
                    legendstream<<", Outlier";
                }
                primitives.push_back(drawLabel(legendstream.str(), legX, legY, 14));
            } else if (hit->type() == xAOD::UncalibMeasType::RpcStripType) {
                std::stringstream legendstream{};
                const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                legendstream<<" DR: "<<idHelper.doubletR(hit->spacePoint()->identify());
                legendstream<<" DZ: "<<idHelper.doubletZ(hit->spacePoint()->identify());
                legendstream<<", GAP: "<<idHelper.gasGap(hit->spacePoint()->identify());
                legendstream<<", #eta/#phi: "<<(hit->measuresEta() ? "si" : "nay") 
                            << "/"<<(hit->measuresPhi() ? "si" : "nay");
                legendstream<<", #chi^{2}: "<<fitResult.chi2PerMeasurement[ihit];
                primitives.push_back(drawLabel(legendstream.str(), legX, legY, 14)); 
            } else if (hit->type() == xAOD::UncalibMeasType::TgcStripType){
               std::stringstream legendstream{};            
               const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
               legendstream<<"ST: "<<m_idHelperSvc ->stationNameString(hit->spacePoint()->identify());            
               legendstream<<", GAP: "<<idHelper.gasGap(hit->spacePoint()->identify());
               legendstream<<", #eta/#phi: "<<(hit->measuresEta() ? "si" : "nay") 
                            << "/"<<(hit->measuresPhi() ? "si" : "nay");
                legendstream<<", #chi^{2}: "<<fitResult.chi2PerMeasurement[ihit];
                primitives.push_back(drawLabel(legendstream.str(), legX, legY, 14)); 
            } else {
                continue;
            }
            legY-=0.05;
        }
        for (auto& drawMe: primitives) {
            drawMe->Draw();
        }
        std::stringstream canvasName;
        canvasName<<"SegmenFitDisplays_"<<ctx.eventID().event_number()<<"_"<<(m_canvCounter++)<<".pdf";
        myCanvas->SaveAs(canvasName.str().c_str());
        myCanvas->SaveAs((m_allCanName+".pdf").c_str());
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
        SpacePointPerLayerSorter hitLayers{*seed.parentBucket()};
        bool hasCandidate{false};
        const auto [locPos, locDir] = beforeRecov.makeLine();
        for (const std::vector<HoughHitType>& mdtLayer : hitLayers.mdtHits()) {
            double bestChi2{std::numeric_limits<double>::max()};
            const SpacePoint* bestHit{nullptr}, *secBestHit{nullptr};
            for (const SpacePoint* mdtHit: mdtLayer) {
                /// Hit is already used in the segment fit
                if (usedSpacePoint.count(mdtHit)){
                    continue;
                }
                const double chi2 = SegmentFitHelpers::chiSqTermMdt(locPos,locDir,mdtHit, msgStream());
                if (chi2 < bestChi2) {
                    secBestHit = bestHit;
                    bestChi2 = chi2;
                    bestHit = mdtHit;
                }
            }
            /** Add the Candidate hits per layer. Best and second best one */
            if (bestHit) {
                candidateHits.push_back(m_calibTool->calibrate(ctx, bestHit, locPos, locDir, beforeRecov.segmentPars[toInt(AxisDefs::time)]));
                const double candChi2 = SegmentFitHelpers::chiSqTermMdt(locPos,locDir, *candidateHits.back(), msgStream());
                ATH_MSG_VERBOSE("Candidate hit for recovery "<<m_idHelperSvc->toString(bestHit->identify())<<", chi2: "<<candChi2);
                if (candChi2 > m_recoveryPull) {
                    candidateHits.back()->setFitState(CalibratedSpacePoint::State::Outlier);
                } else {
                    hasCandidate = true;
                }
            }
            if (secBestHit) {
                candidateHits.push_back(m_calibTool->calibrate(ctx, secBestHit, locPos, locDir, beforeRecov.segmentPars[toInt(AxisDefs::time)]));
                if (SegmentFitHelpers::chiSqTermMdt(locPos,locDir, *candidateHits.back(), msgStream()) > m_recoveryPull) {
                    candidateHits.back()->setFitState(CalibratedSpacePoint::State::Outlier);
                }
            }
        }

        if (!hasCandidate) {
            ATH_MSG_VERBOSE("No space point candidates for recovery were found");
            eraseWrongHits(gctx, beforeRecov);
            return beforeRecov.nDoF > 0;
        }
   
        HitVec copied = copy(beforeRecov.calibMeasurements);
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
            eraseWrongHits(gctx, beforeRecov);
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
                for (unsigned int m = 0; m < beforeRecov.calibMeasurements.size(); ++m) {
                    if (beforeRecov.calibMeasurements[m]->fitState() == CalibratedSpacePoint::State::Outlier && 
                        beforeRecov.chi2PerMeasurement[m] < m_recoveryPull) {
                        copied[m]->setFitState(CalibratedSpacePoint::State::Valid);
                        runAnotherTrial = true;
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
            eraseWrongHits(gctx, beforeRecov);
        }
        return true;
    }
    void SegmentFittingAlg::eraseWrongHits(const ActsGeometryContext& gctx, SegmentFitResult& candidate) const {
        auto [segPos, segDir] = makeLine(candidate.segmentPars);
        candidate.calibMeasurements.erase(std::remove_if(candidate.calibMeasurements.begin(), candidate.calibMeasurements.end(),
                                                [&segPos, &segDir](const HitVec::value_type& hit){
                                                if (hit->fitState() != CalibratedSpacePoint::State::Outlier) {
                                                    return false;
                                                }
                                                /** The segment has never crossed the tube */
                                                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                                                    const double dist = Amg::lineDistance<3>(segPos, segDir, hit->positionInChamber(), hit->directionInChamber());
                                                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit->spacePoint()->primaryMeasurement());
                                                    return dist >= dc->readoutElement()->innerTubeRadius();
                                                }
                                                return true;
                                                }), candidate.calibMeasurements.end());
        const MuonGMR4::MuonChamber* chamber{nullptr};
        for (const auto& hit : candidate.calibMeasurements) {
            if (hit->type() != xAOD::UncalibMeasType::Other){
                chamber = hit->spacePoint()->chamber();
                break;
            }
        }
        std::optional<double> timeOfFlight = candidate.timeFit ? std::make_optional<double>((chamber->localToGlobalTrans(gctx)*segPos).mag() * inv_c) 
                                                                : std::nullopt;
        auto [updatedMeasChi2, updateChi2] = SegmentFitHelpers::postFitChi2PerMas(candidate.segmentPars, timeOfFlight, 
                                                                                  candidate.calibMeasurements, msgStream());
        ATH_MSG_VERBOSE("The measurements before "<<candidate.chi2PerMeasurement<<", after: "<<updatedMeasChi2<<", chi2: "<<updateChi2);
        candidate.chi2PerMeasurement = std::move(updatedMeasChi2);
    }
    void SegmentFittingAlg::resolveAmbiguities(const ActsGeometryContext& gctx,
                                               std::vector<std::unique_ptr<Segment>>& segmentCandidates) const {
        using SegmentVec = std::vector<std::unique_ptr<Segment>>;
        ATH_MSG_VERBOSE("Resolve ambiguities amongst "<<segmentCandidates.size()<<" segment candidates. ");
        std::unordered_map<const MuonGMR4::MuonChamber*, SegmentVec> candidatesPerChamber{};
        
        for (std::unique_ptr<Segment>& sortMe : segmentCandidates) {
            const MuonGMR4::MuonChamber* chamb = sortMe->chamber();
            candidatesPerChamber[chamb].push_back(std::move(sortMe));
        }
        SegmentAmbiSolver ambiSolver{name()};
        segmentCandidates.clear();
        for (auto& [chamber, resolveMe] : candidatesPerChamber) {
            SegmentVec resolvedSegments = ambiSolver.resolveAmbiguity(gctx, std::move(resolveMe));
            segmentCandidates.insert(segmentCandidates.end(), 
                                     std::make_move_iterator(resolvedSegments.begin()),
                                     std::make_move_iterator(resolvedSegments.end()));
        }
    }
}
