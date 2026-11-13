/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#if defined(FLATTEN) && defined(__GNUC__)
// Avoid warning in dbg build
#pragma GCC optimize "-fno-var-tracking-assignments"
#endif

#include "MuonChamberToolTest.h"

#include "MuonReadoutGeometryR4/Chamber.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/BoundsExpander.h"

#include "ActsGeometryInterfaces/ISurfacePlacement.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/DiamondVolumeBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Geometry/VolumePlacementBase.hpp"

#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Enumerate.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"

#include "MuonVisualizationHelpersR4/FileHelpers.h"
#include "GeoModelHelpers/StringUtils.h"

#include <format>
#include <array>

using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;

namespace{
    constexpr double tolerance = 10. *Gaudi::Units::micrometer;

    std::vector<std::shared_ptr<const Acts::Volume>> chamberVolumes(const MuonGMR4::SpectrometerSector& sector) {
        std::vector<std::shared_ptr<const Acts::Volume>> vols{};
        std::ranges::transform(sector.chambers(),std::back_inserter(vols), 
                                [](const auto& ch){ return ch->boundingVolume(); });
        return vols;
    }
    std::vector<const Acts::Volume*> chamberVolumes(const Acts::TrackingVolume& vol) {
        std::vector<const Acts::Volume*> children {};
        for (const Acts::TrackingVolume& childVol : vol.volumes()) {
            children.push_back(&childVol);
            std::vector<const Acts::Volume*> grandChildren = chamberVolumes(childVol);
            std::ranges::move(grandChildren, std::back_inserter(children));
        }
        return children;
    }
    
    std::vector<const Acts::Surface*> extractSurfaces(std::span<const MuonGMR4::MuonReadoutElement* const> reEles){
        std::vector<const Acts::Surface*> surfaces{};
        for (const MuonGMR4::MuonReadoutElement* re : reEles) {
            std::ranges::copy(Acts::unpackSmartPointers(re->getSurfaces()),
                              std::back_inserter(surfaces));
        }
        return surfaces; 
    }

    std::vector<const Acts::Surface*> extractSurfaces(const Acts::TrackingVolume& volume) {
        std::vector<const Acts::Surface*> surfaces{};
        std::ranges::for_each(volume.surfaces(), [&surfaces](const Acts::Surface& surface){
            surfaces.push_back(&surface);
        });
        for (const Acts::TrackingVolume& subVol : volume.volumes()) {
            std::ranges::move(extractSurfaces(subVol), std::back_inserter(surfaces));
        }
        return surfaces; 
    }

    Identifier identify(const Acts::Surface& surface) {
        const auto* detEl = dynamic_cast<const ActsTrk::ISurfacePlacement*>(surface.surfacePlacement());
        return detEl ? detEl->identify(): Identifier{};
    }
    std::string cleanVolumeName(std::string volName) {
        using namespace GeoStrUtils;
        static const std::vector<std::pair<std::string, std::string>> patterns{
            std::make_pair(" ", "_"),
            std::make_pair(":", ""),
            std::make_pair(",", ""),
            std::make_pair("-", "m"),
            std::make_pair(".", "p")
        };
        for ( auto [exp, replace] : patterns) {
            volName = replaceExpInString(volName, exp, replace);
        }
        return volName;
    }
    std::string toString(std::span<const Amg::Vector3D> vert){
        std::stringstream sstr{};
        for (std::size_t v =0 ; v < vert.size(); ++v){
            sstr<<Amg::toString(vert[v])<<", perp: "<<vert[v].perp()<<std::endl;
        }
        return sstr.str();
    }
    template <typename T> void removeDuplicates(std::vector<T>& v) {
        auto [begin, end] = std::ranges::unique(v);
        v.erase(begin,end);
    }
}

namespace MuonGMR4 {

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());      
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        return StatusCode::SUCCESS;
    }
