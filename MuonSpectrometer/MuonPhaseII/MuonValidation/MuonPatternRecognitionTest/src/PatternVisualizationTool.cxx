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
#include "xAODMuonPrepData/sTgcMeasurement.h"

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

        ATH_MSG_VERBOSE("Visualize accumulator");

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
            ATH_MSG_VERBOSE("visualizeAccumulator skipped: no truth segments found and displayOnlyTruth=true");
            return;
        }

        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, extraLabel);
        if (!canvas){
            m_plotsDone = true;
            return;
        }


        const double maxBins = std::max(accumulator.nBinsX(), accumulator.nBinsY());
        double markerTextSize = 0.035;
        // Scale down for large accumulators
        markerTextSize *= 25. / std::max(25., maxBins);
        // Avoid absurdly tiny/large text
        markerTextSize = std::clamp(markerTextSize, 0.006, 0.035);
        accHisto->SetMarkerSize(markerTextSize);
        accHisto->SetMarkerColor(kBlack);
        accHisto->SetMinimum(0.);

        canvas->expandPad(axisRanges.xMin, axisRanges.yMin);
        canvas->expandPad(axisRanges.xMax, axisRanges.yMax);
        canvas->setAxisTitles(std::format("tan#{}", m_accumlIsEta ? "beta" : "#alpha"),
                              std::format("{:}_{{0}} [mm]", m_accumlIsEta ? "y" : "x"));        
        canvas->add(std::move(accHisto), "HIST SAME");

        // Draw explicit bin labels at bin centers. This is more robust than TH2 TEXT
        // when plotting into the visualization service's prebuilt frame.
        const double binLabelPx = std::clamp(1.2 * 500. / maxBins, 7.0, 18.0);
        for (const std::size_t bin : accumulator.getNonEmptyBins()) {
            const auto [xBin, yBin] = accumulator.axisBins(bin);
            const double xCenter = axisRanges.xMin + (xBin + 0.5) * (axisRanges.xMax - axisRanges.xMin) / accumulator.nBinsX();
            const double yCenter = axisRanges.yMin + (yBin + 0.5) * (axisRanges.yMax - axisRanges.yMin) / accumulator.nBinsY();

            //Extract number of entries in the bin with no decimal precision just int
            std::string binLabel = std::format("{:0}", int(accumulator.nHits(bin)));

            auto binTxt = drawLabel(binLabel, xCenter, yCenter, binLabelPx, false, kBlack);
            binTxt->SetTextAlign(22);
            canvas->add(std::move(binTxt));
        }
 
        int iseg = 0;
        for (const xAOD::MuonSegment* segment : truthSegs) {
            const auto [pos, dir] = makeLine(localSegmentPars(*segment));
            const double tan = m_accumlIsEta ? houghTanBeta(dir) : houghTanAlpha(dir);
            const double icept = pos[m_accumlIsEta ? objViewEta : objViewPhi];
            auto truthMarker = std::make_unique<TMarker>(tan, icept, kFullCrossX);
            truthMarker->SetMarkerColor(truthColor);
            truthMarker->SetMarkerSize(4);
            canvas->add(std::move(truthMarker));
            canvas->add(drawLabel(std::format("true parameters: {:}",
                                              makeLabel(localSegmentPars(*segment))),0.2, 0.9 - 0.05*iseg));
            iseg += 1;
        }
        for (const auto& maximum : maxima) {
            auto maxMarker = std::make_unique<TMarker>(maximum.x, maximum.y, kFullTriangleUp);
            maxMarker->SetMarkerColor(parLineColor);
            maxMarker->SetMarkerSize(4);
            canvas->add(std::move(maxMarker));

            // Draw a bounding box around the maximum island center (x,y) and widths (wx,wy).
            const double xMin = maximum.x - maximum.wx;
            const double xMax = maximum.x + maximum.wx;
            const double yMin = maximum.y - maximum.wy;
            const double yMax = maximum.y + maximum.wy;

            auto maxBox = drawBox(xMin, yMin, xMax, yMax, parLineColor, hollowFilling);
            maxBox->SetLineStyle(kDashed);
            maxBox->SetLineWidth(2);
            canvas->add(std::move(maxBox));
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
        
        ATH_MSG_VERBOSE("Visualizing seed with " << seed.getHitsInMax().size() << " hits");
        /** Check whether the canvas limit has been reached */
        if (m_plotsDone) {
            ATH_MSG_VERBOSE("visualizeSeed skipped: canvas quota already exhausted for " << extraLabel);
            return;
        }
        const LabeledSegmentSet truthSegs{getLabeledSegments(seed.getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            ATH_MSG_VERBOSE("visualizeSeed skipped: no truth segments found and displayOnlyTruth=true");
            return;
        }

       

        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }

            std::string_view viewName = view == objViewEta ? "eta" : "phi";

            auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, std::format("{}_{}", extraLabel, viewName));

            if (!canvas) {
                m_plotsDone = true;
                return;
            }

            DynamicLegend legend;
            canvas->add(clone(primitives));

            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");

            if (!drawHits(*seed.parentBucket(), seed.getHitsInMax(), *canvas, legend, view)) {
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
            canvas->add(drawLabel( "Seed parameters: " + makeLabel(seed.parameters()),0.25, 0.89));
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
        ATH_MSG_VERBOSE("visualizeBucket called for " << extraLabel);

        LabeledSegmentSet truthSegs{getLabeledSegments(Acts::unpackConstSmartPointers(bucket))};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            ATH_MSG_VERBOSE("visualizeBucket skipped: no truth segments found and displayOnlyTruth=true");
            return;
        }

        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }

            const std::string_view viewName = view == objViewEta ? "eta" : "phi";

            auto canvas = m_visualSvc->prepareCanvas( ctx, m_clientToken, std::format("{}_{}view", extraLabel, viewName));


            if (!canvas) {
                m_plotsDone = true;
                return;
            }

            DynamicLegend legend(0.8, 0.2, 0.95, 0.5);

            canvas->add(clone(primitives));

            /** reset the primitives */
            if (!drawHits(bucket, bucket, *canvas, legend, view)) {
                canvas->trash();
                continue;
            }
            bool drawnTrueLabel{false};
            for (const xAOD::MuonSegment* segment : truthSegs) {

                auto truthLine =  drawLine(localSegmentPars(*segment), canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh), truthColor, kDotted, view);
                TLine* line = static_cast<TLine*>(truthLine.get());
                canvas->add( std::move(truthLine) );
                legend.addOnce( LegendItem::TruthSegmentLine, line, "Truth Segment", "l" );

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

            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");

            if (!legend.empty()) {
                canvas->add(legend.release(), "same");
            }

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

        ATH_MSG_VERBOSE("Visualizing segment with " << segment.measurements().size() << " hits");

        const LabeledSegmentSet truthSegs{getLabeledSegments(segment.parent()->getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        const ActsTrk::GeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }
        

        const Parameters segPars  = SegmentFit::localSegmentPars(*geoCtx, segment);
        
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }

            const std::string_view viewName = view == objViewEta ? "eta" : "phi";

            auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, std::format("{}_{}view", extraLabel, viewName));
            if (!canvas) {
                m_plotsDone = true;
                return;
            }
            canvas->setAxisTitles(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x'), "z [mm]");
            canvas->add(clone(primitives));

            DynamicLegend legend(0.8, 0.2, 0.95, 0.5);


            ATH_MSG_DEBUG("Drawing hits!");
            if (!drawHits(*segment.parent()->parentBucket(), segment.measurements(), 
                          *canvas, legend, view)) {
                canvas->trash();
                continue;
            }

            ATH_MSG_DEBUG("Drawing chi2!");
            writeChi2(segPars, segment.measurements(), *canvas);


            {
                auto segmentline = drawLine(segPars, canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh), parLineColor, kDashed, view);
                TLine *line = static_cast<TLine*>(segmentline.get());
                canvas->add(std::move(segmentline));
                legend.addOnce(LegendItem::RecoSegmentLine, line, "Reco Segment", "l");
            }

            for (const xAOD::MuonSegment* tru_segment : truthSegs) {
                if(!tru_segment) continue;

                auto truthLine =  drawLine(localSegmentPars(*tru_segment), canvas->corner(Edges::yLow), canvas->corner(Edges::yHigh), truthColor, kDotted, view);
                TLine* line = static_cast<TLine*>(truthLine.get());
                canvas->add( std::move(truthLine) );
                legend.addOnce( LegendItem::TruthSegmentLine, line, "Truth Segment", "l" );
                paintSimHits(ctx,*tru_segment, *canvas, view);
            }

            const Identifier canvasId{segment.parent()->getHitsInMax().front()->identify()};
            std::string legendLabel=std::format("Event: {:}, chamber: {:}, #chi^{{2}} / nDoF: {:.2f} ({:d}), #{:}-view (:)", 
                                                ctx.eventID().event_number(), m_idHelperSvc->toStringChamber(canvasId),
                                                segment.chi2() /std::max(1u, segment.nDoF()), segment.nDoF(),
                                                view ==objViewEta ? "eta" : "phi", extraLabel);

            canvas->add(drawLabel(legendLabel, 0.2, 0.96));
            canvas->add(drawLabel(makeLabel(segPars),0.25, 0.91));

            if (!legend.empty()) {
                canvas->add(legend.release(), "same");
            }
        }
    }

    template<class SpacePointType>        
        const SpacePoint* 
            PatternVisualizationTool::drawHit(const SpacePointType& hit, Canvas_t& canvas, DynamicLegend& legend,
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
            ATH_MSG_VERBOSE("drawHit called with SpacePoint");
            underlyingSp = &hit;
            if (hit.type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(hit.primaryMeasurement());
                if (dc && dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                    fillStyle = invalidCalibFill;
                }
            }
        } else if constexpr(std::is_same_v<SpacePointType, CalibratedSpacePoint>) {
            ATH_MSG_VERBOSE("drawHit called with CalibratedSpacePoint");
            underlyingSp = hit.spacePoint();
                if (hit.fitState() == CalibratedSpacePoint::State::Valid) {
                    fillStyle  = fullFilling;
                } else if (hit.fitState() == CalibratedSpacePoint::State::FailedCalib) {
                    fillStyle = invalidCalibFill;
                } else  {
                    fillStyle = hatchedFilling;
                }
        }
        else{
            ATH_MSG_VERBOSE("drawHit called with unsupported SpacePointType");
        }

        //TODO: investigate MuonSegmentFittingAlg.PatternVisualizationTool throws seg fault without check but the calibrated space point should have reference space point no?
        if (!underlyingSp){
            ATH_MSG_VERBOSE("Underlying space point not found, skipping hit");
            return nullptr;
        }

        const auto* primaryMeas   = underlyingSp->primaryMeasurement();
        const auto* secondaryMeas = underlyingSp->secondaryMeasurement();

        //If there is a secondary measurement and it is not matched, choose a different colour to the cases where we have primary and it is matched
        //Need to cast secondary measurement to the respective det type? 
        auto labeledColor = (secondaryMeas && !isLabeled(*secondaryMeas)) ? kOrange - 3 : truthColor;

        const auto covarianceWidth = [&]() -> double {
            const double variance = std::max(0., underlyingSp->covariance()[view]);
            //In case of strip it would be too small to visualize the hit, so we set a minimum width of 5 mm, should we add sqrt(12) to get the full width of the strip?
            return std::max(5. * Gaudi::Units::mm, std::sqrt(variance));
        };

        const auto addLabeledOutline = [&](const double boxWidth, const double boxHeight) {
            if (!isLabeled(*primaryMeas)) {
                return;
            }
            // Draw a dedicated hollow overlay so the truth-color border is always visible.
            auto outline = drawBox(hit.localPosition(), boxWidth, boxHeight, labeledColor, hollowFilling, view);
            outline->SetFillStyle(hollowFilling);
            outline->SetLineColor(labeledColor);
            TObject* p_outline = outline.get();
            canvas.add(std::move(outline), "l");
            legend.addOnce(LegendItem::LabeledBox, p_outline, "TrueSimHitMatch", "f");
        };

        // Add the hit to the legend depending on the fill style (outlier, full, or hatched)
        const auto addHitLegendByFill = [&](TObject* object,
                                            const LegendItem bucketItem,
                                            const char* bucketLabel,
                                            const LegendItem onObjectItem,
                                            const char* onObjectLabel, const char* option = "f") {

            if (fillStyle == hatchedFilling) {
                legend.addOnce(LegendItem::OutlierHit, object, "Outlier", option);
                return;
            }
            if (fillStyle == fullFilling) {
                legend.addOnce(onObjectItem, object, onObjectLabel, option);
                return;
            }

            // Hits in the bucket with a hollow filling
            legend.addOnce(bucketItem, object, bucketLabel, option);
        };

        switch(hit.type()) {
            case xAOD::UncalibMeasType::MdtDriftCircleType: {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(primaryMeas);
                canvas.add(drawDriftCircle(hit.localPosition(), dc->readoutElement()->innerTubeRadius(), kBlack, hollowFilling));
                auto driftCircle = drawDriftCircle(hit.localPosition(), hit.driftRadius(), kBlue, fillStyle);
                TObject* p_driftCircle = driftCircle.get();
                canvas.add(std::move(driftCircle));

                //Unforunately the legend will be a box ... to have a point we could make TGraph with a point and add it to the legend but then we would have to draw the hit as a TGrapg as well, so for now we keep it as a box
                addHitLegendByFill(p_driftCircle,
                                   LegendItem::MdtHit,
                                   "MDT inBucket",
                                   LegendItem::MdtHitOnObject,
                                   "MDT onObject", "f");

                //In case the hit is also a truth hit, change the line colour to indicate that
                if (isLabeled(*dc)) {
                    auto labeledDriftCircle = drawDriftCircle(hit.localPosition(), hit.driftRadius(), labeledColor, hollowFilling);
                    TObject* p_labeledDriftCircle = labeledDriftCircle.get();
                    canvas.add(std::move(labeledDriftCircle));
                    legend.addOnce( LegendItem::LabeledTube, p_labeledDriftCircle, "TrueSimMdtHitMatch", "f");
                }
                break;
            } case xAOD::UncalibMeasType::RpcStripType: {
                const auto* meas{static_cast<const xAOD::RpcMeasurement*>(primaryMeas)};
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[view]);
                auto rpcBox = drawBox(hit.localPosition(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   kGreen + 2, fillStyle);
                TObject* p_rpcBox = rpcBox.get();
                canvas.add(std::move(rpcBox)); 
                addHitLegendByFill(p_rpcBox,
                                   LegendItem::RpcHit,
                                   "RPC inBucket",
                                   LegendItem::RpcHitOnObject,
                                   "RPC onObject");
                addLabeledOutline(boxWidth, 0.5*meas->readoutElement()->gasGapPitch());
                break; 
            } case xAOD::UncalibMeasType::TgcStripType: {
                const auto* meas{static_cast<const xAOD::TgcStrip*>(primaryMeas)};
                const double boxWidth = 0.5*std::sqrt(12)*std::sqrt(underlyingSp->covariance()[view]);
                auto tgcBox = drawBox(hit.localPosition(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                   kCyan + 2, fillStyle);
                TObject* p_tgcBox = tgcBox.get();
                canvas.add(std::move(tgcBox));
                addHitLegendByFill(p_tgcBox,
                                   LegendItem::TgcHit,
                                   "TGC inBucket",
                                   LegendItem::TgcHitOnObject,
                                   "TGC onObject");
                addLabeledOutline(boxWidth, 0.5*meas->readoutElement()->gasGapPitch());
                break; 
            } case xAOD::UncalibMeasType::MMClusterType: {
                const double boxWidth = covarianceWidth();
                constexpr double boxHeight = 10.*Gaudi::Units::mm;
                auto mmBox = drawBox(hit.localPosition(), boxWidth, boxHeight, kAquamarine, fillStyle, view);
                TObject* p_mmBox = mmBox.get();
                canvas.add(std::move(mmBox));

                addHitLegendByFill(p_mmBox,
                                   LegendItem::MmHit,
                                   "MMG inBucket",
                                   LegendItem::MmHitOnObject,
                                   "MMG onObject");
                addLabeledOutline(boxWidth, boxHeight);
                break; 
            }  case xAOD::UncalibMeasType::Other: {
                break;
            }  case xAOD::UncalibMeasType::sTgcStripType: {
                const double boxWidth = covarianceWidth();
                const double boxHeight = 10.*Gaudi::Units::mm;
                auto stgcBox = drawBox(hit.localPosition(), boxWidth, boxHeight, kTeal, fillStyle, view);


                //In case of pad use a transparent filling to see the underlying strip or wire
                //TODO: do more splitting into strip+wire, strip+pad, wire+pad, strip+wire+pad and add legend accordingly?
                LegendItem legendType = LegendItem::StgcStripWireHit;
                const auto* primary = static_cast<const xAOD::sTgcMeasurement*>( primaryMeas );
                if (primary && primary->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Pad) {
                    stgcBox->SetFillColorAlpha(kTeal, 0.1);
                    legendType = LegendItem::StgcPadHit;
                }
                TObject* p_stgcBox = stgcBox.get();

                canvas.add(std::move(stgcBox));
                if (legendType == LegendItem::StgcPadHit) {
                    addHitLegendByFill(p_stgcBox,
                                       LegendItem::StgcPadHit,
                                       "sTGCp inBucket",
                                       LegendItem::StgcPadHitOnObject,
                                       "sTGCp onObject");
                } else {
                    addHitLegendByFill(p_stgcBox,
                                       LegendItem::StgcStripWireHit,
                                       "sTGCs+w inBucket",
                                       LegendItem::StgcStripWireHitOnObject,
                                       "sTGCs+w onObject");
                }
                addLabeledOutline(boxWidth, boxHeight);
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
                                                Canvas_t& canvas, DynamicLegend& legend,
                                                unsigned int view) const {

        SpacePointSet drawnPoints{};

        for (const SpacePointType& hit : hitsToDraw) {            
            drawnPoints.insert(drawHit(*hit, canvas, legend, view, fullFilling));
        }
        if (m_displayBucket) {
            for (const SpacePointBucket::value_type& hit : bucket) {
                // Don't redraw the other points
                if (drawnPoints.count(hit.get())) {
                    continue;
                }

                drawHit(*hit, canvas, legend, view, hollowFilling);
            } 
        }
        return drawnPoints.size() - drawnPoints.count(nullptr) > 0;
    }
    template<class SpacePointType>
        void PatternVisualizationTool::writeChi2(const MuonR4::SegmentFit::Parameters& pars,
                                                 const std::vector<SpacePointType>& hits,
                                                 Canvas_t& canvas,
                                                 const double legX, double startLegY, 
                                                 const double endLegY) const {
        const auto [pos, dir] = makeLine(pars);
     
        unsigned int nHits = 0;
        for (const SpacePointType& hit : hits) { 
            const SpacePoint* underlyingSp{nullptr};
            bool displayChi2{true};
            if constexpr(std::is_same_v<SpacePointType, Segment::MeasType>) {
                underlyingSp = hit->spacePoint();
                displayChi2 = (hit->fitState() == CalibratedSpacePoint::State::Valid);               
            } else {
                underlyingSp = hit;
            }

            ATH_MSG_VERBOSE("Writing chi2 for hit " << nHits << " of type " << hit->type());

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
                //Make thi chi2 view specific?
                const double chi2 = SeedingAux::chi2Term(pos, dir,*hit);                
                legendstream+=std::format(", #chi^{{2}}: {:.2f}", chi2);
            } else {
                legendstream+=", NotValidState";
            }


            ++nHits;

            canvas.add(drawLabel(legendstream, legX, startLegY, 8));
            startLegY -= 0.02;
            if (startLegY<= endLegY) {
                break;
            }
        }
    }

}
