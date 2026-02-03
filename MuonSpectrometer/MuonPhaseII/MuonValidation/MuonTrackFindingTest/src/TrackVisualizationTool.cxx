/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TrackVisualizationTool.h"

#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/ObjVisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/FileHelpers.h"


#include <filesystem>
#include <format>

#include "TMarker.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TH1F.h"

namespace {
    using namespace MuonR4;
    using PrimitivesVec_t = MuonValR4::TrackVisualizationTool::PrimitivesVec_t;
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
}

namespace MuonValR4{

    void TrackVisualizationTool::PlotLegend::addColor(const int color, const std::string& label){
        if (colors.find(color) != colors.end()) {
            return;
        }
        auto box = MuonValR4::drawBox(-1.5,-1.5,-1.,-1., color, MuonValR4::fullFilling);
        legend->AddEntry(box.get(), label.c_str(), "F");
        colors.insert(std::make_pair(color, std::move(box)));
    }
    void TrackVisualizationTool::PlotLegend::addMarker(const int marker, const std::string& label){
        if (markers.find(marker)!= markers.end()) {
            return;
        }
        auto tMarker = drawMarker(Amg::Vector2D{2.*Gaudi::Units::km, 0.}, marker, kBlack);
        legend->AddEntry(tMarker.get(), label.c_str(), "P");
        markers.insert(std::make_pair(marker, std::move(tMarker)));
    }
    void TrackVisualizationTool::PlotLegend::fillPrimitives(Canvas_t& canvas) {
        legend->SetBorderSize(0);
        legend->SetNColumns(4);
        canvas.add(std::move(legend));
        for (auto&[i, obj] : colors){
            canvas.add(std::move(obj));
        }
        for (auto&[i, obj]: markers) {
            canvas.add(std::move(obj));
        }
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
    StatusCode TrackVisualizationTool::initialize() {
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
        ATH_CHECK(m_truthSegKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_extrapolationTool.retrieve(EnableTool{!m_extrapolationTool.empty()}));
        return StatusCode::SUCCESS;
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seederObj,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds) const {
        displaySeeds(ctx, seederObj, segments, seeds, PrimitivesVec_t{});
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seederObj,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds,
                                              PrimitivesVec_t && extPrimitives) const {
        if (m_plotsDone || segments.empty()) {
            return;
        }
        displaySeeds(ctx, seederObj, DisplayView::RZ, segments, seeds, clone(extPrimitives));        
        displaySeeds(ctx, seederObj, DisplayView::XY, segments, seeds, std::move(extPrimitives));
    
    }
    void TrackVisualizationTool::displaySeeds(const EventContext& ctx,
                                              const MsTrackSeeder& seeder,
                                              const DisplayView view,
                                              const xAOD::MuonSegmentContainer& segments,
                                              const MsTrackSeedContainer& seeds,
                                              PrimitivesVec_t&& extPrimitives) const{

        auto canvas = m_visualSvc->prepareCanvas(ctx, m_clientToken, std::format("SeedDisplay{:}",
                                                                                 view == DisplayView::XY ? "XY" : "RZ" ));  
        
        if (!canvas) {
            m_plotsDone = true;
            return;
        }     
        canvas->setAxisTitles(view == DisplayView::XY ?  "x [m]" : "z [m]",
                              view == DisplayView::XY ?  "y [m]" : "R [m]");
        
        PlotLegend legend{0.005,0.005, 0.6,0.1};
        /** First add the truth points*/
        fillTruthSeedPoints(ctx, seeder, view, legend, *canvas);
              
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
        bool drawnPoint{false};

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
                    drawnPoint = true;
                    const bool isGood = onSeed(segment, loc);
                    const int mStyle = stationMarkerSyle(chIdx, false, isGood);
                    const double phi = seeder.projectedPhi(segment->sector(), secProj);
                    const Amg::Vector2D markerPos{viewVector(phi, projPos, view)};
                    extPrimitives.emplace_back(drawMarker(markerPos, mStyle, mColor));

                    if (view == DisplayView::XY) {
                        const double r = markerPos.mag() + extraMargin;
                        canvas->expandPad(r, r);
                        canvas->expandPad(-r, -r);
                    } else {
                        canvas->expandPad(markerPos[0] - extraMargin, markerPos[1] - extraMargin);
                        canvas->expandPad(markerPos[0] + extraMargin, markerPos[1] + extraMargin);
                    }
                    legend.addMarker(mStyle, std::format("{:}{:}", MuonStationIndex::layerName(MuonStationIndex::toLayerIndex(chIdx)), 
                                                         isGood ? "" : " (discarded)"));
                    legend.addColor(mColor, msSector->barrel() ? "Barrel" : msSector->side() > 0 ? "Endcap A" : "Endcap C");
                }
            }
        }
        if (!drawnPoint) {
            canvas->trash();
            return;
        }
        for (const MsTrackSeed& seed : seeds) {
            const Amg::Vector3D& pos{seed.position()};
            canvas->add(drawMarker(viewVector(pos.phi(), Amg::Vector2D{pos.perp(), pos.z()}, view),
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
            const double r = canvas->corner(Edges::xHigh);
            const CxxUtils::sincos phiM{phi-dPhi}, phiP{phi+dPhi};
            const Amg::Vector2D e1{r * phiM.cs, r * phiM.sn};
            const Amg::Vector2D e2{r * phiP.cs, r * phiP.sn};

            auto theLine = std::make_unique<TLine>(0.,0., e1.x(), e1.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin(), std::move(theLine));
            theLine = std::make_unique<TLine>(0.,0., e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin(), std::move(theLine));
            theLine = std::make_unique<TLine>(e1.x(), e1.y(), e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            extPrimitives.insert(extPrimitives.begin(), std::move(theLine));
        }
        legend.fillPrimitives(*canvas);
    }
    void TrackVisualizationTool::fillTruthSeedPoints(const EventContext& ctx,
                                                     const MsTrackSeeder& seeder,
                                                     const DisplayView view,
                                                     PlotLegend& legend, 
                                                     Canvas_t& canvas) const {
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
                    canvas.add(drawMarker(viewVector(phi, projected, view), mStyle, truthColor, 3));
                    legend.addMarker(mStyle, Muon::MuonStationIndex::layerName(Muon::MuonStationIndex::toLayerIndex(chIdx)));
                    addedEntry = true;
                }
            }
        }
        if (addedEntry) {
            legend.addColor(truthColor, "truth");
        }
    }
    void TrackVisualizationTool::displayTrackSeedObj(const EventContext& ctx,
                                                     const MuonR4::MsTrackSeed& seed,
                                                     const OptBoundPars_t& parsToExt,
                                                     const std::string& objName) const {
        if (m_objCounter >= m_canvasLimit) {
            return;
        }
        const ActsTrk::GeometryContext* gctx{nullptr};
        SG::get(gctx, m_geoCtxKey, ctx).ignore();

        Acts::ObjVisualization3D visualHelper{};
        const std::string subDir = std::format("./ObjDisplays/{:}/", m_subDir.value());
        ensureDirectory(subDir);
        if (parsToExt.ok() && m_extrapolationTool.isEnabled()) {
            auto stepsResult = m_extrapolationTool->propagationSteps(ctx, *parsToExt);
            if (stepsResult.ok()) {
                MuonValR4::drawPropagation(stepsResult->first, visualHelper);
            } else {
                ATH_MSG_WARNING("Failed to extrapolate the seed for visualization: " << stepsResult.error().message());
            }
        }
        std::string segStr{removeNonAlphaNum(objName)};
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            MuonValR4::drawSegmentMeasurements(*gctx,* seg, visualHelper);
            MuonValR4::drawSegmentLine(*gctx,*seg, visualHelper);
            segStr += std::format("_{:}", MuonR4::printID(*seg));
        }

        unsigned fileVersion{0};
        std::string finalStr{};
        while (finalStr.empty() || std::filesystem::exists(finalStr)) {
            finalStr = std::format("{:}/{:}_{:}_{:}_{:}.obj", subDir, 
                m_clientToken.preFixName, ctx.eventID().event_number(), ++fileVersion, segStr);
        } 
        visualHelper.write(finalStr);
        ++m_objCounter;
    }
}