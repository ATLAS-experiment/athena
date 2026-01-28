/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PatternVisualizationTool.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonPatternEvent/Segment.h"
#include "MuonPatternEvent/SegmentSeed.h"

#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"


#include <format>
#include <sstream>
#include <filesystem>


#include "TH1F.h"
#include "TH2F.h"
#include "TMarker.h"
#include "TColor.h"
namespace {
    constexpr int truthColor = kOrange +2;
    constexpr int parLineColor = kRed;
    using SpacePointSet = std::unordered_set<const MuonR4::SpacePoint*>;
}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace SegmentFit;
    using LabeledSegmentSet = PatternVisualizationTool::LabeledSegmentSet;
  
    StatusCode PatternVisualizationTool::initialize(){      
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
        return StatusCode::SUCCESS;
    }
    bool PatternVisualizationTool::isLabeled(const MuonR4::SpacePoint& hit) const {
        return isLabeled(*hit.primaryMeasurement()) || 
              (hit.secondaryMeasurement() && isLabeled(*hit.secondaryMeasurement()));
    }
    bool PatternVisualizationTool::isLabeled(const xAOD::UncalibratedMeasurement& hit) const {
        return std::find_if(m_truthLinkDecors.begin(), m_truthLinkDecors.end(),
                            [&hit](const SegLinkDecor_t& decor){
                                return !decor(hit).empty();
                            }) != m_truthLinkDecors.end();
    }

    LabeledSegmentSet PatternVisualizationTool::getLabeledSegments(const std::vector<const MuonR4::SpacePoint*>& hits) const {
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
    LabeledSegmentSet PatternVisualizationTool::getLabeledSegments(const std::vector<const xAOD::UncalibratedMeasurement*>& hits) const {
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
    // void PatternVisualizationTool::drawPrimitives(const TCanvas& can, PrimitiveVec& primitives) const {
    //     const double yLow = can.GetPad(0)->GetUymin();
    //     const double yHigh = can.GetPad(0)->GetUymax();
    //     for (auto& prim : primitives) {
    //         const TObject &primRef = *prim;
    //         if (typeid(primRef) == typeid(TLine)){
    //             TLine* line = static_cast<TLine*>(prim.get());
    //             const Amg::Vector3D linePoint{line->GetX1(), line->GetY1(), 0.};
    //             const Amg::Vector3D lineDir = Amg::Vector3D{(line->GetX2() - line->GetX1()) / (line->GetY2() - line->GetY1()), 1.,0.}.unit();
                
    //             const Amg::Vector3D newHigh = linePoint + Amg::intersect<3>(linePoint, lineDir, Amg::Vector3D::UnitY(), yHigh).value_or(0.) * lineDir;
    //             const Amg::Vector3D newLow = linePoint + Amg::intersect<3>(linePoint, lineDir, Amg::Vector3D::UnitY(), yLow).value_or(0.) * lineDir;
    //             line->SetX1(newLow.x());
    //             line->SetY1(newLow.y());
    //             line->SetX2(newHigh.x());
    //             line->SetY2(newHigh.y());
    //         }
    //         prim->Draw();
    //     }
    // }
    void PatternVisualizationTool::visualizeAccumulator(const EventContext& ctx,
                                                        const MuonR4::HoughPlane& accumulator,
                                                        const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                                        const MaximumVec& maxima,
                                                        const std::string& extraLabel) const {
        PrimitiveVec primitives{};
        visualizeAccumulator(ctx, accumulator, axisRanges, maxima, extraLabel, std::move(primitives));
    }
    void PatternVisualizationTool::visualizeAccumulator(const EventContext& ctx,
                                                        const MuonR4::HoughPlane& accumulator,
                                                        const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                                        const MaximumVec& maxima,
                                                        const std::string& extraLabel,
                                                        PrimitiveVec&& primitives) const {

        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            return;
        }
        if (accumulator.getNonEmptyBins().empty()) {
            ATH_MSG_WARNING("Hough accumulator is empty");
            return;
        }

        auto accHisto = std::make_unique<TH2F>("AccumulatorHisto", "histo",
                                                accumulator.nBinsX(), axisRanges.xMin, axisRanges.xMax,
                                                accumulator.nBinsY(), axisRanges.yMin, axisRanges.yMax); 
        accHisto->SetDirectory(nullptr);

        std::vector<const SpacePoint*> spacePointsInAcc{};
        for (const std::size_t bin : accumulator.getNonEmptyBins()) {
            const auto [xBin, yBin] = accumulator.axisBins(bin);
            auto hitIds = accumulator.hitIds(xBin, yBin); 
            spacePointsInAcc.insert(spacePointsInAcc.end(),hitIds.begin(), hitIds.end());
            accHisto->SetBinContent(xBin+1, yBin+1, accumulator.nHits(bin));
        }
     
        const LabeledSegmentSet truthSegs{getLabeledSegments(spacePointsInAcc)};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, extraLabel);
        if (!canvas){
            m_plotsDone = true;
            return;
        }
        canvas->expandPad(axisRanges.xMin, axisRanges.yMin);
        canvas->expandPad(axisRanges.xMax, axisRanges.yMax);
        canvas->setAxisTitles(std::format("tan#{}", m_accumlIsEta ? "beta" : "#alpha"),
                              std::format("{:}_{{0}} [mm]", m_accumlIsEta ? "y" : "x"));        
        canvas->add(std::move(accHisto), "HIST SAME");
 
        for (const xAOD::MuonSegment* segment : truthSegs) {
            const auto [pos, dir] = makeLine(localSegmentPars(*segment));
            const double tan = m_accumlIsEta ? houghTanBeta(dir) : houghTanAlpha(dir);
            const double icept = pos[m_accumlIsEta ? objViewEta : objViewPhi];
            auto truthMarker = std::make_unique<TMarker>(tan, icept, kFullCrossX);
            truthMarker->SetMarkerColor(truthColor);
            truthMarker->SetMarkerSize(8);
            canvas->add(std::move(truthMarker));
            canvas->add(drawLabel(std::format("true parameters: {:}",
                                              makeLabel(localSegmentPars(*segment))),0.2, 0.9));
        }
        for (const auto& maximum : maxima) {
            auto maxMarker = std::make_unique<TMarker>(maximum.x, maximum.y, kFullTriangleUp);
            maxMarker->SetMarkerColor(parLineColor);
            maxMarker->SetMarkerSize(8);
            canvas->add(std::move(maxMarker));
        }
        canvas->add(std::move(primitives));
    }
    void PatternVisualizationTool::paintSimHits(const EventContext& ctx,
                                                const xAOD::MuonSegment& truthSeg,
                                                Canvas_t& canvas,
                                                const int view) const {
        if (!m_paintTruthHits) {
            return;
        }
        auto truthHits = getMatchingSimHits(truthSeg);
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }
        for (const xAOD::MuonSimHit* simHit :  truthHits) {
            const MuonGMR4::MuonReadoutElement* re = m_detMgr->getReadoutElement(simHit->identify());
            const IdentifierHash hash = re->detectorType() == ActsTrk::DetectorType::Mdt ?
                                        re->measurementHash(simHit->identify()) :
                                        re->layerHash(simHit->identify());
            const Amg::Transform3D trf = re->msSector()->globalToLocalTransform(*geoCtx) *
                                         re->localToGlobalTransform(*geoCtx, hash);
            const Amg::Vector3D locPos = trf * xAOD::toEigen(simHit->localPosition());
            const Amg::Vector3D locDir = trf.linear() * xAOD::toEigen(simHit->localDirection());
            canvas.add(drawArrow(locPos, locDir, truthColor, kDashed, view));
        }
    }
    void PatternVisualizationTool::visualizeSeed(const EventContext& ctx,
                                                 const MuonR4::SegmentSeed& seed,
                                                 const std::string& extraLabel) const {
        PrimitiveVec primitives{};
        visualizeSeed(ctx, seed, extraLabel, std::move(primitives)); 
    }
    void PatternVisualizationTool::visualizeSeed(const EventContext& ctx,
                                                 const MuonR4::SegmentSeed& seed,
                                                 const std::string& extraLabel,
                                                 PrimitiveVec&& primitives) const {
        
        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            return;
        }
        const LabeledSegmentSet truthSegs{getLabeledSegments(seed.getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }

        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, extraLabel);
        if (!canvas) {
            m_plotsDone = true;
            return;
        }
        canvas->add(std::move(primitives));
       

        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");

            if (!drawHits(*seed.parentBucket(), seed.getHitsInMax(), *canvas, view)) {
                continue;
            }
            for (const xAOD::MuonSegment* segment : truthSegs) {
                canvas->add(drawLine(localSegmentPars(*segment), 
                                              canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh),
                                              truthColor, kDotted, view));
                paintSimHits(ctx,*segment, *canvas, view);
            }
            canvas->add(drawLine(seed.parameters(), canvas->corner(Edges::yLow), 
                                 canvas->corner(Edges::yHigh), parLineColor, kDashed, view));
        
            writeChi2(seed.parameters(), seed.getHitsInMax(), *canvas);
        
            std::string legendLabel = std::format("Event: {:}, chamber : {:}, #{:}-view ({:})",
                                                  ctx.eventID().event_number(),
                                                  m_idHelperSvc->toStringChamber(seed.getHitsInMax().front()->identify()),
                                                  view ==objViewEta ? "eta" : "phi",
                                                  extraLabel);
            canvas->add(drawLabel(legendLabel, 0.1, 0.96));
            canvas->add(drawLabel(makeLabel(seed.parameters()),0.25, 0.89));
        }
    }

    void PatternVisualizationTool::visualizeBucket(const EventContext& ctx,
                                                    const MuonR4::SpacePointBucket& bucket,
                                                    const std::string& extraLabel) const {
        PrimitiveVec primitives{};
        visualizeBucket(ctx, bucket, extraLabel, std::move(primitives));
    }
    void PatternVisualizationTool::visualizeBucket(const EventContext& ctx,
                                                   const MuonR4::SpacePointBucket& bucket,
                                                   const std::string& extraLabel,
                                                   PrimitiveVec&& primitives) const {
        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            return;
        }

        LabeledSegmentSet truthSegs{getLabeledSegments(Acts::unpackConstSmartPointers(bucket))};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
       auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, extraLabel);
        if (!canvas){
            m_plotsDone = true;
            return;
        }
        canvas->add(std::move(primitives));
       
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");
            /** reset the primitives */
            if (!drawHits(bucket, bucket, *canvas, view)) {
                continue;
            }
            bool drawnTrueLabel{false};
            for (const xAOD::MuonSegment* segment : truthSegs) {
                canvas->add(drawLine(localSegmentPars(*segment), 
                                     canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh),
                                     truthColor, kDotted, view));
                if (!drawnTrueLabel) {
                    canvas->add(drawLabel(std::format("true parameters: {:}",makeLabel(localSegmentPars(*segment))),0.2, 0.89));
                    drawnTrueLabel = true;
                }
                paintSimHits(ctx,*segment, *canvas, view);
            }
            
            std::string legendLabel = std::format("Event: {:}, chamber : {:}, #{:}-view ({:})",
                                                  ctx.eventID().event_number(),
                                                  bucket.msSector()->identString(),
                                                  view ==objViewEta ? "eta" : "phi",
                                                  extraLabel);
            canvas->add(drawLabel(legendLabel, 0.15, 0.96));
        }
    }

    void PatternVisualizationTool::visualizeSegment(const EventContext& ctx,
                                                    const MuonR4::Segment& segment,
                                                    const std::string& extraLabel) const {
        PrimitiveVec primitives{};
        visualizeSegment(ctx, segment,extraLabel, std::move(primitives));
    }
            
    void PatternVisualizationTool::visualizeSegment(const EventContext& ctx,
                                                    const MuonR4::Segment& segment,
                                                    const std::string& extraLabel,
                                                    PrimitiveVec&& primitives) const {
        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            return;
        }

        const LabeledSegmentSet truthSegs{getLabeledSegments(segment.parent()->getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }
        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, extraLabel);
        if (!canvas) {
            m_plotsDone = true;
            return;
        }
        canvas->add(std::move(primitives));
        

        const Parameters segPars  = SegmentFit::localSegmentPars(*geoCtx, segment);
        
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");
            if (!drawHits(*segment.parent()->parentBucket(), segment.measurements(), 
                          *canvas, view)) {
                continue;
            }
            for (const xAOD::MuonSegment* segment : truthSegs) {
                canvas->add(drawLine(localSegmentPars(*segment), canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh),
                                           truthColor, kDotted, view));
                paintSimHits(ctx,*segment, *canvas, view);
            }
            writeChi2(segPars, segment.measurements(), *canvas);

            canvas->add(drawLine(segPars, canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh),
                                          parLineColor, kDashed, view));

            const Identifier canvasId{segment.parent()->getHitsInMax().front()->identify()};
            std::string legendLabel=std::format("Event: {:}, chamber: {:}, #chi^{{2}} / nDoF: {:.2f} ({:d}), #{:}-view (:)", 
                                                ctx.eventID().event_number(), m_idHelperSvc->toStringChamber(canvasId),
                                                segment.chi2() /std::max(1u, segment.nDoF()), segment.nDoF(),
                                                view ==objViewEta ? "eta" : "phi", extraLabel);

            canvas->add(drawLabel(legendLabel, 0.2, 0.96));
            canvas->add(drawLabel(makeLabel(segPars),0.25, 0.91));
        }
    }

    void PatternVisualizationTool::visualizeSegmentsMeasurementsObj(const EventContext& ctx, 
                                                                    const MuonR4::Segment& segment,
                                                                    const std::string& extraLabel) const {
                                                                      
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }
        Acts::ObjVisualization3D visualHelper{};
        const MuonR4::Segment::MeasVec& measurements{segment.measurements()};  
        drawObjHits(*geoCtx, measurements, visualHelper);
        const SpacePoint* sp = measurements.front()->type() != xAOD::UncalibMeasType::Other ? measurements.front()->spacePoint() : 
                               measurements[1]->spacePoint();
        auto chamberId = sp->identify();

        //draw also the segment line in the same obj and in the end write the obj file with the visualHelper
        double pathLength{1.*Gaudi::Units::m};      
        Acts::GeometryView3D::drawSegment(visualHelper,
                                         segment.position() + 0.5* pathLength * segment.direction(),
                                         segment.position() - 0.5* pathLength  * segment.direction(),
                                         Acts::s_viewLine);
        std::string fileName = std::format("Event_{:}_chamber_{:}_{:}",  
                           ctx.eventID().event_number(), m_idHelperSvc->toStringChamber(chamberId), extraLabel);
        visualHelper.write(fileName + ".obj");   
                                                                        
    }

    void PatternVisualizationTool::visualizeSegmentsMeasurementsObj(const EventContext& ctx,
                                                                    const MuonR4::Segment::MeasVec& measVec,        
                                                                    const std::string& extraLabel) const {
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }

        Acts::ObjVisualization3D visualHelper{};
        drawObjHits(*geoCtx, measVec, visualHelper);
        const SpacePoint* sp = measVec.front()->type() != xAOD::UncalibMeasType::Other ? measVec.front()->spacePoint() : 
                               measVec[1]->spacePoint();
        auto chamberId = sp->identify();
        std::string fileName = std::format("Event_{:}_chamber_{:}_{:}",  
                           ctx.eventID().event_number(), m_idHelperSvc->toStringChamber(chamberId), extraLabel);
        visualHelper.write(fileName + ".obj");        
    }

    void PatternVisualizationTool::drawObjHits(const ActsTrk::GeometryContext& geoCtx,
                                               const MuonR4::Segment::MeasVec& measVec, 
                                               Acts::ObjVisualization3D& visualHelper) const {


        const SpacePoint* sp = measVec.front()->type() != xAOD::UncalibMeasType::Other ? measVec.front()->spacePoint() : 
                               measVec[1]->spacePoint();
      
        const MuonGMR4::SpectrometerSector* msSector  = sp->msSector();  
        const Amg::Transform3D& locToGlob = msSector->localToGlobalTransform(geoCtx); 
        using CovIdx = SpacePoint::CovIdx;
        double dX{0.}, dY{0.};

        for(const auto& meas : measVec){
          
            //if this is a 2D measurement visualize it as the spacepoint
                      
            if(meas->dimension() == 2){
                //handle straw surfaces
                if(meas->isStraw()){                   
                    const double dR = meas->type() == xAOD::UncalibMeasType::Other ? std::sqrt(meas->covariance()[Acts::toUnderlying(CovIdx::etaCov)]):
                    meas->driftRadius();
                    const double hZ = std::sqrt(meas->covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
                    auto bounds = std::make_unique<Acts::LineBounds>(dR, hZ);
                    Amg::Vector3D globalPos = locToGlob* meas->localPosition();    
                    const Acts::Transform3 trf = Acts::Transform3(
                    Acts::Translation3(globalPos) *locToGlob.rotation());                    
                    auto surface = Acts::Surface::makeShared<Acts::StrawSurface>(trf, std::move(bounds));    
                    Acts::GeometryView3D::drawSurface(visualHelper, *surface, geoCtx.context());    
                    continue;                
                }
                
                dX = std::sqrt(meas->covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
                dY = std::sqrt(meas->covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
                auto bounds = std::make_unique<Acts::RectangleBounds>(dX, dY);               
                Amg::Vector3D globalPos = locToGlob* meas->localPosition();    
                const Acts::Transform3 trf = Acts::Transform3(
                Acts::Translation3(globalPos) *locToGlob.rotation());   
               
                auto surf = Acts::Surface::makeShared<Acts::PlaneSurface>(trf,std::move(bounds));
                Acts::GeometryView3D::drawSurface(visualHelper, *surf, geoCtx.context());          
            }else{
            if(meas->type() == xAOD::UncalibMeasType::Other){
                continue;
            }
            const SpacePoint* underlyingSp{meas->spacePoint()};
            MuonValR4::drawMeasurement(geoCtx, underlyingSp->primaryMeasurement(), visualHelper); 
            } 
        }
    }
   

    template<class SpacePointType>        
        const SpacePoint* 
            PatternVisualizationTool::drawHit(const SpacePointType& hit, Canvas_t& canvas,
                                              const unsigned int view,
                                              unsigned int fillStyle) const {
        
        /// Don't draw any hit which is not participating in the view
        if ((view == objViewEta && !hit.measuresEta()) || (view == objViewPhi && !hit.measuresPhi())) {
            return nullptr;
        }
        
        if (hit.type() != xAOD::UncalibMeasType::Other) {
            canvas.expandPad(hit.localPosition()[view] - hit.driftRadius(), 
                             hit.localPosition().z()   - hit.driftRadius());
            canvas.expandPad(hit.localPosition()[view] + hit.driftRadius(), 
                             hit.localPosition().z()   + hit.driftRadius());
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
        switch(hit.type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(underlyingSp->primaryMeasurement());
                canvas.add(drawDriftCircle(hit.localPosition(), dc->readoutElement()->innerTubeRadius(), 
                                                     kBlack, hollowFilling));

                const int circColor = isLabeled(*dc) ? truthColor : kBlue;                    
                canvas.add(drawDriftCircle(hit.localPosition(), hit.driftRadius(), circColor, fillStyle));
                break;
            } case xAOD::UncalibMeasType::RpcStripType: {
                const auto* meas{static_cast<const xAOD::RpcMeasurement*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kGreen +2;
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[view]);
                canvas.add(drawBox(hit.localPosition(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   boxColor, fillStyle));
                break; 
            } case xAOD::UncalibMeasType::TgcStripType: {
                const auto* meas{static_cast<const xAOD::TgcStrip*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kCyan + 2;
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[view]);
                canvas.add(drawBox(hit.localPosition(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   boxColor, fillStyle));
                break; 
            } case xAOD::UncalibMeasType::MMClusterType: {
                const int boxColor = isLabeled(*underlyingSp->primaryMeasurement()) ? truthColor : kAquamarine;
                const double boxWidth = 5*Gaudi::Units::mm;
                canvas.add(drawBox(hit.localPosition(), boxWidth, 10.*Gaudi::Units::mm, boxColor, fillStyle));
                break; 
            }  case xAOD::UncalibMeasType::Other: {
                break;
            }  case xAOD::UncalibMeasType::sTgcStripType: {
                const int boxColor = isLabeled(*underlyingSp->primaryMeasurement()) ? truthColor : kTeal;
                const double boxWidth = 5*Gaudi::Units::mm;
                canvas.add(drawBox(hit.localPosition(), boxWidth, 10.*Gaudi::Units::mm, boxColor, fillStyle));
                break;
            } default: {
                ATH_MSG_WARNING("Please implement proper drawings of the new small wheel.. "<<__FILE__<<":"<<__LINE__);    
                break;
            }
        }
        return underlyingSp;
    }

    template<class SpacePointType>      
        bool PatternVisualizationTool::drawHits(const SpacePointBucket& bucket,
                                                const std::vector<SpacePointType>& hitsToDraw,
                                                Canvas_t& canvas,
                                                unsigned int view) const {

        SpacePointSet drawnPoints{};
        for (const SpacePointType& hit : hitsToDraw) {            
            drawnPoints.insert(drawHit(*hit, canvas, view, fullFilling));
        }
        if (m_displayBucket) {
            for (const SpacePointBucket::value_type& hit : bucket) {
                // Don't redraw the other points
                if (drawnPoints.count(hit.get())) {
                    continue;
                }
                drawHit(*hit, canvas, view, hollowFilling);
            } 
        }
        return drawnPoints.size() - drawnPoints.count(nullptr) > 1;
    }
    template<class SpacePointType>
        void PatternVisualizationTool::writeChi2(const MuonR4::SegmentFit::Parameters& pars,
                                                 const std::vector<SpacePointType>& hits,
                                                 Canvas_t& canvas,
                                                 const double legX, double startLegY, 
                                                 const double endLegY) const {
        const auto [pos, dir] = makeLine(pars);
     
        for (const SpacePointType& hit : hits) { 
            const SpacePoint* underlyingSp{nullptr};
            bool displayChi2{true};
            if constexpr(std::is_same_v<SpacePointType, Segment::MeasType>) {
                underlyingSp = hit->spacePoint();
                displayChi2 = (hit->fitState() == CalibratedSpacePoint::State::Valid);               
            } else {
                underlyingSp = hit;
            }
            const Identifier hitId =  underlyingSp ? underlyingSp->identify(): Identifier{};
            std::string legendstream{};
            switch(hit->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    const int driftSign{SeedingAux::strawSign(pos, dir, *hit)};
                    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
                    legendstream = std::format("ML: {:1d}, TL: {:1d}, T: {:3d}, {:}",
                                                idHelper.multilayer(hitId), idHelper.tubeLayer(hitId),
                                                idHelper.tube(hitId), driftSign == -1 ? "L" : "R");
                    break;
                } case xAOD::UncalibMeasType::RpcStripType: {
                    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                    legendstream= std::format("DR: {:1d}, DZ: {:1d}, GAP: {:1d}, #eta/#phi: {:}/{:}",
                                              idHelper.doubletR(hitId), idHelper.doubletZ(hitId), idHelper.gasGap(hitId),
                                              hit->measuresEta() ? "si" : "nay", hit->measuresPhi() ? "si" : "nay");
                    break;
                } case xAOD::UncalibMeasType::TgcStripType: {
                    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
                    legendstream = std::format("ST: {:}, GAP: {:1d}, #eta/#phi: {:}/{:}",
                                               m_idHelperSvc->stationNameString(hitId), idHelper.gasGap(hitId),
                                               hit->measuresEta() ? "si" : "nay", hit->measuresPhi() ? "si" : "nay");      
                    break;
                } case xAOD::UncalibMeasType::MMClusterType: {
                    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                    const auto* clus = static_cast<const xAOD::MMCluster*>(underlyingSp->primaryMeasurement());
                    const MuonGMR4::StripDesign& design = clus->readoutElement()->stripLayer(clus->layerHash()).design();
                    legendstream = std::format("ML: {:1d}, GAP: {:1d}, {:}", idHelper.multilayer(hitId), idHelper.gasGap(hitId),
                                                !design.hasStereoAngle() ? "X"  : design.stereoAngle() > 0 ? "U" :"V");
                    break;
                } case xAOD::UncalibMeasType::sTgcStripType: {
                    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                    legendstream = std::format("ML: {:1d}, GAP: {:1d}, #eta/#phi: {:}/{:}", 
                                               idHelper.multilayer(hitId), idHelper.gasGap(hitId),
                                               hit->measuresEta() ? "si" : "nay", hit->measuresPhi() ? "si" : "nay");
                    break;
                }  case xAOD::UncalibMeasType::Other: {
                    legendstream = "Ext. constaint";
                }
                default:
                    break;
            }
            if (displayChi2) {
                const double chi2 = SeedingAux::chi2Term(pos, dir,*hit);                
                legendstream+=std::format(", #chi^{{2}}: {:.2f}", chi2);
            } else {
                legendstream+=", #chi^{2}: ---";
            }
            canvas.add(drawLabel(legendstream, legX, startLegY, 14));
            startLegY -= 0.05;
            if (startLegY<= endLegY) {
                break;
            }
        }
    }
}