#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
    StatusCode MuonChamberToolTest::pointInside(const ActsTrk::GeometryContext& gctx,
                                                const Acts::Volume& boundVol,
                                                const Amg::Vector3D& point,
                                                const std::string_view descr,
                                                const Identifier& channelId) const {
        const Amg::Vector3D locPos{boundVol.globalToLocalTransform(gctx.context()) * point};
        const Acts::VolumeBounds& bounds{boundVol.volumeBounds()};
        if (bounds.inside(locPos, tolerance)) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - In channel "<<m_idHelperSvc->toString(channelId)
                            <<", point "<<descr <<" is inside of the chamber "
                            <<std::endl<<bounds<<std::endl
                            <<"Local position:" <<Amg::toString(locPos));
            return StatusCode::SUCCESS;
        }
        ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - In channel "<<m_idHelperSvc->toString(channelId) <<", the point "
            << descr <<" "<<Amg::toString(point)<<" is not part of the chamber volume."
            <<std::endl<<bounds<<", local position "
            <<Amg::toString(locPos));
        const double shortX = MuonGMR4::halfXlowY(bounds);
        const double longX = MuonGMR4::halfXhighY(bounds);
        const double halfY = MuonGMR4::halfY(bounds);
        const Amg::Vector3D dir = Amg::Vector3D {Acts::copySign(longX - shortX, locPos.x()), 
                                                  2.*halfY,0.}.unit();
        const Amg::Vector3D pos{Acts::copySign(shortX, locPos.x()), -halfY, 0.};

        const Amg::Vector3D sameY = Acts::PlanarHelper::intersectPlane(pos, dir, Amg::Vector3D::UnitY(), 
                                                                        locPos.y()).position();
        ATH_MSG_ALWAYS(__func__<<"() "<<__LINE__<<" - Inclination angle "<<Amg::toString(dir)<<", "<<dir.phi()<<", reference edge: "
                    <<Amg::toString(pos) <<", edge point: "<<Amg::toString(sameY));
        return StatusCode::FAILURE;
    }

    StatusCode MuonChamberToolTest::allReadoutInEnvelope(const ActsTrk::GeometryContext& gctx,
                                                         const Chamber& envelope) const {
        std::shared_ptr<Acts::Volume> boundVol = envelope.boundingVolume();
        const Chamber::ReadoutSet reEles = envelope.readoutEles();
        for(const MuonReadoutElement* readOut : reEles) {
            if (readOut->chamber() != &envelope) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Mismatch in the chamber association "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                    <<std::endl<<(*readOut->chamber())<<std::endl<<envelope);
                return StatusCode::FAILURE;
            }
            switch (readOut->detectorType()) {
                case ActsTrk::DetectorType::Tgc: {
                    const auto* detEle = static_cast<const TgcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::Mdt: {
                    const auto* detEle = static_cast<const MdtReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::Rpc: {
                    const auto* detEle = static_cast<const RpcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, *boundVol));
                    break; 
                }  case ActsTrk::DetectorType::Mm: {
                    const auto* detEle = static_cast<const MmReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::sTgc: {
                    const auto* detEle = static_cast<const sTgcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, *boundVol));
                    break; 
                } default: {
                    ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Who came up with putting "<<readOut->detectorType()<<" into the MS");
                    return StatusCode::FAILURE;
                }
            }
            ATH_CHECK(testReadoutSurfaces(gctx, *readOut, *boundVol));
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - All "<<reEles.size()<<" readout elements are embedded in "<<envelope);
        return StatusCode::SUCCESS;
    }
