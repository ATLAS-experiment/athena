/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FastRecoVisualizationTool.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"

#include "Acts/Utilities/Helpers.hpp"

#include "TColor.h"

namespace {
    constexpr int truthColor = kOrange +2;
    using SpacePointSet = std::unordered_set<const MuonR4::SpacePoint*>;

    double inDegrees(double angle) {
        return angle / Gaudi::Units::deg;
    }
}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace SegmentFit;
  
    StatusCode FastRecoVisualizationTool::initialize(){      
        if (m_canvasLimit > 0) {
            m_clientToken.canvasLimit = m_canvasLimit;
            m_clientToken.preFixName = m_canvasPrefix;
            m_clientToken.saveSinglePlots = m_saveSinglePDFs;
            m_clientToken.saveSummaryPlot = m_saveSummaryPDF;
            m_clientToken.subDirectory = m_subDir;
            ATH_CHECK(m_visualSvc.retrieve());
            ATH_CHECK(m_visualSvc->registerClient(m_clientToken));
        } else {
            m_plotsDone = true;
        }
        ATH_CHECK(m_prepContainerKeys.initialize(!m_truthSegLinks.empty()));
        m_truthLinkDecorKeys.clear();
        ATH_MSG_INFO("Hits linked to the following segment decorations are considered as truth");
        for (const std::string& decorName : m_truthSegLinks) {
            ATH_MSG_INFO(" **** "<<decorName);
            if (decorName.empty()) {
                ATH_MSG_FATAL("Decoration must not be empty");
                return StatusCode::FAILURE;
            }
            for (const SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer>& key : m_prepContainerKeys) {
                m_truthLinkDecorKeys.emplace_back(key, decorName);
                m_truthLinkDecors.push_back(SegLinkDecor_t{decorName});
            }
        }
        ATH_CHECK(m_truthLinkDecorKeys.initialize());
        m_displayOnlyTruth.value() &= !m_truthLinkDecorKeys.empty();

        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        if (!m_doEtaBucketViews && !m_doPhiBucketViews && !m_doRZBucketViews) {
            ATH_MSG_ERROR("No bucket view enabled. Please enable at least one of the bucket views to visualize the buckets.");
            return StatusCode::FAILURE;
        }
        if(int(m_doEtaBucketViews) + int(m_doPhiBucketViews) + int(m_doRZBucketViews) > 1) {
            ATH_MSG_ERROR("Multiple bucket views enabled. Please enable only one bucket view.");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    bool FastRecoVisualizationTool::isLabeled(const MuonR4::SpacePoint& hit) const {
        return isLabeled(*hit.primaryMeasurement()) || 
              (hit.secondaryMeasurement() && isLabeled(*hit.secondaryMeasurement()));
    }
    bool FastRecoVisualizationTool::isLabeled(const xAOD::UncalibratedMeasurement& hit) const {
        return std::find_if(m_truthLinkDecors.begin(), m_truthLinkDecors.end(),
                            [&hit](const SegLinkDecor_t& decor){
                                return !decor(hit).empty();
                            }) != m_truthLinkDecors.end();
    }
    FastRecoVisualizationTool::LabeledSegmentSet FastRecoVisualizationTool::getLabeledSegments(const std::vector<const MuonR4::SpacePoint*>& hits) const {
        std::vector<const xAOD::UncalibratedMeasurement*> measurements{};
        measurements.reserve(2* hits.size());
        for (const SpacePoint* hit: hits) {
            measurements.push_back(hit->primaryMeasurement());
            if(hit->secondaryMeasurement()) {
                measurements.push_back(hit->secondaryMeasurement());
            }
        }
        return getLabeledSegments(measurements);
    }
    FastRecoVisualizationTool::LabeledSegmentSet FastRecoVisualizationTool::getLabeledSegments(const std::vector<const xAOD::UncalibratedMeasurement*>& hits) const {
        LabeledSegmentSet truthSegs{};
        for (const xAOD::UncalibratedMeasurement* hit : hits) {
            for (const SegLinkDecor_t& decor: m_truthLinkDecors) {
                for (const SegLink_t& link : decor(*hit)) {
                    truthSegs.insert(*link);
                }
            }
        }
        return truthSegs;
    }
    void FastRecoVisualizationTool::plotPatternBuckets(const EventContext& ctx,
                                                       const std::string& extraLabel,
                                                       PatternHitVisualInfoVec&& patternVisualVec)  const {
        PrimitiveVec primitives{};
        plotPatternBuckets(ctx, extraLabel, std::move(patternVisualVec), std::move(primitives));
    }
    void FastRecoVisualizationTool::plotPatternBuckets(const EventContext& ctx,
                                                       const std::string& extraLabel,
                                                       PatternHitVisualInfoVec&& patternVisualVec,
                                                       PrimitiveVec&& primitives)  const {
        if (patternVisualVec.empty()) {
            ATH_MSG_VERBOSE("No pattern visual info available. Do not create visualization.");
            return;
        }
        /** Loop over the pattern visual objects. We make a plot for every combination of bucket and pattern */
        for (PatternHitVisualInfo& patVisual : patternVisualVec) {
            PrimitiveVec primitivesCopy {clone(primitives)};
            plotPatternBuckets(ctx, extraLabel, std::move(patVisual), std::move(primitivesCopy));
        }
    }
    void FastRecoVisualizationTool::plotPatternBuckets(const EventContext& ctx,
                                                       const std::string& extraLabel,
                                                       PatternHitVisualInfo&& patternVisual) const {
        PrimitiveVec primitives{};
        plotPatternBuckets(ctx, extraLabel, std::move(patternVisual), std::move(primitives));
    }

    void FastRecoVisualizationTool::plotPatternBuckets(const EventContext& ctx,
                                                      const std::string& extraLabel,
                                                      PatternHitVisualInfo&& patternVisual,
                                                      PrimitiveVec&& extraPaints) const {
        for (const MuonR4::SpacePointBucket* bucket : patternVisual.parentBuckets) {
            PatternHitVisualInfo patternVisualCopy {patternVisual};
            PrimitiveVec extraPaintsCopy {clone(extraPaints)};
            plotPatternBucket(ctx, extraLabel, *bucket, std::move(patternVisualCopy), std::move(extraPaintsCopy));
        }
    }
    void FastRecoVisualizationTool::plotPatternBucket(const EventContext& ctx,
                                                      const std::string& extraLabel,
                                                      const MuonR4::SpacePointBucket& bucket,
                                                      PatternHitVisualInfo&& patternVisual,
                                                      PrimitiveVec&& extraPaints) const {
        using PatternStatus = FastRecoVisualizationTool::PatternHitVisualInfo::PatternStatus;
        using HitStatus = FastRecoVisualizationTool::PatternHitVisualInfo::HitStatus;
        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            return;
        }
        /** Check whether to display only patterns with truth information */
        LabeledSegmentSet truthSegs{getLabeledSegments(Acts::unpackConstSmartPointers(bucket))};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        /** Check whether the pattern status is to be displayed */
        PatternStatus patStatus{patternVisual.status};
        if ((patStatus == PatternStatus::eSuccessful && !m_paintSuccessfullPatterns) || 
            (patStatus == PatternStatus::eFailed && !m_paintFailedPatterns) || 
            (patStatus == PatternStatus::eOverlap && !m_paintOverlapPatterns)) {
            return;
        }
        /** Check whether to display only buckets with patterns. Collect pattern hits belonging to the bucket and check the size */
        std::vector<const MuonR4::SpacePoint*> patHitsInBucket{};
        const MuonR4::GlobalPattern& pat {*patternVisual.patternCopy};
        for (const Muon::MuonStationIndex::StIndex& station : pat.getStations()) {
            for (const MuonR4::SpacePoint* hit : pat.hitsInStation(station)) {   
                if (std::ranges::find_if(bucket, 
                        [hit](const std::shared_ptr<MuonR4::SpacePoint> sp) { return sp.get() == hit; }) != bucket.end()) {
                    patHitsInBucket.push_back(hit);      
                }
            }
        }     
        if (patHitsInBucket.empty() && m_displayOnlyWithPattern) {
            return;
        }
        /** Prepare the canvas for drawing */
        std::string nameTag = std::format("{}_{}_Bkt_{:d}", extraLabel, bucket.msSector()->identString(), bucket.bucketId());
        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, nameTag);
        if (!canvas){
            m_plotsDone = true;
            return;
        }
        canvas->add(std::move(extraPaints));
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            throw std::runtime_error("Failed to retrieve GeometryContext from EventStore");
            return;
        }
        /** Determine the view to use ad, in cae=se of Z-R view, check if we have two swap the axis (e.g. endcap) */
        View view{m_doEtaBucketViews ? View::objViewEta : (m_doPhiBucketViews ? View::objViewPhi : View::objViewZR)};
        const Amg::Transform3D& localToGlobalBucket{bucket.msSector()->localToGlobalTransform(*geoCtx)};
        /** Check the orientatation of the local y axis. If is not aligned to the global Z, we will swap the axis for RZ views */
        if (view == View::objViewZR && !bucket.msSector()->barrel()) {
            view = View::objViewRZ;                        
        }
        switch (view) {
            case View::objViewEta:
                canvas->setAxisTitles("y [mm]", "z [mm]");
                break;
            case View::objViewPhi:
                canvas->setAxisTitles("x [mm]", "z [mm]");
                break;
            case View::objViewZR:
                canvas->setAxisTitles("z [mm]", "R [mm]");
                break;
            case View::objViewRZ:
                canvas->setAxisTitles("R [mm]", "z [mm]");
                break;
        }
        /** First draw the pattern hits in this bucket */
        if (!drawHits(bucket, localToGlobalBucket, patHitsInBucket, *canvas, view)) {
            return;
        }

        PrimitiveVec lines{};

        /** Draw the search window */
        drawSearchWindow(localToGlobalBucket, lines, patternVisual.thetaSearchMin, patternVisual.thetaSearchMax, *canvas, view);
            
        /** Print first all the lines */
        for (const auto& sp : bucket) {
            const MuonR4::SpacePoint* testHit {sp.get()};
            /** Check if we have the bucket hit in this pattern visual */
            if (patternVisual.hitLineInfo.find(testHit) == patternVisual.hitLineInfo.end()) {
                continue;
            }
            /** Check if the hit has been replaced or removed */
            const HitStatus hitStatus {std::ranges::find(patternVisual.discardedHits, testHit) != patternVisual.discardedHits.end() ? HitStatus::eDiscarded : 
                                      (std::ranges::find(patternVisual.replacedHits, testHit) != patternVisual.replacedHits.end() ? HitStatus::eReplaced : HitStatus::eKept)};
                
            /** Print pattern and acceptance window lines corresponding to this bucket hit */
            const auto& [lineSlope, deltaRWindow] = patternVisual.hitLineInfo.at(testHit);
            drawLineResidual(*geoCtx, localToGlobalBucket, lines, patternVisual.seed, testHit, lineSlope, deltaRWindow, hitStatus, *canvas, view);
        }

        /** Print pattern information on the canvas */
        double yLegend{0.9};
        auto printOnCanvas = [&lines, &yLegend](const std::string& text){
            lines.push_back(drawLabel(text, 0.2, yLegend, 13));
            yLegend-=0.03;
        };
        
        printOnCanvas(std::format("Seed hit: {}", m_idHelperSvc->toString(patternVisual.seed->identify())));
        printOnCanvas(std::format("Chamber: {}", m_idHelperSvc->toStringChamber(bucket.front()->identify())));
        printOnCanvas(std::format("nPrec: {:d}", pat.nPrecisionHits()));
        printOnCanvas(std::format("nEtaNonPrec: {:d}", pat.nEtaNonPrecisionHits()));
        printOnCanvas(std::format("nPhi: {:d}", pat.nPhiHits()));
        printOnCanvas(std::format("theta: {:.2f}^{{#circ}}", inDegrees(pat.theta())));
        printOnCanvas(std::format("phi: {:.2f}^{{#circ}}", inDegrees(pat.phi())));
        printOnCanvas(std::format("TotalResidual: {:.2f}", pat.totalResidual()));
        printOnCanvas(std::format("Status: {}", patStatus == PatternStatus::eSuccessful ? "Success" : (patStatus == PatternStatus::eFailed ? "Fail" : "Overlap")));
        printOnCanvas(std::format("Sector: {:d}", pat.sector()));
        printOnCanvas(std::format("OverlapSector: {:d}", pat.isSectorOverlap() ? pat.secondarySector() : -1));

        /** Draw truth segments */
        bool drawnTrueLabel{true};
        for (const xAOD::MuonSegment* segment : truthSegs) {
            drawSegment(*segment, localToGlobalBucket, lines, drawnTrueLabel, *canvas, view);
            paintSimHits(*geoCtx, localToGlobalBucket, *segment, lines, view);
        }
        canvas->add(std::move(lines));
            
        std::string legendLabel = std::format("Event: {:}, chamber : {:}, #{:}-view ({:})",
                                                ctx.eventID().event_number(),
                                                bucket.msSector()->identString(),
                                                view ==View::objViewEta ? "eta" : (view ==View::objViewPhi ? "phi" : (view == View::objViewZR ? "ZR" : "RZ")),
                                                nameTag);
        canvas->add(drawLabel(legendLabel, 0.15, 0.96));
    }
    void FastRecoVisualizationTool::drawSearchWindow(const Amg::Transform3D& localToGlobalBucket,
                                                     PrimitiveVec& outputContainer,
                                                     const double thetaMin, 
                                                     const double thetaMax,
                                                     const Canvas_t& canvas,
                                                     const View view) const {
        /** In phi view we dont plot anything */
        if (view == View::objViewPhi) {
            return;
        }
        auto addSearchWindowLine = [&outputContainer, &localToGlobalBucket, &view, &canvas, this](const double globLineTheta){
            constexpr double smallAngle {1e-3};
            if (std::abs(globLineTheta) < smallAngle || std::abs(globLineTheta - M_PI) < smallAngle) {
                /** We do not expect pattern lines horizontal in global Z vs global R plane */
                throw std::runtime_error("Unexpected horizontal pattern search line in global frame. Cannot draw search window.");
                return;
            }
            Amg::Vector3D lineDirection{};
            Amg::Vector3D linePosition{};    
            double lowEnd{};
            double upEnd{};
            if (view == View::objViewZR || view == View::objViewRZ) {
                /** In RZ views, we have already the slope. We draw the line in the z-y plane of the 3D space (convention) */
                lineDirection = Acts::makeDirectionFromAxisTangents(0., tan(globLineTheta));
                linePosition = Amg::Vector3D::Zero();
                lowEnd = view == View::objViewZR ? canvas.corner(Edges::yLow) : canvas.corner(Edges::xLow);
                upEnd = view == View::objViewZR ? canvas.corner(Edges::yHigh) : canvas.corner(Edges::xHigh);
            } else {
                /** In eta views, we need to convert the slope from global to local frame */
                const double globLinePhi {localToGlobalBucket.translation().phi()};
                const Amg::Vector3D globLineDir {Acts::makeDirectionFromPhiTheta(globLinePhi,globLineTheta)};
                lineDirection = localToGlobalBucket.inverse().linear() * globLineDir;
                linePosition = localToGlobalBucket.inverse() * Amg::Vector3D::Zero();
                lowEnd = canvas.corner(Edges::yLow);
                upEnd = canvas.corner(Edges::yHigh);
            }
            outputContainer.emplace_back(drawLine(lineDirection, linePosition, 
                    lowEnd, upEnd, kOrange, kDashed, view));
        };
        addSearchWindowLine(thetaMin);
        addSearchWindowLine(thetaMax);                                        
    }
    void FastRecoVisualizationTool::drawLineResidual(const ActsTrk::GeometryContext& gctx,
                                                     const Amg::Transform3D& localToGlobalBucket,
                                                     PrimitiveVec& outputContainer,
                                                     const MuonR4::SpacePoint* seed, 
                                                     const MuonR4::SpacePoint* testHit,
                                                     const double lineSlope,
                                                     const double Rwindow,
                                                     const PatternHitVisualInfo::HitStatus status,
                                                     const Canvas_t& canvas,
                                                     const View view) const {
        using HitStatus = PatternHitVisualInfo::HitStatus;
        /** In phi view we dont plot anything */
        if (view == View::objViewPhi) {
            return;
        }
        if (std::abs(lineSlope) < std::numeric_limits<double>::epsilon()) {
            /** We do not expect pattern lines horizontal in global Z vs global R plane */
            return;
        }
        auto color {status == HitStatus::eKept ? kGreen : (status == HitStatus::eReplaced ? kPink : kGray)};
        if (view == View::objViewZR || view == View::objViewRZ) {
            /** Draw the pattern line in the z-y plane of the 3D space (convention) */
            const Amg::Vector3D lineDir {Acts::makeDirectionFromAxisTangents(0., lineSlope)};
            const Amg::Vector3D seedGlobalPos {seed->msSector()->localToGlobalTransform(gctx) * seed->localPosition()};
            const Amg::Vector3D linePos {seedGlobalPos.perp()* Amg::Vector3D::UnitY() + seedGlobalPos.z() * Amg::Vector3D::UnitZ()};
            /** Determine the extremes of that line */
            const double seedGlobalR {seedGlobalPos.perp()};
            const double testGlobalR {(localToGlobalBucket * testHit->localPosition()).perp()};
            const double testGlobalZ {(localToGlobalBucket * testHit->localPosition()).z()};
            const double lineIntercept {linePos.y() - lineSlope * linePos.z()};
            const double line2WindowInters {lineSlope*testGlobalZ + lineIntercept};
            double lowLimit{};
            double upLimit{};
            if (seedGlobalR < testGlobalR) {
                lowLimit = (seed->msSector() == testHit->msSector() ? 
                        seedGlobalR : (view == View::objViewZR ? canvas.corner(Edges::yLow) : canvas.corner(Edges::xLow)));
                upLimit = line2WindowInters;
            } else {
                upLimit = (seed->msSector() == testHit->msSector() ? 
                        seedGlobalR : (view == View::objViewZR ? canvas.corner(Edges::yHigh) : canvas.corner(Edges::xHigh)));
                lowLimit = line2WindowInters;
            }
            outputContainer.emplace_back(drawLine(lineDir, linePos, 
                    lowLimit, upLimit, color, kDashed, view));

            /** Draw the acceptance lines in R */
            const Amg::Vector3D barLineDir {Amg::Vector3D::UnitY()};
            const Amg::Vector3D barLinePos {testGlobalZ* Amg::Vector3D::UnitZ()};
            lowLimit = testGlobalR - Rwindow;
            upLimit = testGlobalR + Rwindow;
            outputContainer.emplace_back(drawLine(barLineDir, barLinePos, 
                    lowLimit, upLimit, color, kDotted, view));
        } else {
            /** Draw the pattern line in the local frame of the testHit. Using the chamber phi we ensure the line is always in the y-z plane */
            const double globLineTheta { lineSlope > 0 ? atan(lineSlope) : atan(lineSlope) + M_PI };
            const double globLinePhi {localToGlobalBucket.translation().phi()};
            const Amg::Vector3D globLineDir {Acts::makeDirectionFromPhiTheta(globLinePhi,globLineTheta)};
            const Amg::Vector3D locLineDir {localToGlobalBucket.inverse().linear() * globLineDir};
            /** Retrieve the seed position in the local frame of the bucket we want to draw */
            const Amg::Vector3D seedGlobalPos {seed->msSector()->localToGlobalTransform(gctx) * seed->localPosition()};
            const Amg::Vector3D seedLocalPos {localToGlobalBucket.inverse() * seedGlobalPos};
            /** Determine the extremes of that line */
            const double& testLocalZ {testHit->localPosition().z()};   
            const double& testLocalY {testHit->localPosition().y()};   
            const double testGlobalR {(localToGlobalBucket * testHit->localPosition()).perp()};
            double seedGlobalR {seedGlobalPos.perp()};
            const bool isBarrel {seed->msSector()->barrel()};
            const double line2WindowInters {isBarrel ? Acts::detail::LineHelper::lineIntersect<3>(seedLocalPos, locLineDir, testLocalY* Amg::Vector3D::UnitY(), Amg::Vector3D::UnitZ()).position().z()
                                                     : Acts::detail::LineHelper::lineIntersect<3>(seedLocalPos, locLineDir, testLocalZ* Amg::Vector3D::UnitZ(), Amg::Vector3D::UnitY()).position().z()};
            double lowLimit{};
            double upLimit{};
            if (seedGlobalR < testGlobalR) {
                lowLimit = (seed->msSector() == testHit->msSector() ? seedLocalPos.z() : canvas.corner(Edges::yLow));
                upLimit = line2WindowInters;
            } else {
                upLimit = (seed->msSector() == testHit->msSector() ? seedLocalPos.z() : canvas.corner(Edges::yHigh));
                lowLimit = line2WindowInters;
            }
            outputContainer.emplace_back(drawLine(locLineDir, seedLocalPos, 
                    lowLimit, upLimit, color, kDashed, view));

            /** Draw the acceptance lines in R */
            const Amg::Vector3D barLineDir {isBarrel ? Amg::Vector3D::UnitZ() : Amg::Vector3D::UnitY()};
            const Amg::Vector3D barLinePos {isBarrel ? testLocalY* Amg::Vector3D::UnitY() : testLocalZ* Amg::Vector3D::UnitZ()};
            lowLimit = isBarrel ? testLocalZ - Rwindow : testLocalY - Rwindow;
            upLimit = isBarrel ? testLocalZ + Rwindow : testLocalY + Rwindow;
            outputContainer.emplace_back(drawLine(barLineDir, barLinePos, 
                    lowLimit, upLimit, color, kDotted, view));
        }
    }
    void FastRecoVisualizationTool::drawSegment(const xAOD::MuonSegment& segment,
                                                const Amg::Transform3D& localToGlobalBucket,
                                                PrimitiveVec& outputContainer,
                                                bool& drawnTrueLabel,
                                                const Canvas_t& canvas,
                                                const View view) const {
        if (!m_paintTruthSegment){
            return;
        }
        auto [linePos, lineDir] = makeLine(localSegmentPars(segment));
        double lowEnd{canvas.corner(Edges::yLow)};
        double highEnd{canvas.corner(Edges::yHigh)};
        if (view == View::objViewZR || view == View::objViewRZ){
            /** Compute line and direction in the global frame */
            const Amg::Vector3D globDir {localToGlobalBucket.linear() * lineDir};
            const Amg::Vector3D globPos {localToGlobalBucket * linePos};
            /** Compute line and direction in the globalR-GlobalZ plane */
            const double slopeRZ {(globPos.x()*globDir.x() + globPos.y()*globDir.y()) / (globPos.perp()*globDir.z())};
            lineDir = Acts::makeDirectionFromAxisTangents(0., slopeRZ);
            linePos = globPos.perp()* Amg::Vector3D::UnitY() + globPos.z() * Amg::Vector3D::UnitZ();
            if (view == View::objViewRZ) {
                lowEnd = canvas.corner(Edges::xLow);
                highEnd = canvas.corner(Edges::xHigh);
            }
        }
        outputContainer.emplace_back(drawLine(lineDir, linePos, lowEnd, highEnd,
                                    truthColor, kDotted, view));
        if (!drawnTrueLabel) {
            outputContainer.emplace_back(drawLabel(std::format("true parameters: {:}",makeLabel(localSegmentPars(segment))),0.2, 0.89));
            drawnTrueLabel = true;
        }
    }
    void FastRecoVisualizationTool::paintSimHits(const ActsTrk::GeometryContext& gctx,
                                                 const Amg::Transform3D& localToGlobalBucket,
                                                 const xAOD::MuonSegment& truthSeg,
                                                 PrimitiveVec& outputContainer,
                                                 const View view) const {
        if (!m_paintTruthHits) {
            return;
        }
        auto truthHits = getMatchingSimHits(truthSeg);
        for (const xAOD::MuonSimHit* simHit :  truthHits) {
            const MuonGMR4::MuonReadoutElement* re = m_detMgr->getReadoutElement(simHit->identify());
            const IdentifierHash hash = re->detectorType() == ActsTrk::DetectorType::Mdt ?
                                        re->measurementHash(simHit->identify()) :
                                        re->layerHash(simHit->identify());
            const Amg::Transform3D trf = re->msSector()->globalToLocalTransform(gctx) *
                                         re->localToGlobalTransform(gctx, hash);
            const Amg::Vector3D locPos = trf * xAOD::toEigen(simHit->localPosition());
            const Amg::Vector3D locDir = trf.linear() * xAOD::toEigen(simHit->localDirection());
            if (view == View::objViewEta || view == View::objViewPhi){
                outputContainer.emplace_back(drawArrow(locPos, locDir, truthColor, kDashed, static_cast<int>(view)));
            } else {
                constexpr double arrowLength = 2.*Gaudi::Units::cm; 
                /** Compute line and direction in the global frame */
                const Amg::Vector3D globDir {localToGlobalBucket.linear() * locDir};
                const Amg::Vector3D globPos {localToGlobalBucket * locPos};
                const Amg::Vector3D end = globPos + (arrowLength / std::hypot(globDir.z(),globDir.perp()) ) * globDir;
                auto arrow = view == View::objViewZR ? std::make_unique<TArrow>(globPos.z(), globPos.perp(), end.z(), end.perp(),0.01) :
                                                       std::make_unique<TArrow>(globPos.perp(), globPos.z(), end.perp(), end.z(),0.01);
                arrow->SetLineColor(truthColor);
                arrow->SetLineWidth(2);
                arrow->SetLineStyle(kDashed);
                outputContainer.emplace_back(std::move(arrow));
            }
        }
    }
    template<class SpacePointType>        
        const SpacePoint* 
            FastRecoVisualizationTool::drawHit(const SpacePointType& hit, 
                                               const Amg::Transform3D& localToGlobalBucket,
                                               Canvas_t& canvas,
                                               const View view,
                                               unsigned int fillStyle) const {
        
        /// Don't draw any hit which is not participating in the view
        const bool isEtaView {view == View::objViewEta || view == View::objViewRZ || view == View::objViewZR};
        if ((isEtaView && !hit.measuresEta()) || (view == View::objViewPhi && !hit.measuresPhi())) {
            return nullptr;
        }
        const Amg::Vector3D& localPos {hit.localPosition()};
        const Amg::Vector3D globalPos {localToGlobalBucket * localPos};
        const auto expand = [&canvas, &hit]( double centerX, double centerY) {
            canvas.expandPad(centerX- hit.driftRadius(), centerY - hit.driftRadius());
            canvas.expandPad(centerX+ hit.driftRadius(), centerY + hit.driftRadius());
        };
        if (hit.type() != xAOD::UncalibMeasType::Other) {
            if (view == View::objViewEta || view == View::objViewPhi) {
                expand(localPos[static_cast<int>(view)], localPos.z());
            } else if (view == View::objViewZR) {
                expand(globalPos.z(), globalPos.perp());
            } else if (view == View::objViewRZ) {
                expand(globalPos.perp(), globalPos.z());
            }
        }
        const SpacePoint* underlyingSp{nullptr};
        /// Update the fill style accrodingly
        constexpr int invalidCalibFill = 3305;
        if constexpr (std::is_same_v<SpacePointType, SpacePoint>) {
            underlyingSp = &hit;
            if (hit.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit.primaryMeasurement());
                if (dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                    fillStyle = invalidCalibFill;
                }
            }
        } else if constexpr(std::is_same_v<SpacePointType, CalibratedSpacePoint>) {
            underlyingSp = hit.spacePoint();
            if (hit.fitState() == CalibratedSpacePoint::State::Valid) {
                fillStyle  = fullFilling;
            } else if (hit.fitState() == CalibratedSpacePoint::State::FailedCalib) {
                fillStyle = invalidCalibFill;
            } else  {
                fillStyle = hatchedFilling;
            }
        }
        const Amg::Vector3D posInCanvas = [view, localPos, globalPos]() -> Amg::Vector3D{
            switch (view) {
                case View::objViewEta:
                case View::objViewPhi:
                    return localPos;
                case View::objViewZR:
                    return globalPos.z()* Amg::Vector3D::UnitY() + globalPos.perp() * Amg::Vector3D::UnitZ();
                case View::objViewRZ:
                    return globalPos.z()* Amg::Vector3D::UnitZ() + globalPos.perp() * Amg::Vector3D::UnitY();
            }
            return Amg::Vector3D::Zero();
        }();
        const int covIdx {view == View::objViewPhi ? static_cast<int>(View::objViewPhi) : static_cast<int>(View::objViewEta)};
        switch(hit.type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(underlyingSp->primaryMeasurement());
                canvas.add(drawDriftCircle(posInCanvas, dc->readoutElement()->innerTubeRadius(), 
                                                     kBlack, hollowFilling));

                const int circColor = isLabeled(*dc) ? truthColor : kBlue;                    
                canvas.add(drawDriftCircle(posInCanvas, hit.driftRadius(), circColor, fillStyle));
                break;
            } case xAOD::UncalibMeasType::RpcStripType: {
                const auto* meas{static_cast<const xAOD::RpcMeasurement*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kGreen +2;
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[covIdx]);
                canvas.add(drawBox(posInCanvas, boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   boxColor, fillStyle, covIdx));
                break; 
            } case xAOD::UncalibMeasType::TgcStripType: {
                const auto* meas{static_cast<const xAOD::TgcStrip*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kCyan + 2;
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[covIdx]);
                canvas.add(drawBox(posInCanvas, boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   boxColor, fillStyle, covIdx));
                break; 
            } case xAOD::UncalibMeasType::MMClusterType: {
                const int boxColor = isLabeled(*underlyingSp->primaryMeasurement()) ? truthColor : kAquamarine;
                const double boxWidth = 5*Gaudi::Units::mm;
                canvas.add(drawBox(posInCanvas, boxWidth, 10.*Gaudi::Units::mm, boxColor, fillStyle, covIdx));
                break; 
            }  case xAOD::UncalibMeasType::Other: {
                break;
            }  case xAOD::UncalibMeasType::sTgcStripType: {
                const int boxColor = isLabeled(*underlyingSp->primaryMeasurement()) ? truthColor : kTeal;
                const double boxWidth = 5*Gaudi::Units::mm;
                canvas.add(drawBox(posInCanvas, boxWidth, 10.*Gaudi::Units::mm, boxColor, fillStyle, covIdx));
                break;
            } default: {
                ATH_MSG_WARNING("Please implement proper drawings of the new small wheel.. "<<__FILE__<<":"<<__LINE__);    
                break;
            }
        }
        return underlyingSp;
    }

    template<class SpacePointType>      
        bool FastRecoVisualizationTool::drawHits(const SpacePointBucket& bucket,
                                                 const Amg::Transform3D& localToGlobalBucket,
                                                 const std::vector<SpacePointType>& hitsToDraw,
                                                 Canvas_t& canvas,
                                                 const View view) const {

        SpacePointSet drawnPoints{};
        for (const SpacePointType& hit : hitsToDraw) {            
            drawnPoints.insert(drawHit(*hit, localToGlobalBucket, canvas, view, fullFilling));
        }
        for (const SpacePointBucket::value_type& hit : bucket) {
            // Don't redraw the other points
            if (drawnPoints.count(hit.get())) {
                continue;
            }
            drawHit(*hit, localToGlobalBucket, canvas, view, hollowFilling);
        }
        return drawnPoints.size() - drawnPoints.count(nullptr) > 1;
    }
    std::unique_ptr<TLine> FastRecoVisualizationTool::drawLine(const Amg::Vector3D& lineDirection,
                                                               const Amg::Vector3D& linePoint,
                                                               const double lowEnd, 
                                                               const double highEnd,
                                                               const int color,
                                                               const int lineStyle,
                                                               const View view) const {
        using Parameters = MuonR4::SegmentFit::Parameters;
        using ParamDefs = MuonR4::SpacePoint::SeedingAux::FitParIndex;
        auto makeLine = [](const double x1, const double y1, const double x2, const double y2, const int color, const int style){
            auto line = std::make_unique<TLine>(x1, y1, x2, y2);
            line->SetLineColor(color);
            line->SetLineWidth(2);
            line->SetLineStyle(style);
            return line;
        };
        
        if (view == View::objViewEta || view == View::objViewPhi){
            /** For eta and phi views, we work in the local frame */
            if (std::abs(lineDirection.z()) < std::numeric_limits<double>::epsilon()) {
                /** In the case of a line parallel to the loc Y axis (i.e. horizontal in the Canvas) */
                return makeLine(lowEnd, linePoint.z(), highEnd, linePoint.z(), color, lineStyle);
            } else {
                return MuonValR4::drawLine(linePoint + Amg::intersect<3>(linePoint,lineDirection,Amg::Vector3D::UnitZ(), lowEnd).value_or(0.)* lineDirection,
                                    linePoint + Amg::intersect<3>(linePoint,lineDirection,Amg::Vector3D::UnitZ(), highEnd).value_or(0.)* lineDirection,
                                    color, lineStyle, static_cast<int>(view));
            }
        } else {
            /** For R-Z and Z-R views, we work in the global frame, and the limits are on R. By convention, we use the Z-y plane in 3D space to store the line parameters */
            if (std::abs(lineDirection.y()) < std::numeric_limits<double>::epsilon()) {
                /** Special case: line parallel to global Z axis (constant in global R) */
                if (view == View::objViewZR) {
                    /** Z along the x-axis of the canvas, R along the y-axis */
                    return makeLine(lowEnd, linePoint.y(), highEnd, linePoint.y(), color, lineStyle);
                } else {
                    /** R along the x-axis, Z along the y-axis */
                    return makeLine(linePoint.y(), lowEnd, linePoint.y(), highEnd, color, lineStyle);
                }
            } else if (std::abs(lineDirection.z()) < std::numeric_limits<double>::epsilon()) {
                /** Special case: line parallel to global R axis */
                if (view == View::objViewZR) {
                    /** Z along the x-axis of the canvas, R along the y-axis */
                    return makeLine(linePoint.z(), lowEnd, linePoint.z(), highEnd, color, lineStyle);
                } else {
                    /** R along the x-axis, Z along the y-axis */
                    return makeLine(lowEnd, linePoint.z(), highEnd, linePoint.z(), color, lineStyle);
                }
            } else {
                const double slope {lineDirection.y() / lineDirection.z()};
                const double intercept {linePoint.y() - slope * linePoint.z()};
                const double Zlow {(lowEnd - intercept) / slope};
                const double Zhigh {(highEnd - intercept) / slope};
                if (view == View::objViewZR) {
                    /** Z along the x-axis of the canvas, R along the y-axis */
                    return makeLine(Zlow, lowEnd, Zhigh, highEnd, color, lineStyle);
                } else {
                    /** R along the x-axis, Z along the y-axis */
                    return makeLine(lowEnd, Zlow, highEnd, Zhigh, color, lineStyle);
                }
            }
        }
    }
}
