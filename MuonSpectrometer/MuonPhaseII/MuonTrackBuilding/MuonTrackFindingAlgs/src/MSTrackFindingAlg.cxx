/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSTrackFindingAlg.h"

#include "AthContainers/ConstDataVector.h"

#include "MuonStationIndex/MuonStationIndex.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "MuonTrackEvent/TrackingHelpers.h"

#include "CxxUtils/sincos.h"

#include "Acts/Utilities/KDTree.hpp"
#include <array>
#include <sstream>


#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "TCanvas.h"
#include "TMarker.h"
#include "TH2I.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLegend.h"
namespace {
    using LayIdx_t = Muon::MuonStationIndex::LayerIndex;
}

namespace MuonR4{
    StatusCode MSTrackFindingAlg::initialize() {
        ATH_CHECK(m_segmentKeys.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_magFieldKey.initialize());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_truthSegKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_msTrkSeedKey.initialize());
        m_summaryCan = std::make_unique<TCanvas>("summary", "summary", 800,600);
        m_summaryCan->SaveAs("AllMSTrksSeed.pdf[");

        gROOT->SetStyle("ATLAS");
        TStyle* plotStyle = gROOT->GetStyle("ATLAS");
        plotStyle->SetOptTitle(0);
        plotStyle->SetHistLineWidth(1.);
        plotStyle->SetPalette(kViridis);

