/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackVisualizationTool.h"

#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"


#include <filesystem>
#include <format>

#include "TMarker.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TH1F.h"

namespace {
    using namespace MuonR4;
    constexpr int truthColor = kOrange +2;

    constexpr int ColorBarrel = kRed;
    constexpr int ColorEndcapA = kBlue;
    constexpr int ColorEndcapC = kGreen +2;
    constexpr double inM = 1./ Gaudi::Units::m;

    constexpr double extraMargin = 50.*Gaudi::Units::cm * inM;

    static const Muon::MuonSectorMapping sectorMap{};


    std::unique_ptr<TMarker> drawMarker(const Amg::Vector2D& pos, const int mStyle, const int mColor, const int mSize = 2) {
        auto marker = std::make_unique<TMarker>(pos.x(), pos.y(), mStyle);
        marker->SetMarkerColor(mColor);
        marker->SetMarkerSize(mSize);
        return marker;
    }

    inline int stationMarkerSyle(const Muon::MuonStationIndex::ChIndex ch,
                                 const bool openMarker, const bool onSeed) {
        switch (Muon::MuonStationIndex::toLayerIndex(ch)) {
            using enum Muon::MuonStationIndex::LayerIndex;
            case Inner:
            case BarrelExtended:
            case Extended:
                return onSeed ? (openMarker ? kOpenTriangleUp : kFullTriangleUp)
                              : (openMarker ? kOpenTriangleDown : kFullTriangleDown);
            case Middle:
                return onSeed ? (openMarker ? kOpenCrossX : kFullCrossX)
                              : (openMarker ? kOpenCross : kFullCross);
            case Outer:
                return onSeed ? (openMarker ? kOpenFourTrianglesX: kFullFourTrianglesX)
                              : (openMarker ? kOpenThreeTriangles : kFullThreeTriangles);
            default:
                return 0;
        }
    }
    std::string removeNonAlphaNum(std::string str) {
        str.erase(std::remove_if(str.begin(),str.end(),
                  [](const unsigned char c){
                    return !std::isalnum(c);
                   }), str.end());
        return str;
    }
}

namespace MuonValR4{

    void TrackVisualizationTool::PlotLegend::addColor(const int color, const std::string& label){
        if (colors.find(color)!= colors.end()) {
            return;
        }
        auto box = MuonValR4::drawBox(0.,0,1,1, color, MuonValR4::fullFilling);
        legend->AddEntry(box.get(), label.c_str(), "F");
        colors.insert(std::make_pair(color, std::move(box)));
    }
    void TrackVisualizationTool::PlotLegend::addMarker(const int marker, const std::string& label){
        if (markers.find(marker)!= markers.end()) {
            return;
        }
        auto tMarker = drawMarker(Amg::Vector2D::Zero(), marker, kBlack);
        legend->AddEntry(tMarker.get(), label.c_str(), "P");
        markers.insert(std::make_pair(marker, std::move(tMarker)));
    }
    TrackVisualizationTool::BoundBox_t 
        TrackVisualizationTool::emptyBounds() {
        return BoundBox_t{std::array{1.e9,1.e9}, std::array{-1.e9,-1.e9}};
    }
    
    std::mutex TrackVisualizationTool::s_mutex{};

