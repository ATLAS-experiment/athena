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

    //** @brief Visualizing seeds and corresponding segments with different colors */
    int seedColorIdx(std::size_t iSeed){
        // Golden ratio hue stepping -> good spread even for many seeds
        constexpr double phi = 0.6180339887498949;
        const double h = 360 * std::fmod(0.13 + phi * double(iSeed), 1.0);  // [0,1)
        constexpr double s = 0.75;
        constexpr double v = 0.95;

        float r=0.f, g=0.f, b=0.f;
        TColor::HLS2RGB(h, s, v, r, g, b);
        return TColor::GetColor(r, g, b);
    }

    void TrackVisualizationTool::PlotLegend::addColor(const int color, const std::string& label){
        if (colors.find(color) != colors.end()) {
            return;
        }
        auto box = MuonValR4::drawBox(-1.5,-1.5,-1.,-1., color, MuonValR4::fullFilling);
        legend->AddEntry(box.get(), label.c_str(), "F");
        colors.insert(std::make_pair(color, std::move(box)));
    }
    void TrackVisualizationTool::PlotLegend::addMarker(const int marker, const std::string& label, const int color){
        if (markers.find(marker)!= markers.end()) {
            return;
        }
        auto tMarker = drawMarker(Amg::Vector2D{2.*Gaudi::Units::km, 0.}, marker, color);
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

        //For each seed plot respective segment parameters

        displaySeedSegmentsGlobalWithTruth(ctx, DisplayView::RZ, seeds);
        displaySeedSegmentsGlobalWithTruth(ctx, DisplayView::XY, seeds);
    
    }

void TrackVisualizationTool::displaySeedSegmentsGlobalWithTruth(
    const EventContext& ctx,
    const DisplayView view,
    const MsTrackSeedContainer& seeds) const {

  auto canvas = m_visualSvc->prepareCanvas(
      ctx, m_clientToken,
      std::format("SeedSegsGlobalTruth{:}", view == DisplayView::XY ? "XY" : "RZ"));

  if (!canvas) { m_plotsDone = true; return; }

  canvas->setAxisTitles(view == DisplayView::XY ? "x [m]" : "z [m]",
                        view == DisplayView::XY ? "y [m]" : "R [m]");

  PlotLegend legend{0.005, 0.005, 0.6, 0.1};

  PrimitivesVec_t primitives{};
  bool drewAny{false};

  // -----------------------
  // (A) Draw TRUTH segments
  // -----------------------
  const xAOD::MuonSegmentContainer* truthSegs{nullptr};
  if (!SG::get(truthSegs, m_truthSegKey, ctx).isSuccess()) {
    THROW_EXCEPTION("Failed to fetch the truth segment container");
  }
  if (truthSegs) {
    for (const xAOD::MuonSegment* seg : *truthSegs) {
      if (!seg) continue;

      const Amg::Vector3D p = seg->position();
      const double x = p.x() * inM;
      const double y = p.y() * inM;
      const double z = p.z() * inM;
      const double r = std::hypot(x, y);

      const Amg::Vector2D plotPos =
          (view == DisplayView::XY) ? Amg::Vector2D{x, y}
                                   : Amg::Vector2D{z, r};

      // Same station-based marker, but OPEN marker to visually separate truth vs reco
      const auto chIdx = seg->chamberIndex();
      const int mStyleTruth = stationMarkerSyle(chIdx, /*openMarker=*/true, /*onSeed=*/true);

      primitives.emplace_back(drawMarker(plotPos, mStyleTruth, truthColor, /*mSize=*/3));
      legend.addMarker(
          mStyleTruth,
          std::string(Muon::MuonStationIndex::layerName(Muon::MuonStationIndex::toLayerIndex(chIdx))) + " (truth)", truthColor);

      canvas->expandPad(plotPos.x() - extraMargin, plotPos.y() - extraMargin);
      canvas->expandPad(plotPos.x() + extraMargin, plotPos.y() + extraMargin);
      drewAny = true;
    }
  }

  // ---------------------------------
  // (B) Draw RECO seed-associated segs
  // ---------------------------------
  for (std::size_t iSeed = 0; iSeed < seeds.size(); ++iSeed) {
    const MsTrackSeed& seed = seeds[iSeed];
    const int color = seedColorIdx(iSeed);

    std::string seedLabel = std::format("s{:d}: ", static_cast<int>(iSeed));

    for (const xAOD::MuonSegment* seg : seed.segments()) {
      if (!seg) continue;

      const Amg::Vector3D p = seg->position();
      const double x = p.x() * inM;
      const double y = p.y() * inM;
      const double z = p.z() * inM;
      const double r = std::hypot(x, y);

      const Amg::Vector2D plotPos =
          (view == DisplayView::XY) ? Amg::Vector2D{x, y}
                                   : Amg::Vector2D{z, r};

      const auto chIdx = seg->chamberIndex();
      const int mStyle = stationMarkerSyle(chIdx, /*openMarker=*/false, /*onSeed=*/true);

      primitives.emplace_back(drawMarker(plotPos, mStyle, color));

      std::string label = printID(*seg);
      seedLabel += label + "_";

      primitives.emplace_back(MuonValR4::drawLabel(label, plotPos.x() + 0.01, plotPos.y() + 0.01, 10, false, color));

      // Marker legend: only one entry per station style (PlotLegend dedups)
      legend.addMarker(
          mStyle,
          std::string(Muon::MuonStationIndex::layerName(Muon::MuonStationIndex::toLayerIndex(chIdx))));

      canvas->expandPad(plotPos.x() - extraMargin, plotPos.y() - extraMargin);
      canvas->expandPad(plotPos.x() + extraMargin, plotPos.y() + extraMargin);
      drewAny = true;
    }
      //Add text which shows segment names associated to the respective seeed
      primitives.emplace_back(MuonValR4::drawLabel(seedLabel, 0.15, 0.9 - (0.03*iSeed), 10, true, color));
  }

  if (!drewAny) { canvas->trash(); return; }

  for (auto& p : primitives) canvas->add(std::move(p));
  legend.fillPrimitives(*canvas);
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
                    const int mStyle = stationMarkerSyle(chIdx, true, isGood);
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
                                                         isGood ? "" : " (discarded)"), mColor);
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
            const int mStyle = stationMarkerSyle(chIdx, true, true);

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
                    legend.addMarker(mStyle, Muon::MuonStationIndex::layerName(Muon::MuonStationIndex::toLayerIndex(chIdx)), truthColor);
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