#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
    bool MuonChamberToolTest::hasOverlap(const ActsTrk::GeometryContext& gctx,
                                         const std::vector<Amg::Vector3D>& chamberEdges,
                                         const Acts::Volume& volume) const {
        
        /// First check that there's at least a chance that the edges might overlap with the center
        const Amg::Vector3D center{volume.center(gctx.context())};
        double minDist = 1._km;
        for (const Amg::Vector3D& edge : chamberEdges) {
            minDist = std::min(minDist, (edge - center).mag());
        }
        /// The bound values contain the half length's if the minDist is within reach
        /// Then continue the check
        if (std::ranges::none_of(volume.volumeBounds().values(), 
            [minDist](const double bound){
                return minDist < 2.5*bound;
            })) {
            return false;
        }
        const double stepLength = 1. / m_overlapSamples;

        const Acts::VolumeBounds& volBounds = volume.volumeBounds();
        const Acts::Transform3& transform = volume.globalToLocalTransform(gctx.context());
        for (unsigned edge1 = 1; edge1 < chamberEdges.size(); ++edge1) {
            for (unsigned edge2 = 0; edge2 < edge1; ++edge2) {
                for (unsigned step = 0 ; step <= m_overlapSamples; ++step) {
                    const double section = stepLength * step;
                    const Amg::Vector3D testPoint = section* chamberEdges[edge1] + (1. -section) *chamberEdges[edge2];
                    // Using acts::Volume::inside is horribly slow in dbg builds.
                    // Using the bounds method directly is much faster.
                    if (volBounds.inside (transform * testPoint)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    StatusCode MuonChamberToolTest::checkChambers(const ActsTrk::GeometryContext& gctx) const {

        const Acts::GeometryContext tgContext = gctx.context();
        const std::vector<const MuonReadoutElement*> allRE = m_detMgr->getAllReadoutElements();
        using ChamberSet = MuonDetectorManager::MuonChamberSet;
        const ChamberSet chamberVec = m_detMgr->getAllChambers();
        ATH_MSG_INFO(__func__<<"() "<<__LINE__<<" - Fetched "<<chamberVec.size()<<" chambers.");

        
        const auto missChamb = std::ranges::find_if(allRE, [&chamberVec](const MuonGMR4::MuonReadoutElement* re){
            return !Acts::rangeContainsValue(chamberVec, re->chamber());
        });
        if (missChamb != allRE.end()) {
            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The chamber "<<(*(*missChamb)->chamber())
                        <<" is not in the chamber set");
            return StatusCode::FAILURE;
        }

        // Retrieve bounds here rather than inside the loop below,
        // so we only need to do it O(N) rather than O(N^2) times.
        std::vector<std::shared_ptr<Acts::Volume> > chamberBoundsVec;
        chamberBoundsVec.reserve (chamberVec.size());
        for (const Chamber* ch : chamberVec) {
          chamberBoundsVec.push_back(ch->boundingVolume());
        }
        std::set<const Chamber*> overlapChambers{};
        std::stringstream overlapstream{};
        for (std::size_t chIdx = 0; chIdx< chamberVec.size(); ++chIdx) {
            const Chamber& chamber{*chamberVec[chIdx]};
            const Acts::Volume& chamberBounds = *chamberBoundsVec[chIdx];
            if (m_dumpObjs) {
                saveEnvelope(tgContext, std::format("Chamber_{:}{:}{:}{:}{:}", 
                                                chamber.detectorType(),
                                                chamber.chamberIndex(),
                                                std::abs(chamber.stationEta()),
                                                chamber.stationEta() > 0 ? 'A' : 'C',
                                                chamber.stationPhi()), 
                            chamberBounds, extractSurfaces(chamber.readoutEles()), {});
            }
            ATH_CHECK(allReadoutInEnvelope(gctx, chamber));
            const std::vector<Amg::Vector3D> chambCorners = BoundsExpander::cornerPoints(tgContext, chamberBounds);
            /// Check the overlap with other chambers
            std::vector<const Chamber*> overlaps{};
            for (std::size_t chIdx1 = 0; chIdx1<chamberVec.size(); ++chIdx1) {
                if (chIdx == chIdx1) {
                    continue;
                }
                const Chamber* overlapTest{chamberVec[chIdx1]};
                if (hasOverlap(gctx, chambCorners, *chamberBoundsVec[chIdx1])) {
                    overlaps.push_back(overlapTest);
                }
            }
            if (overlaps.empty()) {
                continue;
            }
            overlapstream<<"The chamber "<<chamber<<" overlaps with "<<std::endl;
            for (const Chamber* itOverlaps : overlaps) {
                 overlapstream<<" ***  "<<(*itOverlaps)<<std::endl;
            }
            overlapstream<<std::endl<<std::endl;
            overlapChambers.insert(overlaps.begin(), overlaps.end());
            overlapChambers.insert(chamberVec[chIdx]);
        }
        if (!overlapChambers.empty()) {
            Acts::ObjVisualization3D visualHelper{};
            for (const Chamber* hasOverlap: overlapChambers) {
                Acts::GeometryView3D::drawVolume(visualHelper, *hasOverlap->boundingVolume(), gctx.context());
                visualHelper.write(m_overlapChambObj.value());
            }
            if (m_ignoreOverlapCh) {
                ATH_MSG_WARNING(overlapstream.str());
            } else {
                ATH_MSG_ERROR(overlapstream.str());
            }
        }
        ATH_MSG_INFO(__func__<<"() "<<__LINE__<<" - Chamber test completed. Found "<<overlapChambers.size()<<" overlapping chambers");
        return overlapChambers.empty() || m_ignoreOverlapCh ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    StatusCode MuonChamberToolTest::checkEnvelopes(const ActsTrk::GeometryContext& gctx) const {
        const Acts::GeometryContext tgContext = gctx.context();
        const std::vector<const MuonReadoutElement*> allREs = m_detMgr->getAllReadoutElements();
        using SectorSet = MuonDetectorManager::MuonSectorSet;
        const SectorSet sectors = m_detMgr->getAllSectors();

        for (const MuonReadoutElement* re : allREs) {
            if (!re->msSector()) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The readout element "<<m_idHelperSvc->toStringDetEl(re->identify())<<" does not have any sector associated ");
                return StatusCode::FAILURE;
            }
            const SpectrometerSector* sectorFromDet = m_detMgr->getSectorEnvelope(re->chamberIndex(), 
                                                                                  m_idHelperSvc->sector(re->identify()),
                                                                                  re->stationEta());
            if (sectorFromDet != re->msSector()) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The sector attached to "<<m_idHelperSvc->toStringDetEl(re->identify())
                          <<", chIdx: "<<re->chamberIndex()<<", sector: "<<m_idHelperSvc->sector(re->identify())
                          <<" is not the one attached to the readout geometry \n"<<(*re->msSector())<<"\n"<<(*sectorFromDet));
                return StatusCode::FAILURE;
            }
            if (!Acts::rangeContainsValue(sectors, sectorFromDet)){
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The sector "<<(*sectorFromDet)<<" is not part of the sector set");
                return StatusCode::FAILURE;
            }
        }

        ATH_MSG_INFO(__func__<<"() "<<__LINE__<<" - Fetched "<<sectors.size()<<" sectors. ");
        for (const SpectrometerSector* sector : sectors) {
            if (m_dumpObjs) {
                const auto subVols = chamberVolumes(*sector);
                saveEnvelope(tgContext, std::format("Sector_{:}{:}{:}",
                                               sector->chamberIndex(),
                                               sector->side()  >0? 'A' :'C', 
                                               sector->stationPhi()  ), 
                            *sector->boundingVolume(), 
                            extractSurfaces(sector->readoutEles()),
                            Acts::unpackSmartPointers(subVols));
            }
            const std::shared_ptr<Acts::Volume> secVolume = sector->boundingVolume();
            for (const SpectrometerSector::ChamberPtr& chamber : sector->chambers()){
                for (const auto&[edgeCount, edge]  : 
                        Acts::enumerate(BoundsExpander::cornerPoints(tgContext, *chamber->boundingVolume()))) {
                    ATH_CHECK(pointInside(gctx, *secVolume, edge, std::format("Edge {:}", edgeCount+1),
                                          chamber->readoutEles().front()->identify()));
                }
            }
        }
        ATH_MSG_INFO(__func__<<"() "<<__LINE__<<" - Sector envelope test completed.");
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::checkPortals(const Acts::GeometryContext& tgContext,
                                                 const Acts::TrackingVolume& volume) const {
        if (!volume.isAlignable()) {
            return StatusCode::SUCCESS;
        }
        std::vector<std::shared_ptr<const Acts::Surface>> portals{};
        for (const Acts::Portal& portal : volume.portals()) {
            if (portal.surface().geometryId().withBoundary(0) != volume.geometryId()) {
                continue;
            }
            portals.push_back(portal.surface().getSharedPtr());
        }
        const auto unAlignedPortals = volume.volumeBounds().orientedSurfaces(volume.localToGlobalTransform(tgContext));
        
        if (unAlignedPortals.size() != portals.size()) {
            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The size of the aligned and unaligned portals don't match for volume "
                <<volume.volumeName()<<". Aligned: "<<portals.size()<<", unaligned: "<<unAlignedPortals.size());
            return StatusCode::FAILURE;
        }
        StatusCode retCode = StatusCode::SUCCESS;
        for (std::size_t p =0 ; p < portals.size(); ++p){
            /// First check the bounds
            if (portals[p]->bounds() != unAlignedPortals[p].surface->bounds()) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The bounds of the "<<p
                            <<"-th portal  differ:\n -- aligned: "<<portals[p]->bounds()
                            <<"\n -- unaligned: "<<unAlignedPortals[p].surface->bounds());
                retCode = StatusCode::FAILURE;
            }
            const Acts::Transform3& uTrf{unAlignedPortals[p].surface->localToGlobalTransform(tgContext)};
            const Acts::Transform3& aTrf{portals[p]->localToGlobalTransform(tgContext)};

            if (!Amg::isIdentity(uTrf * aTrf.inverse())) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__
                    <<" - The unaligned and aligned portals don't end up at the same point \n"
                    <<" --   aligned: "<<Amg::toString(aTrf)<<"\n"<<" -- unaligned: "<<Amg::toString(uTrf));
                retCode = StatusCode::FAILURE;
            }
        }
        return retCode;
    }

    StatusCode MuonChamberToolTest::checkSurfaceVolumeBounds(const Acts::GeometryContext& tgContext,
                                                        const Acts::TrackingVolume& volume,
                                                        Acts::TrackingVolume::SurfaceRange surfaces,
                                                        std::vector<const Acts::Surface*>& badSurfaces,
                                                        const bool sameVolume) const {
        const Acts::Transform3 toVol = volume.globalToLocalTransform(tgContext);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check whether "<<(sameVolume ? "all" : "none")
            <<" of the "<<surfaces.size()<<" surfaces are within "<<volume.volumeName()<<"("
            <<volume.geometryId()<<") "<<volume.volumeBounds());
        StatusCode retCode{StatusCode::SUCCESS};
       
        for (const Acts::Surface& surface : surfaces) {
            if (!surface.isAlignable() && sameVolume) {
            ATH_MSG_ALWAYS(__func__<<"() "<<__LINE__<<" - Check surface "<<surface.geometryId()<<", "
                            <<surface.bounds()<<" @ "<<Amg::toString(surface.localToGlobalTransform(tgContext)));
            }
            std::vector<std::size_t> badIndices{};
            const std::vector<Amg::Vector3D> vertices = BoundsExpander::cornerPoints(tgContext, surface);
            for (const auto& [idx, globVtx] : Acts::enumerate(vertices)) {
                const Amg::Vector3D locVtx = toVol * globVtx;
                if (sameVolume != volume.volumeBounds().inside(locVtx, tolerance)) {
                    badIndices.push_back(idx);
                }
            }
            if (badIndices.empty()) {
                continue;
            }
            std::vector<Amg::Vector3D> badVertices{};
            for (const auto idx : badIndices) {
                badVertices.emplace_back(toVol * vertices.at(idx));
            }
            ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The following vertices of surface "
                <<surface.geometryId()<<"@ "<<Amg::toString(surface.localToGlobalTransform(tgContext))
                <<", "<<surface.bounds()<<" are "<<(sameVolume ? "outside" : "inside")
                <<" of the volume "<<volume.volumeName()<<", "<<volume.volumeBounds()<<"\n"<<toString(badVertices));
            retCode = StatusCode::FAILURE;
            badSurfaces.push_back(&surface);
        }
        return retCode;
    }
    StatusCode MuonChamberToolTest::checkTrackingVolume(const Acts::GeometryContext& tgContext,
                                                        const Acts::TrackingVolume& volume) const {
        /// The Muon chamber tool test only checks the muon volumes
        if (volume.geometryId().volume() == ActsTrk::detail::GeoVolIds::s_caloEnvelopeID) {
            return StatusCode::SUCCESS;
        }
        ATH_CHECK(checkPortals(tgContext, volume));

        const Acts::Transform3& toVol = volume.globalToLocalTransform(tgContext);
        std::vector<const Acts::TrackingVolume*> overlapVols{};
        std::vector<const Acts::Surface*> overlapSurfs{};
        StatusCode retCode = StatusCode::SUCCESS; 
        for (std::size_t vol = 0; vol < volume.volumes().size(); ++vol) {
            const Acts::TrackingVolume& subVolume = volume.volumes()[vol];
            const std::vector<Amg::Vector3D> vertices = BoundsExpander::cornerPoints(tgContext, subVolume);
            std::vector<Amg::Vector3D> outsideVertices{};
            for (const Amg::Vector3D& vertex : vertices) {
                const Amg::Vector3D volVertex = toVol * vertex;
                if (!volume.volumeBounds().inside(volVertex, tolerance)) {
                    outsideVertices.push_back(volVertex);
                }
            }
            if (outsideVertices.size()) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The following vertices of "
                <<subVolume.volumeName()<<" is outside of the mother volume's bounds "
                <<volume.volumeName()<<" "<<volume.volumeBounds()<<"\n "
                <<toString(outsideVertices));
                overlapVols.push_back(&subVolume);
                retCode = StatusCode::FAILURE;
            }
            /// Check whether the volume overlaps with one of its neighbours
            for (std::size_t neighbour = 0; neighbour < vol; ++neighbour) {
                const Acts::TrackingVolume& neighbourVol = volume.volumes()[neighbour];
                std::vector<Amg::Vector3D> overlapVert{};
                bool sharePortal = std::ranges::any_of(neighbourVol.portals(),[&](const Acts::Portal& portalA) {
                    return std::ranges::any_of(subVolume.portals(), [&](const Acts::Portal& portalB) {
                        return &portalA == &portalB;
                    });
                });
                for (const Amg::Vector3D& vertex : BoundsExpander::cornerPoints(tgContext, neighbourVol)) {
                    const Amg::Vector3D volVertex = subVolume.globalToLocalTransform(tgContext) * vertex;
                    if ((!sharePortal || std::ranges::none_of(vertices, [&](const Amg::Vector3D& vtx) {
                            return (vtx - vertex).mag2() < tolerance; })) && 
                        subVolume.volumeBounds().inside(volVertex, tolerance)) {
                            overlapVert.push_back(volVertex);
                    }
                }
                if (overlapVert.empty()) {
                    continue;
                }
                const auto [begin, end] = std::ranges::unique(overlapVert,
                        [](const Amg::Vector3D& a, const Amg::Vector3D& b){
                            return (a-b).mag2() < Acts::square(0.1_mm);
                        });
                overlapVert.erase(begin, end);
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The following vertices of "
                    <<neighbourVol.volumeName()<<" overlap with "<<subVolume.volumeName()
                    <<", "<<subVolume.volumeBounds()<<"\n"<<toString(overlapVert));
                retCode = StatusCode::FAILURE;
                overlapVols.push_back(&subVolume);
                overlapVols.push_back(&neighbourVol);
            }
        }

        // Next check whether the surfaces are all fully contained
        if(auto surfCheck =  checkSurfaceVolumeBounds(tgContext, volume, volume.surfaces(), overlapSurfs, true); 
           !surfCheck.isSuccess()) {
            retCode = StatusCode::FAILURE;
        }
        for (const Acts::TrackingVolume& subVolume : volume.volumes()) {
            if(auto surfCheck = checkSurfaceVolumeBounds(tgContext, subVolume, volume.surfaces(), 
                                                         overlapSurfs ,false); !surfCheck.isSuccess()) {
                overlapVols.push_back(&subVolume);
                retCode = StatusCode::FAILURE;
            }
            if (!checkTrackingVolume(tgContext, subVolume).isSuccess()) {
                retCode = StatusCode::FAILURE;
            }
        }
        if (retCode == StatusCode::FAILURE && (overlapSurfs.size()  || overlapVols.size())) {
            removeDuplicates(overlapVols);
            std::vector<const Acts::Volume*> subVolumes{overlapVols.begin(), overlapVols.end()};
            for (const Acts::TrackingVolume* vol : overlapVols) {
                if (!vol->isAlignable()) {
                    continue;
                }
                std::ranges::copy_if(extractSurfaces(*vol), std::back_inserter(overlapSurfs),
                                     [](const Acts::Surface* surf) { 
                                        return surf->type() == Acts::Surface::SurfaceType::Plane; 
                                    });
                std::vector<const Acts::Volume*> child = chamberVolumes(*vol);
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add "<<child.size()<<" children of "
                                <<vol->volumeName());
                std::ranges::move(child, std::back_inserter(subVolumes));
            }
            removeDuplicates(overlapSurfs);
            removeDuplicates(subVolumes);
            std::string objName =  std::format("InvalidTrkVol_{:}", 
                                                cleanVolumeName(volume.volumeName()));
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Save invalid tracking volumes in "<<objName<<".obj");
            saveEnvelope(tgContext, objName,
                         volume, overlapSurfs, subVolumes);
        } else if (retCode == StatusCode::SUCCESS && 
                   m_dumpObjs && (!volume.isAlignable() ||
                   MuonGMR4::highestAlignable(&volume) == &volume)) {
            saveEnvelope(tgContext, std::format("GoodTrkVol_{:}", cleanVolumeName(volume.volumeName())),
                                                volume, extractSurfaces(volume),
                                                chamberVolumes(volume));
        }
        return retCode;
    }
    void MuonChamberToolTest::saveEnvelope(const Acts::GeometryContext& tgContext,
                                           const std::string& envName,
                                           const Acts::Volume& envelopeVol,
                                           const std::vector<const Acts::Surface*>& assocSurfaces,
                                           const std::vector<const Acts::Volume*>& subVols) const {
        Acts::ObjVisualization3D visualHelper{};
        std::ranges::for_each(assocSurfaces, [&visualHelper, &tgContext](const Acts::Surface* surface) {
            Acts::GeometryView3D::drawSurface(visualHelper, *surface, tgContext);

        });
        std::ranges::for_each(subVols, [&visualHelper, &tgContext](const Acts::Volume* subVol) {
                Acts::GeometryView3D::drawVolume(visualHelper,*subVol, tgContext, Acts::Transform3::Identity(),
                                                  Acts::s_viewPassive);
        });
        Acts::GeometryView3D::drawVolume(visualHelper, envelopeVol, tgContext);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Save new envelope 'ChamberGeoTest_"<<envName<<".obj'");
        visualHelper.write(std::format("ChamberGeoTest_{:}.obj", envName));
    }   

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        /** Check that all chambers covered by their sector envelopes */
        ATH_CHECK(checkEnvelopes(*gctx));
        ATH_CHECK(checkChambers(*gctx));
        ATH_CHECK(checkTrackingVolume(gctx->context(), 
                 *m_trackingGeometrySvc->trackingGeometry()->highestTrackingVolume()));

        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const MdtReadoutElement& mdtMl,
                                                   const Acts::Volume& detVol) const {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Test whether "<<m_idHelperSvc->toStringDetEl(mdtMl.identify())<<std::endl<<mdtMl.getParameters());
        constexpr std::array<std::string_view, 7> tubeStr{
          "tube center", "tube readout", "tube HV", "bottom of the tube box",
          "sealing of the tube box", "wall to the previous tube", "wall to the next tube" 
        };
        enum StrIndex{center, readout, HV, bottom, sealing, prevWall, nextWall};
        for (unsigned int layer = 1; layer <= mdtMl.numLayers(); ++layer) {
            for (unsigned int tube = 1; tube <= mdtMl.numTubesInLay(); ++tube) {
                const IdentifierHash idHash = mdtMl.measurementHash(layer, tube);
                if (!mdtMl.isValid(idHash)) {
                    continue;
                }
                const Acts::Transform3& locToGlob{mdtMl.localToGlobalTransform(gctx, idHash)}; 
                const Identifier measId{mdtMl.measurementId(idHash)};

                ATH_CHECK(pointInside(gctx, detVol, mdtMl.globalTubePos(gctx, idHash), tubeStr[center], measId));

                ATH_CHECK(pointInside(gctx, detVol, mdtMl.readOutPos(gctx, idHash), tubeStr[readout], measId));
                ATH_CHECK(pointInside(gctx, detVol, mdtMl.highVoltPos(gctx, idHash), tubeStr[HV], measId));
                ATH_CHECK(pointInside(gctx, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      tubeStr[bottom], measId));
                ATH_CHECK(pointInside(gctx, detVol, locToGlob*(mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      tubeStr[sealing], measId));
                ATH_CHECK(pointInside(gctx, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
                                      tubeStr[prevWall], measId));
                ATH_CHECK(pointInside(gctx, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
                                      tubeStr[nextWall], measId));
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testReadoutSurfaces(const ActsTrk::GeometryContext& gctx,
                                                        const MuonReadoutElement& element,
                                                        const Acts::Volume& boundVol) const {
        const Acts::GeometryContext tgContext = gctx.context();
        const Acts::Transform3& invTrf{boundVol.globalToLocalTransform(tgContext)};
        for (const auto& surf : element.getSurfaces()) {
            if(element.detectorType() == ActsTrk::DetectorType::Mdt && 
               surf->type() != Acts::Surface::SurfaceType::Straw) {
               continue;
            }
            if (surf->geometryId() == Acts::GeometryIdentifier{}) {
                ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The surface "<<surf->bounds()<<"  is not part of the tracking geometry "
                            <<m_idHelperSvc->toString(identify(*surf)));
                return StatusCode::FAILURE;
            }
            for (const Amg::Vector3D& vertex : BoundsExpander::cornerPoints(tgContext, *surf)) {
                const Amg::Vector3D lVertex = invTrf * vertex;
                if (!boundVol.volumeBounds().inside(lVertex, tolerance)) {
                    ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - The vertex "<<Amg::toString(lVertex)<<" is outside of the bounds "
                        <<boundVol.volumeBounds()<<". The vertex belongs to "
                        <<m_idHelperSvc->toString(identify(*surf))<<"/"<<surf->geometryId());
                    return StatusCode::FAILURE;
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const RpcReadoutElement& rpc,                                                  
                                                   const Acts::Volume& detVol) const {
  
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Test whether "<<m_idHelperSvc->toStringDetEl(rpc.identify())<<std::endl
                        <<rpc.getParameters()<<" is contained in the envelope ");
    
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        constexpr std::string_view centreStr{"center"};
        constexpr std::string_view leftStr{"left edge"};
        constexpr std::string_view rightStr{"right edge"};
        for (unsigned int gasGap = 1 ; gasGap <= rpc.nGasGaps(); ++gasGap) {
            for (int doubletPhi = rpc.doubletPhi(); doubletPhi <= rpc.doubletPhiMax(); ++doubletPhi){
                for (bool measPhi : {false, true}) {
                    const int nStrips = measPhi ? rpc.nPhiStrips() : rpc.nEtaStrips();
                    for (int strip = 1; strip <= nStrips; ++strip) {
                        const Identifier stripId = idHelper.channelID(rpc.identify(),rpc.doubletZ(), 
                                                                      doubletPhi, gasGap, measPhi, strip);
                        ATH_CHECK(pointInside(gctx, detVol, rpc.stripPosition(gctx, stripId), centreStr, stripId));
                        ATH_CHECK(pointInside(gctx, detVol, rpc.leftStripEdge(gctx, stripId), rightStr, stripId));
                        ATH_CHECK(pointInside(gctx, detVol, rpc.rightStripEdge(gctx, stripId), leftStr, stripId));
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const TgcReadoutElement& tgc,
                                                   const Acts::Volume& detVol) const {   
        constexpr std::string_view centreStr{"center"};     
        for (unsigned int gasGap = 1; gasGap <= tgc.nGasGaps(); ++gasGap){
            for (bool isStrip : {false}) {
                const IdentifierHash layHash = tgc.constructHash(0, gasGap, isStrip);
                const unsigned int nChannel = tgc.numChannels(layHash);
                for (unsigned int channel = 1; channel <= nChannel ; ++channel) {
                    const IdentifierHash measHash = tgc.constructHash(channel, gasGap, isStrip);
                    ATH_CHECK(pointInside(gctx, detVol, tgc.channelPosition(gctx, measHash), 
                                          centreStr, tgc.measurementId(measHash)));
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const MmReadoutElement& mm,
                                                   const Acts::Volume& detVol) const {

        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        constexpr std::string_view centreStr{"center"};
        constexpr std::string_view leftStr{"left edge"};
        constexpr std::string_view rightStr{"right edge"};
        for(unsigned int gasGap = 1; gasGap <= mm.nGasGaps(); ++gasGap){
           IdentifierHash gasGapHash =  MmReadoutElement::createHash(gasGap,0);
           unsigned int firstStrip = mm.firstStrip(gasGapHash);
            for(unsigned int strip = firstStrip; strip <= mm.numStrips(gasGapHash); ++strip){
                const Identifier stripId = idHelper.channelID(mm.identify(), mm.multilayer(), gasGap, strip);
                ATH_CHECK(pointInside(gctx, detVol, mm.stripPosition(gctx, stripId), centreStr, stripId));
                ATH_CHECK(pointInside(gctx, detVol, mm.leftStripEdge(gctx, mm.measurementHash(stripId)), leftStr, stripId));
                ATH_CHECK(pointInside(gctx, detVol, mm.rightStripEdge(gctx, mm.measurementHash(stripId)), rightStr, stripId));
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const sTgcReadoutElement& stgc,
                                                   const Acts::Volume& detVol) const{
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        constexpr std::string_view posStr{"channel position"};
        for(unsigned int gasGap = 1; gasGap <= stgc.numLayers(); ++gasGap) {
           
            for(unsigned int nch = 1; nch <= stgc.nChTypes(); ++nch){                
                IdentifierHash gasGapHash = sTgcReadoutElement::createHash(gasGap, nch, 0, 0);
                const unsigned int nStrips = stgc.numChannels(gasGapHash);
                sTgcReadoutElement::ReadoutChannelType channelType = static_cast<sTgcReadoutElement::ReadoutChannelType>(nch);
                
                for(unsigned int strip = 1; strip <= nStrips; ++strip){
                    const Identifier stripId = idHelper.channelID(stgc.identify(), stgc.multilayer(), gasGap, nch, strip);
                    const IdentifierHash stripHash = stgc.measurementHash(stripId);
                    ATH_CHECK(pointInside(gctx, detVol, stgc.globalChannelPosition(gctx, stripHash), posStr, stripId));
                
                    if(channelType == sTgcReadoutElement::ReadoutChannelType::Wire || channelType == sTgcReadoutElement::ReadoutChannelType::Strip){
                        ATH_CHECK(pointInside(gctx, detVol, stgc.rightStripEdge(gctx, stripHash), posStr, stripId));
                        ATH_CHECK(pointInside(gctx, detVol, stgc.leftStripEdge(gctx, stripHash), posStr, stripId));
                    }
                }
            }            
        }
        return StatusCode::SUCCESS;

    }
}