        return StatusCode::SUCCESS;
    }
    StatusCode MSTrackFindingAlg::finalize() {
        if (m_summaryCan) m_summaryCan->SaveAs("AllMSTrksSeed.pdf]");
        return StatusCode::SUCCESS;
    }
    MSTrackFindingAlg::~MSTrackFindingAlg() = default;


    StatusCode MSTrackFindingAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("Run track finding in event "<<ctx.eventID().event_number());
 
        const SortTree_t segSearchTree = constructTree(ctx);

        auto seedContainer = std::make_unique<MsTrackSeedContainer>();

        findTrackSeeds(ctx,segSearchTree, *seedContainer);

        ATH_CHECK(drawEvent(ctx, segSearchTree, *seedContainer));


        SG::WriteHandle writeHandle{m_msTrkSeedKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(seedContainer)));
        
        return StatusCode::SUCCESS;
    }

    void MSTrackFindingAlg::fillInSegment(const xAOD::MuonSegment* seg, 
                                          const PlaneProjection project, 
                                          TreeDataVec_t& dataVec) const {
        const Segment* recoSeg = detailedSegment(*seg);
        const MuonGMR4::SpectrometerSector* msSec = recoSeg->msSector();
        const Amg::Vector3D& pos{recoSeg->position()};
        const Amg::Vector3D& dir{recoSeg->direction()};

        const Amg::Vector2D projPos{pos.perp(), pos.z()};
        const Amg::Vector2D projDir{dir.perp(), dir.z()};

        double lambda{0.};
        if (PlaneProjection::Barrel == project) {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitX(), 
                                        m_refBarrelR).value_or(0.);
        } else {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitY(), 
                                       msSec->side()* m_refEndcapDiscZ).value_or(0.);
        } 
        const Amg::Vector2D refPoint{ (projPos + lambda * projDir)};
        /// If it's a barrel segment, then the extrapolation must not
        /// cross the perpendicular disc
        if (std::abs(refPoint.y()) > m_refEndcapDiscZ) {
            return;
        } 
        /// Endcap segments may not exceed the radius of the endcap disc
        else if (refPoint.x() < 0 || refPoint.x() > m_refEndcapDiscR) {
            return;
        }
        ATH_MSG_VERBOSE("Sort segment in "<<msSec->identString()<<", 3D-pos: "
            <<Amg::toString(pos)<<" + "<<Amg::toString(dir)
            <<", chi2: "<<(recoSeg->chi2()/ recoSeg->nDoF())<<", nDoF: "<<recoSeg->nDoF()
            <<" projected 2D: "<<Amg::toString(projPos)
            <<", cross on ref cylinder: " <<Amg::toString(refPoint));

        SortTree_t::coordinate_t point;
        point[2] = seg->sector();
        point[1] = (project == PlaneProjection::Barrel ? refPoint.y() :  refPoint.x());
        point[0] = static_cast<int>(project)*msSec->side();
        /// To properly catch the sector overlap at -pi append the segment another time if it's in sector 1 or 16
        if (point[2] == 1 || point[2] == 16) {
            dataVec.emplace_back(point, seg);
            point[2] = point[2] == 1 ? 17 : 0;
            dataVec.emplace_back(std::move(point), seg);
        } else {
            dataVec.emplace_back(std::move(point), seg);
        }
    }

    MSTrackFindingAlg::SortTree_t 
        MSTrackFindingAlg::constructTree(const EventContext& ctx) const {
    
        TreeDataVec_t treeData{};
        /** Fetch the legacy & NSW segment container from store gate and  */
        for (const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& key : m_segmentKeys) {
            const xAOD::MuonSegmentContainer* inCont{nullptr};
            if(!SG::get(inCont, key, ctx).isSuccess()) {
                THROW_EXCEPTION("Tree filling failed");
            }

            treeData.reserve(2*inCont->size() + treeData.capacity());
                
            for (const xAOD::MuonSegment* seg : *inCont) {
                fillInSegment(seg, PlaneProjection::Barrel, treeData);
                fillInSegment(seg, PlaneProjection::Endcap, treeData);
            }
        }
        return SortTree_t{std::move(treeData)};
    }

    void MSTrackFindingAlg::findTrackSeeds(const EventContext& ctx,
                                           const SortTree_t& segSearchTree, 
                                           MsTrackSeedContainer& trackSeeds) const {

        for (const auto& [coords, seedCandidate] : segSearchTree) {
            /** Bad segment not suitable for track seeding or the segment coordinates are
             *  just mirrored at the overlap between sector 1 -> 16 */
            const Segment* recoCandidate = detailedSegment(*seedCandidate);
             if (coords[2] == 0 || coords[2] == 17 || 
                !m_segSelector->passSeedingQuality(ctx, *recoCandidate)){
                continue;
            }
            const MuonGMR4::SpectrometerSector* msSeg = recoCandidate->msSector();
            
            /** Define the search range. */    
            SortTree_t::range_t selectRange{};
            /** Ensure that only endcap / barrel seeds are considered */
            selectRange[0].shrink(coords[0] - 0.1, coords[0] + 0.1);
            /** Move 25 cm along the projected plane */
            selectRange[1].shrink(coords[1] - 25.*Gaudi::Units::cm, coords[1] + 25.*Gaudi::Units::cm);
            /** Include the neighbouring sectors */
            selectRange[2].shrink(coords[2]-1.25, coords[2] + 1.25);
            
            MsTrackSeed newSeed{};
            /** Using the cube above, let the tree search for all compatible segments */
            
            segSearchTree.rangeSearchMapDiscard(selectRange, [this, &ctx, &newSeed, &recoCandidate](
                    const SortTree_t::coordinate_t& /*coords*/,
                    const SortTree_t::value_t &extendWithMe) {
                        /** Ensure that the sector overlap and momentum vectors are compatible with a MS trajectory */
                        const Segment* extendCandidate = detailedSegment(*extendWithMe);
                        if (m_segSelector->compatibleForTrack(ctx, *recoCandidate, *extendCandidate)) {
                            newSeed.addSegment(extendWithMe);
                        }
            });
            /** No segments were combined */
            if (newSeed.segments().empty()) {
                continue;
            }
            newSeed.addSegment(seedCandidate);
            /** @brief calculate the seed's position */
            const double r = coords[0] == static_cast<int>(PlaneProjection::Barrel) ? m_refBarrelR : coords[1];
            const double z = coords[0] == static_cast<int>(PlaneProjection::Barrel) ? coords[1] : msSeg->side()* m_refEndcapDiscZ;
            Amg::Vector3D pos = r * Amg::dirFromAngles(seedCandidate->position().phi(), 90. * Gaudi::Units::deg)
                              + z * Amg::Vector3D::UnitZ();
            
            newSeed.setPosition(std::move(pos));
            
            /** Search in all existing seeds whether the segments are already part of it  */
            MsTrackSeedContainer::iterator exist_itr = 
                std::ranges::find_if(trackSeeds, [&newSeed](const MsTrackSeed& exist) {
                    const int sector = newSeed.msSector()->sector();
                    const int side = newSeed.msSector()->side();
                    const int exSector = exist.msSector()->sector();
                    const int exSide =  exist.msSector()->side();
                    if (side != exSide || std::abs(sector - exSector) > 1){
                        return false;
                    }
                    const unsigned overlap = std::ranges::count_if(newSeed.segments(), 
                        [&exist](const xAOD::MuonSegment* seg){
                        return std::ranges::find(exist.segments(), seg)!= exist.segments().end();
                    });
                    return overlap == newSeed.segments().size();
                });
            if (exist_itr != trackSeeds.end()) {
                if (exist_itr->segments().size() >= newSeed.segments().size()) {
                    continue;
                }
                /** This seed is better replace the worse one with this one*/
                std::swap(*exist_itr, newSeed);
                continue;
            }
            trackSeeds.emplace_back(std::move(newSeed));
        }
    }
    StatusCode MSTrackFindingAlg::drawEvent(const EventContext& ctx,
                                            const SortTree_t& segSearchTree,
                                            const MsTrackSeedContainer& trackSeeds) const {


        if (segSearchTree.begin() == segSearchTree.end() || !m_summaryCan) {
            return StatusCode::SUCCESS;
        }
        static std::mutex canvasMutex{};
        std::lock_guard guard{canvasMutex};

        if (!m_summaryCan) {
            return StatusCode::SUCCESS;
        }

        std::vector<std::unique_ptr<TObject>> primitives{}, primitivesRPhi{}, legendPrim{};

        primitives.emplace_back(MuonValR4::drawLabel(std::format("Event: {:}", ctx.eventID().event_number()),0.3,0.96));
        primitivesRPhi.emplace_back(MuonValR4::drawLabel(std::format("Event: {:}", ctx.eventID().event_number()),0.3,0.96));

        
        /// Truth parameter markers
        const xAOD::MuonSegmentContainer* truthSegs{nullptr};
        ATH_CHECK(SG::get(truthSegs, m_truthSegKey, ctx));
        if (truthSegs) {
            std::unordered_set<const xAOD::TruthParticle*> truth{nullptr};
            double legY{0.9};
            for (const xAOD::MuonSegment* seg : *truthSegs){
                const Amg::Vector3D pos{seg->position()};
                const Amg::Vector3D dir{seg->direction()};

                const Amg::Vector2D projPos{pos.perp(), pos.z()};
                const Amg::Vector2D projDir{dir.perp(), dir.z()};

                CxxUtils::sincos phi{pos.phi()};

                double lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitX(), 
                                                  m_refBarrelR).value_or(0.);
           
                const Amg::Vector2D barrel2D{projPos + lambda * projDir};

            
                int markerStyle{0};
                switch (Muon::MuonStationIndex::toLayerIndex(seg->chamberIndex())) {
                    case LayIdx_t::Inner:
                    case LayIdx_t::BarrelExtended:
                    case LayIdx_t::Extended:
                        markerStyle =  kFullTriangleUp;
                        break;
                    case LayIdx_t::Middle:
                        markerStyle =  kFullCrossX;
                        break;
                    case LayIdx_t::Outer:
                        markerStyle =  kFullFourTrianglesX;
                        break;
                    default:
                        break;
                }
            
                auto marker = std::make_unique<TMarker>(barrel2D.x(),barrel2D.y(), markerStyle);
                marker->SetMarkerColor(kOrange + 2);
                marker->SetMarkerSize(3);
                if (std::abs(barrel2D.y()) < m_refEndcapDiscZ) {
                    primitives.emplace_back(std::move(marker));
                    marker = std::make_unique<TMarker>(barrel2D.x() * phi.cs,
                                                       barrel2D.x() * phi.sn, markerStyle);
                    marker->SetMarkerColor(kOrange + 2);
                    marker->SetMarkerSize(3);
                    primitivesRPhi.emplace_back(std::move(marker));
                }

                lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitY(), 
                                           sign(pos.z())* m_refEndcapDiscZ).value_or(0.);
            
                const Amg::Vector2D endcap2D{projPos + lambda * projDir};

                marker = std::make_unique<TMarker>(endcap2D.x(),endcap2D.y(), markerStyle);
                marker->SetMarkerColor(kOrange + 2);
                marker->SetMarkerSize(3);
                if (endcap2D.x() < m_refEndcapDiscR) {
                    primitives.emplace_back(std::move(marker));
                    marker = std::make_unique<TMarker>(endcap2D.x() * phi.cs,
                                                       endcap2D.x() * phi.sn, markerStyle);
                    marker->SetMarkerColor(kOrange + 2);
                    marker->SetMarkerSize(3);
                    primitivesRPhi.emplace_back(std::move(marker));
                }

                const auto insert_itr = truth.insert(getTruthMatchedParticle(*seg));
                if (insert_itr.second) {
                   const xAOD::TruthParticle* truthLabel = *insert_itr.first;

                   primitives.emplace_back(MuonValR4::drawLabel( std::format("truth p_{{T}}: {:.2f} GeV, #eta: {:.2f}, #phi: {:.1f}", 
                                                                   truthLabel->pt() / 1.e3, truthLabel->eta(), truthLabel->phi() / Gaudi::Units::deg), 0.2, legY));
                   primitivesRPhi.emplace_back(primitives.back()->Clone());
                   legY-=0.05;
                }
            }
        }
        
        Acts::RangeXD<2, double> bounding{std::array{1.e9,1.e9}, std::array{-1.e9,-1.e9}};
        std::stringstream sstr{};
        sstr<<" Found "<<segSearchTree.size()<<" segments: "<<std::endl;
        
        auto segOnSeed = [&trackSeeds](const Segment* segment) -> bool{
            return std::ranges::find_if(trackSeeds, [&segment](const MsTrackSeed& seed){
                return std::ranges::find(seed.detailedSegments(),segment) != seed.detailedSegments().end();
            }) != trackSeeds.end();
        };
        for (const auto& [coords, aodSeg] : segSearchTree){
            if (coords[2] == 0 || coords[2] == 17){
                continue;
            }
            const double r = (coords[0] == static_cast<int>(PlaneProjection::Barrel) ? m_refBarrelR : coords[1]);
            const double z = (coords[0] == static_cast<int>(PlaneProjection::Barrel) ? coords[1] : m_refEndcapDiscZ *  coords[0]);
            const Segment* seg = detailedSegment(*aodSeg);
            const MuonGMR4::SpectrometerSector* sector = seg->msSector();
            sstr<<" **** "<<sector->identString()<<" "<<Amg::toString(seg->position())<<" + "<<Amg::toString(seg->direction())
                            <<", nDoF: "<<seg->nDoF()
                            <<", seed: "<<m_segSelector->passSeedingQuality(ctx, *seg)
                            <<", extend: "<<m_segSelector->passTrackQuality(ctx, *seg)
                            <<", r: "<<r<<", z: "<<z <<", phi: "<<seg->position().phi()<<std::endl;
            for (const auto& meas : seg->measurements()) {
                if (meas->spacePoint()) {
                    sstr<<"      --- "<<m_idHelperSvc->toString(meas->spacePoint()->identify())
                            <<(meas->fitState() == CalibratedSpacePoint::State::Valid ? " good hit" : " baad hit")
                            <<", dim: "<<meas->dimension()<<std::endl;
                }
            }

            int markerStyle{0}, markerColor{sector->barrel() ? kRed : (sector->side() > 0 ? kBlue : kGreen)};
               
            switch (Muon::MuonStationIndex::toLayerIndex(seg->msSector()->chamberIndex())) {
                case LayIdx_t::Inner:
                case LayIdx_t::BarrelExtended:
                case LayIdx_t::Extended:
                    markerStyle = segOnSeed(seg) ? kOpenTriangleUp : kOpenTriangleDown;                   
                    break;
                case LayIdx_t::Middle:
                    markerStyle = segOnSeed(seg) ? kOpenCrossX : kOpenCross;
                    break;
                case LayIdx_t::Outer:
                    markerStyle = segOnSeed(seg) ?  kOpenFourTrianglesX : kOpenThreeTriangles;                    
                    break;
                default:
                    break;
            }

            bounding[0].expand(r-50.*Gaudi::Units::cm, r+ 50.*Gaudi::Units::cm);
            bounding[1].expand(z-50.*Gaudi::Units::cm, z+ 50.*Gaudi::Units::cm);
            auto marker = std::make_unique<TMarker>(r,z, markerStyle);
            marker->SetMarkerColor(markerColor);
            marker->SetMarkerSize(2);
            primitives.emplace_back(std::move(marker));
            CxxUtils::sincos phi{seg->position().phi()};
            const double x = r * phi.cs;
            const double y = r * phi.sn;
            marker = std::make_unique<TMarker>(x,y, markerStyle);
            marker->SetMarkerColor(markerColor);
            marker->SetMarkerSize(2);
            primitivesRPhi.emplace_back(std::move(marker));
        }
        /** Draw the seeds in the R-Z & x-y plane */
        
        for (const MsTrackSeed& seed : trackSeeds) {
            Acts::RangeXD<2, double> seedBounds{std::array{1.e9,1.e9}, std::array{-1.e9,-1.e9}};
            
            const Amg::Vector3D& pos{seed.position()};
            const double r = seed.position().perp();
            const double z = seed.position().z();
            
            auto marker = std::make_unique<TMarker>(pos.x(),pos.y(), kFullDiamond);
            marker->SetMarkerColor(kBlack);
            marker->SetMarkerSize(2);
            primitivesRPhi.push_back(std::move(marker));
            marker = std::make_unique<TMarker>(r,z, kFullDiamond);
            marker->SetMarkerColor(kBlack);
            marker->SetMarkerSize(2);
            primitives.emplace_back(std::move(marker));
        }
        
        /** Draw the sector map */
        for (unsigned int s = 1; s<=16 ; ++s) {
            const double phi =  m_sectorMap.sectorPhi(s);
            const double dPhi = m_sectorMap.sectorWidth(s);

            const int lStyle = s%2 ? kDashed : kDotted;
            const double r = bounding[0].max();
            const CxxUtils::sincos phiM{phi-dPhi}, phiP{phi+dPhi};
            const Amg::Vector2D e1{r * phiM.cs, r * phiM.sn};
            const Amg::Vector2D e2{r * phiP.cs, r * phiP.sn};

            auto theLine = std::make_unique<TLine>(0.,0., e1.x(), e1.y());
            theLine->SetLineStyle(lStyle);
            primitivesRPhi.insert(primitivesRPhi.begin(), std::move(theLine));
            theLine = std::make_unique<TLine>(0.,0., e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            primitivesRPhi.insert(primitivesRPhi.begin(), std::move(theLine));
            theLine = std::make_unique<TLine>(e1.x(), e1.y(), e2.x(), e2.y());
            theLine->SetLineStyle(lStyle);
            primitivesRPhi.insert(primitivesRPhi.begin(), std::move(theLine));
        }
        /** Draw the legend */
        auto legend = std::make_unique<TLegend>(0.1,0.01,0.5,0.2);
        {
            auto triangle = std::make_unique<TMarker>(0.,0, kOpenTriangleUp);            
            legend->AddEntry(triangle.get(), "Inner");
            legendPrim.emplace_back(std::move(triangle));
            auto openCross = std::make_unique<TMarker>(0,0, kOpenCrossX);
            legend->AddEntry(openCross.get(), "Middle");
            legendPrim.emplace_back(std::move(openCross));
            triangle = std::make_unique<TMarker>(0,0, kOpenFourTrianglesX);
            legend->AddEntry(triangle.get(), "Outer");
            legendPrim.emplace_back(std::move(triangle));


        }

        ATH_MSG_VERBOSE(std::endl<<sstr.str());

        auto canvas = std::make_unique<TCanvas>("can", "can", 800, 600);
        canvas->cd();
  
        auto h1 = std::make_unique<TH2I>("frame", "frame;r[mm];z[mm]", 
                                         1, bounding[0].min(), bounding[0].max(), 
                                         1, bounding[1].min(), bounding[1].max());
        h1->Draw("AXIS");
        for (auto& prim : primitives) {
            prim->Draw();
        }
        legend->SetBorderSize(0);
        legend->Draw();
        canvas->SaveAs("AllMSTrksSeed.pdf");
        primitives.clear();

        h1 = std::make_unique<TH2I>("frame1","frame;x[mm];y[mm]",
                                    1,-bounding[0].max(),bounding[0].max(),
                                    1,-bounding[0].max(),bounding[0].max());
        h1->Draw("AXIS");
        for (auto& prim : primitivesRPhi) {
            prim->Draw();
        }
        legend->Draw();
        canvas->SaveAs("AllMSTrksSeed.pdf");

        if (++m_canvCounter >= 5000) {
            m_summaryCan->SaveAs("AllMSTrksSeed.pdf]");
            m_summaryCan.reset();
        }
        return StatusCode::SUCCESS;
    }

}