    void TrackVisualizationTool::closeSummaryCanvas() const {
        if (!m_outFile) return;
        ATH_MSG_INFO("Close summary pdf & root file "<<m_allCanName);
        if (m_summaryCan) {
            m_summaryCan->cd();
            m_summaryCan->SaveAs((m_allCanName +".pdf]").c_str());
            m_summaryCan.reset();
        }
        m_outFile.reset();
    }
    Amg::Vector2D TrackVisualizationTool::viewVector(const double phiV,
                                                     const Amg::Vector2D& posOnCylinder,
                                                     const DisplayView view) {
        using enum DisplayView;
        if (view == RZ) { 
            return Amg::Vector2D{ posOnCylinder[1]*inM,  posOnCylinder[0]*inM};
        }
        const CxxUtils::sincos phi{phiV};
        return posOnCylinder[0] * inM * Amg::Vector2D{phi.cs, phi.sn};
    }
    StatusCode TrackVisualizationTool::finalize() {
        closeSummaryCanvas();
        return StatusCode::SUCCESS;
    }
    StatusCode TrackVisualizationTool::initialize() {
        if (m_canvasLimit > 0) {
            if (m_canvasPrefix.value().empty() || m_allCanName.value().empty()) {
                ATH_MSG_FATAL("Please define "<<m_canvasPrefix<<" && "<<m_allCanName);
                return StatusCode::FAILURE;
            }
            if (m_saveSummaryPDF) {
                m_summaryCan = std::make_unique<TCanvas>("all", "all", m_canvasWidth, m_canvasHeight);        
                m_summaryCan->SaveAs((m_allCanName +".pdf[").c_str());
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
        ATH_CHECK(m_truthSegKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_geoCtxKey.initialize());
        return StatusCode::SUCCESS;
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seederObj,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds,
                                              const std::string& extraLabel) const {
        displaySeeds(ctx, seederObj, segments, seeds, extraLabel, PrimitivesVec_t{});
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seederObj,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds,
                                              const std::string& extraLabel,
                                              PrimitivesVec_t && extPrimitives) const {
        if (m_canvCounter >= m_canvasLimit || segments.empty()) {
            return;
        }
        std::lock_guard guard{s_mutex};
        if (m_canvCounter >= m_canvasLimit) {
            return;
        }
        unsigned int nPrim = extPrimitives.size();
        displaySeeds(ctx, seederObj, DisplayView::RZ, segments, seeds, extraLabel, extPrimitives);
        extPrimitives.resize(nPrim);
        displaySeeds(ctx, seederObj, DisplayView::XY, segments, seeds, extraLabel, extPrimitives);
        if ((++m_canvCounter) >= m_canvasLimit) {
            closeSummaryCanvas();
        }
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seeder,
                                              const DisplayView view,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds,
                                              const std::string& extraLabel,
                                              PrimitivesVec_t& extPrimitives) const{

 
        const unsigned nPrim = extPrimitives.size();
        PlotLegend legend{0.005,0.005, 0.6,0.1};
        /** First add the truth points*/
        fillTruthSeedPoints(ctx, seeder, view, legend, extPrimitives);
        /** Then we can add the  */
        auto boundBox = emptyBounds();
              
        auto onSeed = [&seeds](const xAOD::MuonSegment* segment,
                               const Location loc) {
            return std::ranges::any_of(seeds,[segment, loc](const MsTrackSeed& seed){
                return seed.location() == loc && std::ranges::find(seed.segments(), segment) != seed.segments().end();
            });
        };
           
        const ActsTrk::GeometryContext* gctx{nullptr};
        if (!SG::get(gctx, m_geoCtxKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to fetch the geometry context "<<m_geoCtxKey.fullKey());
        }

        for (const xAOD::MuonSegment* segment: segments) {
            using enum Location;
            using enum MsTrackSeeder::SectorProjector;
            using namespace Muon;
            const MuonGMR4::SpectrometerSector* msSector = detailedSegment(*segment)->msSector();
            const auto chIdx = segment->chamberIndex();
            const int mColor = msSector->barrel() ? ColorBarrel : (msSector->side() > 0 ? ColorEndcapA : ColorEndcapC);

            for (const auto secProj : {leftOverlap, center, rightOverlap}) {
                if (!sectorMap.insideSector(segment->sector() + Acts::toUnderlying(secProj),
                                            segment->position().phi())){
                    continue;
                }

                for (const Location loc : {Barrel, Endcap}) {
                    const Amg::Vector2D projPos{seeder.expressOnCylinder(*gctx, *segment, loc, secProj)};
                    if (!seeder.withinBounds(projPos, loc)) {
                        continue;
                    }
                    const bool isGood = onSeed(segment, loc);
                    const int mStyle = stationMarkerSyle(chIdx, false, isGood);
                    const double phi = seeder.projectedPhi(segment->sector(), secProj);
                    const Amg::Vector2D markerPos{viewVector(phi, projPos, view)};
                    extPrimitives.emplace_back(drawMarker(markerPos, mStyle, mColor));

                    if (view == DisplayView::XY) {
                        const double r = markerPos.mag() + extraMargin;
                        boundBox[0].expand(-r, r);
                        boundBox[1].expand(-r, r);
                    } else {
                        boundBox[0].expand(markerPos[0] - extraMargin, markerPos[0] + extraMargin);
                        boundBox[1].expand(markerPos[1] - extraMargin, markerPos[1] + extraMargin);
                    }
                    legend.addMarker(mStyle, std::format("{:}{:}", MuonStationIndex::layerName(MuonStationIndex::toLayerIndex(chIdx)), 
                                                         isGood ? "" : " (discarded)"));
                    legend.addColor(mColor, msSector->barrel() ? "Barrel" : msSector->side() > 0 ? "Endcap A" : "Endcap C");
                }
            }
        }
        for (const MsTrackSeed& seed : seeds) {
            const Amg::Vector3D& pos{seed.position()};
            extPrimitives.emplace_back(drawMarker(viewVector(pos.phi(), Amg::Vector2D{pos.perp(), pos.z()}, view),
                                                  kFullDiamond, kBlack));
            legend.addMarker(kFullDiamond, "track seed");
        }
        /** Draw the sector map */
        for (unsigned int s = 1; s<=16 ; ++s) {
            if (view != DisplayView::XY) {
                break;
            }
            const double phi =  sectorMap.sectorPhi(s);
            const double dPhi = sectorMap.sectorWidth(s);

            const int lStyle = s%2 ? kDashed : kDotted;
            const double r = boundBox[0].max();
            const CxxUtils::sincos phiM{phi-dPhi}, phiP{phi+dPhi};
            const Amg::Vector2D e1{r * phiM.cs, r * phiM.sn};
            const Amg::Vector2D e2{r * phiP.cs, r * phiP.sn};

            auto theLine = std::make_unique<TLine>(0.,0., e1.x(), e1.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin() + nPrim, std::move(theLine));
            theLine = std::make_unique<TLine>(0.,0., e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin() + nPrim, std::move(theLine));
            theLine = std::make_unique<TLine>(e1.x(), e1.y(), e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin() + nPrim, std::move(theLine));
        }
        auto canvas = makeCanvas(ctx, boundBox, view == DisplayView::XY ?  
                                 std::tie("x [m]", "y [m]") : std::tie("z [m]", "R [m]"));

        for (auto& prim: extPrimitives){
           prim->Draw(); 
        }
        legend.legend->SetBorderSize(0);
        legend.legend->SetNColumns(4);
        legend.legend->Draw();
        std::stringstream canvasName{};
        canvasName<<m_canvasPrefix.value()<<"_"<<ctx.eventID().event_number()
                   <<"_SeedDisplay"<<(view == DisplayView::XY ? "XY" : "RZ");
        if (!extraLabel.empty()) canvasName<<"_"<<removeNonAlphaNum(extraLabel);
        ATH_MSG_VERBOSE("Save new plot "<<canvasName.str());
        if (m_saveSinglePDFs) {
            canvas->SaveAs(std::format("Plots/{:}/{:}.pdf", m_canvasPrefix.value(), canvasName.str()).c_str());
        }
        if (m_saveSummaryPDF) {
            canvas->SaveAs((m_allCanName+".pdf").c_str());
        }
        m_outFile->WriteObject(canvas.get(), canvasName.str().c_str());
    }
    void TrackVisualizationTool::fillTruthSeedPoints(const EventContext& ctx,
                                                     const MsTrackSeeder& seeder,
                                                     const DisplayView view,
                                                     PlotLegend& legend, 
                                                     PrimitivesVec_t& drawPrim) const {
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        if (!SG::get(truthSegs,m_truthSegKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to fetch the truth segment container");
        }
        if (!truthSegs) {
            return;
        }
        const ActsTrk::GeometryContext* gctx{nullptr};
        if (!SG::get(gctx, m_geoCtxKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to fetch the geometry context "<<m_geoCtxKey.fullKey());
        }

        bool addedEntry{false};
        for (const xAOD::MuonSegment* segment: *truthSegs) {
            const auto chIdx = segment->chamberIndex();
            const int mStyle = stationMarkerSyle(chIdx, false, true);

            using enum Location;
            using enum MsTrackSeeder::SectorProjector;
            for (const auto secProj : {leftOverlap, center, rightOverlap}) {
                if (!sectorMap.insideSector(segment->sector() + Acts::toUnderlying(secProj),
                                            segment->position().phi())){
                    continue;
                }
                for (const Location loc : {Barrel, Endcap}) {
                    const Amg::Vector2D projected{seeder.expressOnCylinder(*gctx, *segment, loc, secProj)};
                    if (!seeder.withinBounds(projected, loc)) {
                        continue;
                    }
                    const double phi = MsTrackSeeder::projectedPhi(segment->sector(), secProj);
                    drawPrim.push_back(drawMarker(viewVector(phi, projected, view), mStyle, truthColor, 3));
                    legend.addMarker(mStyle, Muon::MuonStationIndex::layerName(Muon::MuonStationIndex::toLayerIndex(chIdx)));
                    addedEntry = true;
                }
            }
        }
        if (addedEntry) {
            legend.addColor(truthColor, "truth");
        }
    }
 
    std::unique_ptr<TCanvas> TrackVisualizationTool::makeCanvas(const EventContext& ctx,
                                                                const BoundBox_t& bounds, 
                                                                const AxisLabels_t& axisLabels) const {
        std::stringstream canvasName{};
        canvasName<<name()<<"_"<<ctx.eventID().event_number()<<"_"<<m_canvCounter;
        ATH_MSG_VERBOSE("Create new canvas "<<canvasName.str());
        auto canvas = std::make_unique<TCanvas>(canvasName.str().c_str(), "all", m_canvasWidth, m_canvasHeight);
        canvas->cd();
        TH1F* frame = canvas->DrawFrame(bounds[0].min(), bounds[1].min(),bounds[0].max(), bounds[1].max());
        frame->GetXaxis()->SetTitle(std::get<0>(axisLabels).c_str());
        frame->GetYaxis()->SetTitle(std::get<1>(axisLabels).c_str());
        return canvas;
    }
}