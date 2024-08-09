/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SegmentFittingAlg.h"


#include <GeoPrimitives/GeoPrimitivesHelpers.h>

#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/CalibSegmentChi2Minimizer.h>
#include <MuonPatternHelpers/SegmentAmbiSolver.h>

#include <xAODMuonPrepData/RpcMeasurement.h>
#include <xAODMuonPrepData/TgcStrip.h>

#include <GaudiKernel/PhysicalConstants.h>
#include <Minuit2/Minuit2Minimizer.h>
#include <Math/Minimizer.h>

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
}

namespace MuonR4{
    using namespace SegmentFit;
    using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
 
    SegmentFittingAlg::~SegmentFittingAlg() = default;
    SegmentFittingAlg::SegmentFittingAlg(const std::string& name, ISvcLocator* pSvcLocator): 
            AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode SegmentFittingAlg::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_seedKey.initialize());
        ATH_CHECK(m_outSegments.initialize());  
        ATH_CHECK(m_calibTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
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

    void SegmentFittingAlg::configureMinimizer(const SegmentFitResult& data,
                                               ROOT::Math::Minimizer& minimizer) const {

        minimizer.SetMaxFunctionCalls(100000);
        minimizer.SetTolerance(0.0001);
        minimizer.SetPrintLevel(-1);
        minimizer.SetStrategy(1);

        minimizer.SetVariable(toInt(AxisDefs::y0), "y0", data.segmentPars[toInt(AxisDefs::y0)], 1.e-5);
        minimizer.SetVariable(toInt(AxisDefs::tanTheta), "tanTheta", data.segmentPars[toInt(AxisDefs::tanTheta)], 1.e-5);
        minimizer.SetVariableLimits(toInt(AxisDefs::y0), 
                                    data.segmentPars[toInt(AxisDefs::y0)] - 60. *Gaudi::Units::cm, 
                                    data.segmentPars[toInt(AxisDefs::y0)] + 60. *Gaudi::Units::cm);
        minimizer.SetVariableLimits(toInt(AxisDefs::tanTheta),
                                    data.segmentPars[toInt(AxisDefs::tanTheta)] - 0.6, 
                                    data.segmentPars[toInt(AxisDefs::tanTheta)] + 0.6);
        
        if (data.hasPhi) {
            minimizer.SetVariable(toInt(AxisDefs::x0), "x0", data.segmentPars[toInt(AxisDefs::x0)], 1.e-5);
            minimizer.SetVariable(toInt(AxisDefs::tanPhi), "tanPhi", data.segmentPars[toInt(AxisDefs::tanPhi)], 1.e-5);
            minimizer.SetVariableLimits(toInt(AxisDefs::x0), 
                                        data.segmentPars[toInt(AxisDefs::x0)] - 600, 
                                        data.segmentPars[toInt(AxisDefs::x0)] + 600);
            minimizer.SetVariableLimits(toInt(AxisDefs::tanPhi), 
                                        data.segmentPars[toInt(AxisDefs::tanPhi)] - 0.6, 
                                        data.segmentPars[toInt(AxisDefs::tanPhi)] + 0.6);
        } else {
            minimizer.SetFixedVariable(toInt(AxisDefs::x0), "x0", data.segmentPars[toInt(AxisDefs::x0)]);
            minimizer.SetFixedVariable(toInt(AxisDefs::tanPhi), "tanPhi", data.segmentPars[toInt(AxisDefs::tanPhi)]);
        }

        if (m_doT0Fit) {
            minimizer.SetVariable(toInt(AxisDefs::time), "t0", data.segmentPars[toInt(AxisDefs::time)], 1.e-5 );
            minimizer.SetVariableLimits(toInt(AxisDefs::time), 
                                        data.segmentPars[toInt(AxisDefs::time)] - 10., 
                                        data.segmentPars[toInt(AxisDefs::time)] + 10.);
        } else{
            minimizer.SetFixedVariable(toInt(AxisDefs::time), "t0", data.segmentPars[toInt(AxisDefs::time)]);
        }
    }
    void SegmentFittingAlg::harvestParameters(const ROOT::Math::Minimizer& minimizer,
                                              CalibSegmentChi2Minimizer& chiSqFunc,
                                              SegmentFitResult& data) const  {
        const double* xs = minimizer.X();
        const double* errs = minimizer.Errors();

        data.segmentPars[toInt(AxisDefs::y0)] = xs[toInt(AxisDefs::y0)];
        data.segmentPars[toInt(AxisDefs::tanTheta)] = xs[toInt(AxisDefs::tanTheta)];
        data.segmentParErrs[toInt(AxisDefs::y0)] = errs[toInt(AxisDefs::y0)];
        data.segmentParErrs[toInt(AxisDefs::tanTheta)] = errs[toInt(AxisDefs::tanTheta)];

        if (data.hasPhi) {
            data.segmentPars[toInt(AxisDefs::x0)] = xs[toInt(AxisDefs::x0)];
            data.segmentPars[toInt(AxisDefs::tanPhi)] = xs[toInt(AxisDefs::tanPhi)];
            data.segmentParErrs[toInt(AxisDefs::x0)] = errs[toInt(AxisDefs::x0)];
            data.segmentParErrs[toInt(AxisDefs::tanPhi)] = errs[toInt(AxisDefs::tanPhi)];
        }
        data.segmentPars[toInt(AxisDefs::time)] = xs[toInt(AxisDefs::time)];
        if (m_doT0Fit) {
            data.segmentParErrs[toInt(AxisDefs::time)] = errs[toInt(AxisDefs::time)];
        }
        data.nIter = minimizer.NCalls();
        const auto [locPos, locDir] = data.makeLine();

        data.calibMeasurements = chiSqFunc.release(xs);

        data.chi2PerMeasurement.clear();
        for (const std::unique_ptr<CalibratedSpacePoint>& meas : data.calibMeasurements) {
            data.chi2PerMeasurement.push_back(SegmentFitHelpers::chiSqTerm(locPos, locDir, std::nullopt, *meas, msgStream()));
        }
        data.chi2 = std::accumulate(data.chi2PerMeasurement.begin(), data.chi2PerMeasurement.end(), 0.);
        data.nDoF = chiSqFunc.nDoF();
    }
    CalibSegmentChi2Minimizer 
           SegmentFittingAlg::defineChi2Functor(const EventContext& ctx,
                                                const ActsGeometryContext& gctx,
                                                const SegmentFitResult::Parameters& initialPars,
                                                const std::vector<HoughHitType>& spacePoints) const {
        
        
        const auto [seedPos, seedDir] = makeLine(initialPars);
        SegmentFitResult::HitVec hits = m_calibTool->calibrate(ctx, spacePoints, seedPos, seedDir, 
                                                               initialPars[toInt(AxisDefs::time)]);

        unsigned int numPhi{0}, numEta{0};
        for (const SegmentFitResult::HitType& hit : hits) {
            numPhi+=hit->spacePoint()->measuresPhi();
            numEta+=hit->spacePoint()->measuresEta();
        }
        /** @brief add beamspot if neccessary */
        if (m_doBeamspotConstraint && numPhi > 1 && numPhi < 3) {
                
            const Amg::Transform3D globToLoc{spacePoints[0]->chamber()->globalToLocalTrans(gctx)};
            Amg::Vector3D beamSpot{globToLoc.translation()};

            AmgSymMatrix(3) covariance{AmgSymMatrix(3)::Identity()}; 
            /// placeholder for a very generous beam spot: 300mm in X,Y (tracking volume), 20000 along Z
            covariance(0,0) = std::pow(m_beamSpotR, 2);
            covariance(1,1) = std::pow(m_beamSpotR, 2);
            covariance(2,2) = std::pow(m_beamSpotL, 2);
            AmgSymMatrix(3) jacobian =  globToLoc.linear();
            covariance = jacobian * covariance * jacobian.transpose(); 
            AmgSymMatrix(2) beamSpotCov = covariance.block<2,2>(0,0);
            auto beamSpotSP = std::make_unique<CalibratedSpacePoint>(nullptr, std::move(beamSpot), Amg::Vector3D::Zero());
            beamSpotSP->setCovariance(std::move(beamSpotCov));
            hits.push_back(std::move(beamSpotSP));
            ++numEta;
            ++numPhi;
        }
        /** Count the degrees of freedom. If there're enough fit the T0 */
        int nDoF = numEta + numPhi - 2 - (numPhi ? 2 : 0);
        return CalibSegmentChi2Minimizer{name(), std::move(hits),
                                         [this, &ctx](const std::vector<const SpacePoint*>& hits,
                                                            const Amg::Vector3D& pos,
                                                            const Amg::Vector3D& dir,
                                                            const double t0){
                                                 return m_calibTool->calibrate(ctx, hits, pos, dir, t0);
                                         }, m_doT0Fit && (nDoF > 0)};

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
        /** Draw the pattern with all possible seeds */
        {
            SegmentFitResult data{};
            data.segmentPars = patternSeed->parameters();
            const Amg::Vector3D seedPos{patternSeed->positionInChamber()};
            const Amg::Vector3D seedDir{patternSeed->directionInChamber()};
            data.segmentPars[toInt(AxisDefs::time)] = (locToGlob*seedPos).mag() * inv_c;

        
            data.calibMeasurements = m_calibTool->calibrate(ctx, patternSeed->getHitsInMax(), seedPos, seedDir, 
                                                            data.segmentPars[toInt(AxisDefs::time)]);

            for (const std::unique_ptr<CalibratedSpacePoint>& meas : data.calibMeasurements) {
                data.chi2PerMeasurement.push_back(SegmentFitHelpers::chiSqTerm(seedPos, seedDir, std::nullopt, *meas, msgStream()));
            }
            data.chi2 = std::accumulate(data.chi2PerMeasurement.begin(), data.chi2PerMeasurement.end(), 0.);
            data.nDoF = 1;

            // /** Draw the full pattern seed */
            std::vector<std::unique_ptr<TObject>> seedLines{};
            MdtSegmentSeedGenerator drawMe{name(), patternSeed, genCfg};
            while(auto s = drawMe.nextSeed()) {
                seedLines.push_back(drawLine(s->parameters()));
            }
            seedLines.push_back(drawLabel("possible seeds: "+std::to_string(drawMe.numGenerated()),0.15, 0.85, 14));
            visualizeFit(ctx, data, patternSeed->parentBucket(), "pattern", std::move(seedLines));

        }
        MdtSegmentSeedGenerator seedGen{name(),patternSeed, std::move(genCfg)};
        while (const std::optional<SegmentSeed> seed = seedGen.nextSeed()) {

            SegmentFitResult data{};
            data.segmentPars = seed->parameters();
            const auto [seedPos, seedDir] = SegmentFit::makeLine(data.segmentPars);
            data.segmentPars[toInt(AxisDefs::time)] = (locToGlob*seedPos).mag() * inv_c;

            /// Define the minimizer function
            CalibSegmentChi2Minimizer c2f = defineChi2Functor(ctx, gctx, data.segmentPars, seed->getHitsInMax());
            /// Check for phi measurements
            data.hasPhi = c2f.hasPhiMeas();
            data.nDoF = c2f.nDoF();
            
            /// Minuit instance
            ROOT::Minuit2::Minuit2Minimizer minimizer((name() + std::to_string(ctx.eventID().event_number())).c_str());
            configureMinimizer(data, minimizer);            
            minimizer.SetFunction(c2f);
 
            
            /// Draw the prefit
            if (m_canvCounter < m_nDrawCanvases) {
                data.calibMeasurements = m_calibTool->calibrate(ctx, seed->getHitsInMax(), seedPos, seedDir, 
                                                                data.segmentPars[toInt(AxisDefs::time)]);
           

                for (const std::unique_ptr<CalibratedSpacePoint>& meas : data.calibMeasurements) {
                    data.chi2PerMeasurement.push_back(SegmentFitHelpers::chiSqTerm(seedPos, seedDir, std::nullopt, *meas, msgStream()));
                }
                data.chi2 = std::accumulate(data.chi2PerMeasurement.begin(), data.chi2PerMeasurement.end(), 0.);

                std::vector<std::unique_ptr<TObject>> primitives{};
                primitives.push_back(drawLine(seed->parameters()));

                visualizeFit(ctx, data, patternSeed->parentBucket(), "prefit", std::move(primitives));
            }
           
            if (!minimizer.Minimize() || !minimizer.Hesse()) {
                 harvestParameters(minimizer, c2f, data);
                 const auto [seedPos, seedDir] = data.makeLine();
                 data.calibMeasurements = m_calibTool->calibrate(ctx, seed->getHitsInMax(), seedPos, seedDir, 
                                                                 data.segmentPars[toInt(AxisDefs::time)]);
            } else {
                harvestParameters(minimizer, c2f, data);
                visualizeFit(ctx, data, patternSeed->parentBucket(), "postfit");     
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
            segments.push_back(std::move(finalSeg));

        }
        return segments;
    }

    bool SegmentFittingAlg::removeOutliers(const EventContext& ctx,
                                           const ActsGeometryContext& gctx,
                                           const SegmentSeed& seed,
                                           SegmentFitResult& data) const {
        
        /** If no degree of freedom is in the segment fit then try to plug the holes  */
        if (data.nDoF<=0) {
            ATH_MSG_VERBOSE("No degree of freedom available. What shall be removed?!");
            return true;
        }

        const auto [segPos, segDir] = data.makeLine();

        if (data.chi2 / data.nDoF < m_outlierRemovalCut) {
            ATH_MSG_VERBOSE("The segment "<<Amg::toString(segPos)<<" + "<<Amg::toString(segDir)
                            <<" is already of good quality "<<data.chi2 / std::max(data.nDoF, 1)
                            <<". Don't remove outliers");
            return true;
        }
        /** Next sort the measurements by chi2 */
        std::sort(data.calibMeasurements.begin(), data.calibMeasurements.end(),
                  [&, this](const HitVec::value_type& a, const HitVec::value_type& b){
                    return SegmentFitHelpers::chiSqTerm(segPos, segDir, std::nullopt, *a, msgStream()) <
                           SegmentFitHelpers::chiSqTerm(segPos, segDir, std::nullopt, *b, msgStream());
                  });
        /** Remove the last measurement as it has the largest discrepancy */
        data.calibMeasurements.pop_back();

        ROOT::Minuit2::Minuit2Minimizer minimizer((name() + "OutlierRemoval" + std::to_string(ctx.eventID().event_number())).c_str());
        configureMinimizer(data, minimizer);

        std::vector<HoughHitType> uncalib{};
        for (const HitVec::value_type& calib : data.calibMeasurements) {
            if (calib->spacePoint()) uncalib.push_back(calib->spacePoint());
        }
        CalibSegmentChi2Minimizer c2f = defineChi2Functor(ctx,gctx, data.segmentPars, std::move(uncalib));
       
        minimizer.SetFunction(c2f);
        if(c2f.nDoF() <= 0 || !minimizer.Minimize() || !minimizer.Hesse()) {
            return false;
        }
        harvestParameters(minimizer, c2f, data);

        visualizeFit(ctx, data, seed.parentBucket(), "bad fit recovery");

        return removeOutliers(ctx, gctx, seed, data);
    }
    void SegmentFittingAlg::visualizeFit(const EventContext& ctx,
                                         const SegmentFitResult& fitResult,
                                         const SpacePointBucket* bucket,
                                         const std::string& extraLabel,
                                         std::vector<std::unique_ptr<TObject>> primitives) const {
        

        static std::mutex mutex{};
        std::unordered_set<const SpacePoint*> usedSpacePoint{};
        for (const HitVec::value_type& hit : fitResult.calibMeasurements) {
            usedSpacePoint.insert(hit->spacePoint());
        }
        const auto [locPos, locDir] = fitResult.makeLine();
        if(m_canvCounter >= m_nDrawCanvases) return;
        std::lock_guard guard{mutex};

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
                    circle->SetFillColor(kBlue); 
                    circle->SetLineColor(kBlue); 
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
        {
            std::stringstream legendLabel{};
            legendLabel<<"Event: "<<ctx.evt()<<", chamber : "<<m_idHelperSvc->toStringChamber(bucket->at(0)->identify())
                       <<" #chi^{2} /nDoF: "<<fitResult.chi2/std::max(1, fitResult.nDoF)<<", nDoF: "<<fitResult.nDoF;
            if (!extraLabel.empty()) {
                legendLabel<<" ("<<extraLabel<<")";
            }
            primitives.push_back(drawLabel(legendLabel.str(), 0.1, 0.96));
        }
        primitives.push_back(drawLabel(makeLabel(fitResult.segmentPars),0.25,0.91));

      
        double legX{0.15};
        double legY{0.8};
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        for (size_t ihit = 0; ihit < fitResult.calibMeasurements.size(); ++ihit) {
            const HitVec::value_type& hit{fitResult.calibMeasurements[ihit]};
            if (hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType) continue;
            
            std::stringstream legendstream{};
            legendstream<<"ML: "<<idHelper.multilayer(hit->spacePoint()->identify());
            legendstream<<", TL: "<<idHelper.tubeLayer(hit->spacePoint()->identify());
            legendstream<<", T: "<<idHelper.tube(hit->spacePoint()->identify());
            legendstream<<", #chi^{2}: "<<fitResult.chi2PerMeasurement[ihit];
            primitives.push_back(drawLabel(legendstream.str(), legX, legY, 14));
            legY-=0.05;
        }

        for (auto& drawMe: primitives) {
            drawMe->Draw();
        }
        std::stringstream canvasName;
        canvasName<<"SegmenFitDisplays_"<<ctx.evt()<<"_"<<(m_canvCounter++)<<".pdf";
        myCanvas->SaveAs(canvasName.str().c_str());
        myCanvas->SaveAs((m_allCanName+".pdf").c_str());
    }

    bool SegmentFittingAlg::plugHoles(const EventContext& ctx,
                                      const ActsGeometryContext& gctx,
                                      const SegmentSeed& seed,
                                      SegmentFitResult& beforeRecov) const {
        /** We've the first estimator of the segment fit */
        const auto [locPos, locDir] = beforeRecov.makeLine();
        ATH_MSG_VERBOSE("plugHoles() -- segment "<<Amg::toString(locPos)<<", "<<Amg::toString(locDir)<<
                        "chi2: "<<beforeRecov.chi2<<", nDoF: "<<beforeRecov.nDoF<<", "
                        <<beforeRecov.chi2 /std::max(beforeRecov.nDoF, 1) );

        /** Setup a map to replace space points if they better suit */
        std::unordered_set<const SpacePoint*> usedSpacePoint{};
        for (const HitVec::value_type& hit : beforeRecov.calibMeasurements) {
            usedSpacePoint.insert(hit->spacePoint());
        }

        std::vector<const SpacePoint*> candidateSPs{};
        candidateSPs.reserve(seed.parentBucket()->size());
        /** Loop over the space points in the bucket and preselect the ones which are compatible 
         *  within the segment parameters */
        for (const SpacePointBucket::value_type& spInBucket : *seed.parentBucket()) {
            if (usedSpacePoint.count(spInBucket.get())){
                continue;
            }
            /// If the Mdt drift circle has an invalid state it may be resurrected via
            /// calibration. If so add the object to the list...
            if (spInBucket->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const auto* prd = static_cast<const xAOD::MdtDriftCircle*>(spInBucket->primaryMeasurement());
                if (prd->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                    const double time = Amg::intersect<3>(spInBucket->positionInChamber(), spInBucket->directionInChamber(), 
                                                          locPos, locDir).value_or(0) * inv_c + beforeRecov.segmentPars[toInt(AxisDefs::time)];
                    const auto calib = m_calibTool->calibrate(ctx, spInBucket.get(), locPos, locDir, time);
                    if (!calib) continue;
                    const double chi2{SegmentFitHelpers::chiSqTermMdt(locPos, locDir, *calib, msgStream())};
                    if (chi2 < m_recoveryPull){
                        candidateSPs.push_back(spInBucket.get());
                    }
                    continue;
                }                
            }
            const double chi2{SegmentFitHelpers::chiSqTerm(locPos,locDir, spInBucket.get(), msgStream())};
            ATH_MSG_VERBOSE("Check "<<m_idHelperSvc->toString(spInBucket->identify())<<" for hit recovery. Chi2: "
                        <<chi2<<", cut: "<<m_recoveryPull);
            if (chi2< m_recoveryPull) {
                candidateSPs.push_back(spInBucket.get());
            }
        }       
        if (candidateSPs.empty()) {
            ATH_MSG_VERBOSE("No space point candidates for recovery were found");
            return beforeRecov.nDoF > 0;
        }

        for (HitVec::value_type& hit : beforeRecov.calibMeasurements) {
            if (hit->spacePoint()) candidateSPs.push_back(hit->spacePoint());
        }

        SegmentFitResult recovered{};
        recovered.segmentPars = beforeRecov.segmentPars;
        
        ROOT::Minuit2::Minuit2Minimizer minimizer((name() + "OutlierRemoval" + std::to_string(ctx.eventID().event_number())).c_str());
        configureMinimizer(recovered, minimizer);

        CalibSegmentChi2Minimizer c2f = defineChi2Functor(ctx, gctx, beforeRecov.segmentPars, candidateSPs);

        minimizer.SetFunction(c2f);
        recovered.nDoF = c2f.nDoF();

        if (recovered.nDoF <= 0 || !minimizer.Minimize() || !minimizer.Hesse()) {
            return false;
        }
        harvestParameters(minimizer,c2f, recovered);
        if (recovered.calibMeasurements.size() <= beforeRecov.calibMeasurements.size()) {
            return true;
        }
        ATH_MSG_VERBOSE("Chi2, nDOF before:"<<beforeRecov.chi2<<", "<<beforeRecov.nDoF
                    <<" after recovery: "<<recovered.chi2<<", "<<recovered.nDoF);
        const double recChi2 = recovered.chi2 / std::max(recovered.nDoF,1);
        /// If the chi2 is less than 5, no outlier rejection is launched. So also accept any recovered segment below
        /// that threshold
        if (recChi2 < m_outlierRemovalCut || (beforeRecov.nDoF == 0) || recChi2 < beforeRecov.chi2 / beforeRecov.nDoF) {
            ATH_MSG_VERBOSE("Accept segment with recovered "<<candidateSPs.size() - beforeRecov.calibMeasurements.size()<<" hits.");
            beforeRecov = std::move(recovered);
            return plugHoles(ctx, gctx, seed, beforeRecov) || true;
        }
        return true;
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
