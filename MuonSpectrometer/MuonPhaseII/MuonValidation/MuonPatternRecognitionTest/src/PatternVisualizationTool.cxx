/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PatternVisualizationTool.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonPatternEvent/Segment.h"
#include "MuonPatternEvent/SegmentSeed.h"

#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"

#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"

#include <format>
#include <sstream>
#include <filesystem>


#include "TH1F.h"
#include "TH2F.h"
#include "TMarker.h"
#include "TROOT.h"
#include "TStyle.h"
namespace {
    std::string removeNonAlphaNum(std::string str) {
        str.erase(std::remove_if(str.begin(),str.end(),
                  [](const unsigned char c){
                    return !std::isalnum(c);
                   }), str.end());
        return str;
    }
    std::vector<const MuonR4::SpacePoint*> stripSmartPtr(const MuonR4::SpacePointBucket& bucket) {
        std::vector<const MuonR4::SpacePoint*> ret{};
        std::transform(bucket.begin(), bucket.end(), std::back_inserter(ret), 
                      [](const MuonR4::SpacePointBucket::value_type& sp){ return sp.get();});
        return ret;
    }
    constexpr int truthColor = kOrange +2;
    constexpr int parLineColor = kRed;
    using SpacePointSet = std::unordered_set<const MuonR4::SpacePoint*>;
    enum Edges {
        yLow = 0, yHigh, zLow, zHigh
    };
}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace SegmentFit;
    using LabeledSegmentSet = PatternVisualizationTool::LabeledSegmentSet;
    std::mutex PatternVisualizationTool::s_mutex{};

    StatusCode PatternVisualizationTool::initialize(){        
        if (m_canvasLimit > 0) {
            if (m_canvasPrefix.value().empty() || m_allCanName.value().empty()) {
                ATH_MSG_FATAL("Please define "<<m_canvasPrefix<<" && "<<m_allCanName);
                return StatusCode::FAILURE;
            }
            if (m_saveSummaryPDF) {
                m_allCan = std::make_unique<TCanvas>("all", "all", m_canvasWidth, m_canvasHeight);        
                m_allCan->SaveAs((m_allCanName +".pdf[").c_str());
            }
            m_outFile = std::make_unique<TFile>( (m_allCanName +".root").c_str(), "RECREATE");
            if (m_saveSinglePDFs) {
                std::filesystem::create_directories("Plots/" + m_canvasPrefix);
            }            
            gROOT->SetStyle("ATLAS");
            TStyle* plotStyle = gROOT->GetStyle("ATLAS");
            plotStyle->SetOptTitle(0);
            plotStyle->SetHistLineWidth(1.);
            plotStyle->SetPalette(kViridis);
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
    StatusCode PatternVisualizationTool::finalize(){
        closeSummaryCanvas();
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
    void PatternVisualizationTool::drawPrimitives(const TCanvas& can, PrimitiveVec& primitives) const {
        const double yLow = can.GetPad(0)->GetUymin();
        const double yHigh = can.GetPad(0)->GetUymax();
        for (auto& prim : primitives) {
            const TObject &primRef = *prim;
            if (typeid(primRef) == typeid(TLine)){
                TLine* line = static_cast<TLine*>(prim.get());
                const Amg::Vector3D linePoint{line->GetX1(), line->GetY1(), 0.};
                const Amg::Vector3D lineDir = Amg::Vector3D{(line->GetX2() - line->GetX1()) / (line->GetY2() - line->GetY1()), 1.,0.}.unit();
                
                const Amg::Vector3D newHigh = linePoint + Amg::intersect<3>(linePoint, lineDir, Amg::Vector3D::UnitY(), yHigh).value_or(0.) * lineDir;
                const Amg::Vector3D newLow = linePoint + Amg::intersect<3>(linePoint, lineDir, Amg::Vector3D::UnitY(), yLow).value_or(0.) * lineDir;
                line->SetX1(newLow.x());
                line->SetY1(newLow.y());
                line->SetX2(newHigh.x());
                line->SetY2(newHigh.y());
            }
            prim->Draw();
        }

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
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        /** Enter the lock phase */
        std::lock_guard guard{s_mutex};
        /** Check again in case multiple threads are simultaneously in the lock phase */
        //cppcheck-suppress identicalConditionAfterEarlyExit
        if (m_canvCounter >= m_canvasLimit) {
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
        accHisto->GetXaxis()->SetTitle(std::format("tan#{}", m_accumlIsEta ? "theta" : "phi" ).c_str());
        accHisto->GetYaxis()->SetTitle( std::string{m_accumlIsEta ? "y_{0}" : "x_{0}"}.c_str());

        std::vector<const SpacePoint*> spacePointsInAcc{};
        for (const std::size_t bin : accumulator.getNonEmptyBins()) {
            const auto [xBin, yBin] = accumulator.axisBins(bin);
            const SpacePointSet& hitsInBin{accumulator.hitIds(xBin, yBin)};
            spacePointsInAcc.insert(spacePointsInAcc.end(),hitsInBin.begin(), hitsInBin.end());
            accHisto->SetBinContent(xBin+1, yBin+1, accumulator.nHits(bin));
        }

        const LabeledSegmentSet truthSegs{getLabeledSegments(spacePointsInAcc)};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        for (const xAOD::MuonSegment* segment : truthSegs) {
            const auto [pos, dir] = makeLine(localSegmentPars(*segment));
            const double tan = m_accumlIsEta ? houghTanTheta(dir) : houghTanPhi(dir);
            const double icept = pos[m_accumlIsEta ? objViewEta : objViewPhi];
            auto truthMarker = std::make_unique<TMarker>(tan, icept, kFullCrossX);
            truthMarker->SetMarkerColor(truthColor);
            truthMarker->SetMarkerSize(8);
            primitives.push_back(std::move(truthMarker));
            primitives.push_back(drawLabel(std::format("true parameters: {:}",makeLabel(localSegmentPars(*segment))),0.2, 0.9));
        }
        for (const auto& maximum : maxima) {
            auto maxMarker = std::make_unique<TMarker>(maximum.x, maximum.y, kFullTriangleUp);
            maxMarker->SetMarkerColor(parLineColor);
            maxMarker->SetMarkerSize(8);
            primitives.push_back(std::move(maxMarker));
        }
        primitives.push_back(drawAtlasLabel(0.65, 0.26, m_AtlasLabel));
        primitives.push_back(drawLumiSqrtS(0.65,0.21, m_sqrtSLabel, m_lumiLabel));
        
        std::stringstream canvasName{};
        canvasName<<name()<<"_"<<ctx.eventID().event_number()<<"_"<<m_canvCounter;
        auto canvas = std::make_unique<TCanvas>(canvasName.str().c_str(), 
                                                 "can", m_canvasWidth, m_canvasHeight);
        canvas->GetPad(0)->SetRightMargin(0.12);
        canvas->GetPad(0)->SetTopMargin(0.12);
        canvas->cd();
        accHisto->Draw("COLZ");
        drawPrimitives(*canvas, primitives);
        primitives.push_back(std::move(accHisto));
        saveCanvas(ctx, spacePointsInAcc.front()->identify(), *canvas, extraLabel);
        primitives.push_back(std::move(canvas));

        primitives.clear();        
        if (m_canvasLimit <= m_canvCounter) {
            closeSummaryCanvas();
        }
    }
    void PatternVisualizationTool::paintSimHits(const EventContext& ctx,
                                                const xAOD::MuonSegment& truthSeg,
                                                PrimitiveVec& primitives,
                                                const int view) const {
        if (!m_paintTruthHits) {
            return;
        }
        auto truthHits = getMatchingSimHits(truthSeg);
        const ActsGeometryContext* geoCtx{nullptr};
        if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
            return;
        }
        for (const xAOD::MuonSimHit* simHit :  truthHits) {
            const MuonGMR4::MuonReadoutElement* re = m_detMgr->getReadoutElement(simHit->identify());
            const IdentifierHash hash = re->detectorType() == ActsTrk::DetectorType::Mdt ?
                                        re->measurementHash(simHit->identify()) :
                                        re->layerHash(simHit->identify());
            const Amg::Transform3D trf = re->msSector()->globalToLocalTrans(*geoCtx) *
                                         re->localToGlobalTrans(*geoCtx, hash);
            const Amg::Vector3D locPos = trf * xAOD::toEigen(simHit->localPosition());
            const Amg::Vector3D locDir = trf.linear() * xAOD::toEigen(simHit->localDirection());
            primitives.push_back(drawArrow(locPos, locDir, truthColor, kDashed, view));
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
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        /** Enter the lock phase */
        std::lock_guard guard{s_mutex};
        /** Check again in case multiple threads are simultaneously in the lock phase */
        //cppcheck-suppress identicalConditionAfterEarlyExit
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }

        const LabeledSegmentSet truthSegs{getLabeledSegments(seed.getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }

        std::array<double, 4> canvasDim{};
        const std::size_t parsedPrimSize{primitives.size()};
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            /** reset the primitives */
            primitives.resize(parsedPrimSize);

            if (!drawHits(*seed.parentBucket(), seed.getHitsInMax(), primitives, canvasDim, view)) {
                continue;
            }
            for (const xAOD::MuonSegment* segment : truthSegs) {
                primitives.push_back(drawLine(localSegmentPars(*segment), canvasDim[Edges::zLow], canvasDim[Edges::zHigh],
                                               truthColor, kDotted, view));
                paintSimHits(ctx,*segment, primitives, view);
            }
            primitives.push_back(drawLine(seed.parameters(), canvasDim[Edges::zLow], canvasDim[Edges::zHigh],
                                         parLineColor, kDashed, view));
        
            writeChi2(seed.parameters(), seed.getHitsInMax(), primitives);
        
            std::stringstream legendLabel{};
            double chi2{0.};
            unsigned nDoF{0};
            legendLabel<<"Event: "<<ctx.eventID().event_number()<<", chamber : "<<m_idHelperSvc->toStringChamber(seed.getHitsInMax().front()->identify())
                       <<std::format(", #chi^{{2}} /nDoF: {:.2f} ({:})", chi2/std::max(1u, nDoF), std::max(1u,nDoF))
                       <<", #"<<(view ==objViewEta ? "eta" : "phi")<<"-view";
            
            if (!extraLabel.empty()) {
                legendLabel<<" ("<<extraLabel<<")";
            }
            primitives.push_back(drawLabel(legendLabel.str(), 0.1, 0.96));
            primitives.push_back(drawLabel(makeLabel(seed.parameters()),0.25, 0.89));
        
            auto canvas = makeCanvas(ctx, canvasDim, view);
            primitives.push_back(drawAtlasLabel(0.75, 0.26, m_AtlasLabel));
            primitives.push_back(drawLumiSqrtS(0.75,0.21, m_sqrtSLabel, m_lumiLabel));

            drawPrimitives(*canvas, primitives);

            saveCanvas(ctx, seed.getHitsInMax().front()->identify(), *canvas, extraLabel);
        }
        if (m_canvasLimit <= m_canvCounter) {
            primitives.clear();
            closeSummaryCanvas();
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
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        /** Enter the lock phase */
        std::lock_guard guard{s_mutex};
        /** Check again in case multiple threads are simultaneously in the lock phase */
        //cppcheck-suppress identicalConditionAfterEarlyExit
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        std::array<double, 4> canvasDim{};        
        LabeledSegmentSet truthSegs{getLabeledSegments(stripSmartPtr(bucket))};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        const std::size_t parsedPrimSize{primitives.size()};
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            /** reset the primitives */
            primitives.resize(parsedPrimSize);
            if (!drawHits(bucket, bucket, primitives, canvasDim, view)) {
                continue;
            }
            bool drawnTrueLabel{false};
            for (const xAOD::MuonSegment* segment : truthSegs) {
                primitives.push_back(drawLine(localSegmentPars(*segment), canvasDim[Edges::zLow], canvasDim[Edges::zHigh],
                                           truthColor, kDotted, view));
                if (!drawnTrueLabel) {
                    primitives.push_back(drawLabel(std::format("true parameters: {:}",makeLabel(localSegmentPars(*segment))),0.2, 0.89));
                    drawnTrueLabel = true;
                }
                paintSimHits(ctx,*segment, primitives, view);
            }
            
            std::stringstream legendLabel{};
            legendLabel<<"Event: "<<ctx.eventID().event_number()
                        <<", chamber : "<<m_idHelperSvc->toStringChamber(bucket.front()->identify())
                        <<",#"<<(view ==objViewEta ? "eta" : "phi")<<"-view";
            if (!extraLabel.empty()) {
                legendLabel<<" ("<<extraLabel<<")";
            }
            primitives.push_back(drawLabel(legendLabel.str(), 0.2, 0.96));

            primitives.push_back(drawAtlasLabel(0.75, 0.26, m_AtlasLabel));
            primitives.push_back(drawLumiSqrtS(0.75,0.21, m_sqrtSLabel, m_lumiLabel));

            auto can = makeCanvas(ctx , canvasDim, view);
            drawPrimitives(*can, primitives);

            saveCanvas(ctx, bucket.front()->identify(), *can, extraLabel);
        }
        if (m_canvasLimit <= m_canvCounter) {
            primitives.clear();
            closeSummaryCanvas();
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
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        /** Enter the lock phase */
        std::lock_guard guard{s_mutex};
        /** Check again in case multiple threads are simultaneously in the lock phase */
        //cppcheck-suppress identicalConditionAfterEarlyExit
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        const LabeledSegmentSet truthSegs{getLabeledSegments(segment.parent()->getHitsInMax())};
        if (truthSegs.empty() && m_displayOnlyTruth) {
            return;
        }
        Parameters segPars{};
        {
            const ActsGeometryContext* geoCtx{nullptr};
            if (!SG::get(geoCtx, m_geoCtxKey, ctx).isSuccess()) {
                return;
            }
            const Amg::Transform3D trf{segment.msSector()->globalToLocalTrans(*geoCtx)};
            const Amg::Vector3D locPos = trf * segment.position();
            const Amg::Vector3D locDir = trf.linear() * segment.direction();
            segPars[toInt(ParamDefs::x0)] = locPos.x();
            segPars[toInt(ParamDefs::y0)] = locPos.y();
            segPars[toInt(ParamDefs::theta)] = locDir.theta();
            segPars[toInt(ParamDefs::phi)] = locDir.phi();
            segPars[toInt(ParamDefs::time)] = segment.segementT0();
        }
        
        std::array<double, 4> canvasDim{};
        const std::size_t parsedPrimSize{primitives.size()};
        for (const int view : {objViewEta, objViewPhi}) {
            if ((view == objViewEta && !m_doEtaBucketViews) ||
                (view == objViewPhi && !m_doPhiBucketViews)){
                continue;
            }
            /** reset the primitives */
            primitives.resize(parsedPrimSize);

            if (!drawHits(*segment.parent()->parentBucket(), segment.measurements(), 
                          primitives, canvasDim, view)) {
                continue;
            }
            for (const xAOD::MuonSegment* segment : truthSegs) {
                primitives.push_back(drawLine(localSegmentPars(*segment), canvasDim[Edges::zLow], canvasDim[Edges::zHigh],
                                           truthColor, kDotted, view));
                paintSimHits(ctx,*segment, primitives, view);
            }
            writeChi2(segPars, segment.measurements(), primitives);

            primitives.push_back(drawAtlasLabel(0.75, 0.26, m_AtlasLabel));
            primitives.push_back(drawLumiSqrtS(0.75,0.21, m_sqrtSLabel, m_lumiLabel));

            primitives.push_back(drawLine(segPars, canvasDim[Edges::zLow], canvasDim[Edges::zHigh],
                                          parLineColor, kDashed, view));


            std::stringstream legendLabel{};
            const Identifier canvasId{segment.parent()->getHitsInMax().front()->identify()};
            legendLabel<<"Event: "<<ctx.eventID().event_number() <<", chamber : "<<m_idHelperSvc->toStringChamber(canvasId)
                        <<std::format(", #chi^{{2}} /nDoF: {:.2f} ({:d})", segment.chi2() /std::max(1u, segment.nDoF()), segment.nDoF())
                        <<", #"<<(view ==objViewEta ? "eta" : "phi")<<"-view";

            if (!extraLabel.empty()) {
                legendLabel<<" ("<<extraLabel<<")";
            }
            primitives.push_back(drawLabel(legendLabel.str(), 0.2, 0.96));
            primitives.push_back(drawLabel(makeLabel(segPars),0.25, 0.91));

            auto canvas = makeCanvas(ctx, canvasDim, view);
            drawPrimitives(*canvas, primitives);

            saveCanvas(ctx, canvasId, *canvas, extraLabel);
        }
        if (m_canvasLimit <= m_canvCounter) {
            primitives.clear();
            closeSummaryCanvas();
        }
    }

    template<class SpacePointType>        
        const SpacePoint* 
            PatternVisualizationTool::drawHit(const SpacePointType& hit, PrimitiveVec& primitives,
                                              std::array<double, 4>& canvasDim, const unsigned int view,
                                              unsigned int fillStyle) const {
        
        static_assert(std::is_same_v<SpacePointType, SpacePoint> ||
                      std::is_same_v<SpacePointType, CalibratedSpacePoint>, "Only usual & calibrated space points are supported");
        /// Don't draw any hit which is not participating in the view
        if ((view == objViewEta && !hit.measuresEta()) || (view == objViewPhi && !hit.measuresPhi())) {
            return nullptr;
        }
        
        if (hit.type() != xAOD::UncalibMeasType::Other) {
            canvasDim[Edges::yLow] = std::min(canvasDim[Edges::yLow], hit.positionInChamber()[view] - hit.driftRadius());
            canvasDim[Edges::yHigh] = std::max(canvasDim[Edges::yHigh], hit.positionInChamber()[view] + hit.driftRadius());
            canvasDim[Edges::zLow] = std::min(canvasDim[Edges::zLow], hit.positionInChamber().z() - hit.driftRadius());
            canvasDim[Edges::zHigh] = std::max(canvasDim[Edges::zHigh], hit.positionInChamber().z() + hit.driftRadius());
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
                primitives.push_back(drawDriftCircle(hit.positionInChamber(), dc->readoutElement()->innerTubeRadius(), 
                                                     kBlack, hollowFilling));

                const int circColor = isLabeled(*dc) ? truthColor : kBlue;                    
                primitives.push_back(drawDriftCircle(hit.positionInChamber(), hit.driftRadius(), circColor, fillStyle));
                break;
            } case xAOD::UncalibMeasType::RpcStripType: {
                const auto* meas{static_cast<const xAOD::RpcMeasurement*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kGreen +2;
                const double boxWidth = 0.5*std::sqrt(12)*underlyingSp->uncertainty()[view];
                primitives.push_back(drawBox(hit.positionInChamber(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                             boxColor, fillStyle));
                break; 
            } case xAOD::UncalibMeasType::TgcStripType: {
                const auto* meas{static_cast<const xAOD::TgcStrip*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kCyan + 2;
                const double boxWidth = 0.5*std::sqrt(12)*underlyingSp->uncertainty()[view];
                primitives.push_back(drawBox(hit.positionInChamber(), boxWidth, 0.5*meas->readoutElement()->gasGapPitch(),
                                             boxColor, fillStyle));
                break; 
            } case xAOD::UncalibMeasType::MMClusterType: {
                const auto* meas{static_cast<const xAOD::MMCluster*>(underlyingSp->primaryMeasurement())};
                const int boxColor = isLabeled(*meas) ? truthColor : kAquamarine;
                const double boxWidth = 0.5*Gaudi::Units::mm;
                primitives.push_back(drawBox(hit.positionInChamber(), boxWidth, 10.*Gaudi::Units::mm,
                                             boxColor, fillStyle));
                break; 
            }  case xAOD::UncalibMeasType::Other :{
                break;
            } default:
                ATH_MSG_WARNING("Please implement proper drawings of the new small wheel.. "<<__FILE__<<":"<<__LINE__);    
                break;
        }
        return underlyingSp;
    }

    template<class SpacePointType>      
        bool PatternVisualizationTool::drawHits(const SpacePointBucket& bucket,
                                                const std::vector<SpacePointType>& hitsToDraw,
                                                std::vector<PrimitivePtr>& primitives,
                                                std::array<double, 4>& canvasDim,
                                                unsigned int view) const {

        canvasDim[Edges::yLow] = canvasDim[Edges::zLow] = 100. *Gaudi::Units::m;
        canvasDim[Edges::yHigh] = canvasDim[Edges::zHigh] = -100. *Gaudi::Units::m;
        
        SpacePointSet drawnPoints{};
        for (const SpacePointType& hit : hitsToDraw) {            
            drawnPoints.insert(drawHit(*hit, primitives, canvasDim, view, fullFilling));
        }
        if (m_displayBucket) {
            for (const SpacePointBucket::value_type& hit : bucket) {
                // Don't redraw the other points
                if (drawnPoints.count(hit.get())) {
                    continue;
                }
                drawHit(*hit, primitives, canvasDim, view, hollowFilling);
            } 
        }

        // adapt the draw range to make sure any detector elements we wish to display 
        // are included 
        for (auto & prim : primitives){
            TBox* theBox = dynamic_cast<TBox*>(prim.get()); 
            if (theBox){
                canvasDim[Edges::zLow] = std::min(canvasDim[Edges::zLow], theBox->GetY1()); 
                canvasDim[Edges::zHigh] = std::max(canvasDim[Edges::zHigh], theBox->GetY2()); 
            }
        }

        double width =  (canvasDim[Edges::yHigh] - canvasDim[Edges::yLow])*m_canvasExtraScale;
        double height = (canvasDim[Edges::zHigh] - canvasDim[Edges::zLow])*m_canvasExtraScale;
        if (height > width) width = height; 
        else height = width;

        const double midPointX = 0.5 * (canvasDim[Edges::yHigh] + canvasDim[Edges::yLow]);
        const double midPointY = 0.5 * (canvasDim[Edges::zHigh] + canvasDim[Edges::zLow]);
        canvasDim[Edges::yLow] = midPointX - 0.5 * width;
        canvasDim[Edges::zLow] = midPointY - 0.5 * height;
        canvasDim[Edges::yHigh] = midPointX + 0.5 * width;
        canvasDim[Edges::zHigh] = midPointY + 0.5 * height;        
        return drawnPoints.size() - drawnPoints.count(nullptr) > 1;
    }
    template<class SpacePointType>
        void PatternVisualizationTool::writeChi2(const MuonR4::SegmentFit::Parameters& pars,
                                                 const std::vector<SpacePointType>& hits,
                                                 PrimitiveVec& primitives,
                                                 const double legX, double startLegY, 
                                                 const double endLegY) const {
        
        const auto [locPos, locDir] = makeLine(pars);
        for (const SpacePointType& hit : hits) { 
            const SpacePoint* underlyingSp{nullptr};
            double chi2{0.};
            bool displayChi2{true};
            if constexpr(std::is_same_v<SpacePointType, Segment::MeasType>) {
                underlyingSp = hit->spacePoint();
                displayChi2 = (hit->fitState() == CalibratedSpacePoint::State::Valid );
                chi2 = SegmentFitHelpers::chiSqTerm(locPos, locDir, pars[toInt(AxisDefs::t0)], 
                                                    std::nullopt, *hit, msgStream());
            } else {
                underlyingSp = hit;
                chi2  = SegmentFitHelpers::chiSqTerm(locPos, locDir, *hit, msgStream());
            }
            
            const Identifier hitId =  underlyingSp ? underlyingSp->identify(): Identifier{};
            std::string legendstream{};
            switch(hit->type()) {
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    const int driftSign{SegmentFitHelpers::driftSign(locPos, locDir, *hit, msgStream())};
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
                legendstream+=std::format(", #chi^{{2}}: {:.2f}", chi2);
            } else {
                legendstream+=", #chi^{2}: ---";
            }
            primitives.push_back(drawLabel(legendstream, legX, startLegY, 14));
            startLegY -= 0.05;
            if (startLegY<= endLegY) {
                break;
            }
        }
    }
    void PatternVisualizationTool::closeSummaryCanvas() const {
        if (!m_outFile) return;
        ATH_MSG_INFO("Close summary pdf & root file "<<m_allCanName);
        if (m_allCan) {
            m_allCan->cd();
            m_allCan->SaveAs((m_allCanName +".pdf]").c_str());
            m_allCan.reset();
        }
        m_outFile.reset();
    }
    std::unique_ptr<TCanvas> PatternVisualizationTool::makeCanvas(const EventContext& ctx,
                                                                  const std::array<double, 4>& canvasDim,
                                                                  const int view) const {
        std::stringstream canvasName{};
        canvasName<<name()<<"_"<<ctx.eventID().event_number()<<"_"<<m_canvCounter;
        ATH_MSG_VERBOSE("Create new canvas "<<canvasName.str()<<" "<<canvasDim);
        auto canvas = std::make_unique<TCanvas>(canvasName.str().c_str(), "all", m_canvasWidth, m_canvasHeight);
        canvas->cd();
        TH1F* frame = canvas->DrawFrame(canvasDim[Edges::yLow],canvasDim[Edges::zLow], canvasDim[Edges::yHigh], canvasDim[Edges::zHigh]);
        frame->GetXaxis()->SetTitle(std::format("{:} [mm]", view == objViewEta ? 'y' : 'x').c_str());
        frame->GetYaxis()->SetTitle("z [mm]");
        return canvas;
    }
    void PatternVisualizationTool::saveCanvas(const EventContext& ctx,
                                              const Identifier& chambId,
                                              TCanvas& canvas,
                                              const std::string& extraLabel) const {
        // If an eta view closes the summary canvas & a phi view is processed
        std::stringstream canvasName{};
        canvasName<<m_canvasPrefix.value()<<"_"<<ctx.eventID().event_number()<<"_"<<(m_canvCounter++)<<"_"
                  <<m_idHelperSvc->stationNameString(chambId)
                  <<std::abs(m_idHelperSvc->stationEta(chambId))
                  <<(m_idHelperSvc->stationEta(chambId) >0 ? "A" : "C")
                  <<m_idHelperSvc->stationPhi(chambId);
        if (!extraLabel.empty()) canvasName<<"_"<<removeNonAlphaNum(extraLabel);
        ATH_MSG_VERBOSE("Save new plot "<<canvasName.str());
        if (m_saveSinglePDFs) {
            canvas.SaveAs(("Plots/" + m_canvasPrefix+"/" + canvasName.str()+".pdf").c_str());
        }
        if (m_saveSummaryPDF) {
            canvas.SaveAs((m_allCanName+".pdf").c_str());
        }
        m_outFile->WriteObject(&canvas, canvasName.str().c_str());
    }
}
