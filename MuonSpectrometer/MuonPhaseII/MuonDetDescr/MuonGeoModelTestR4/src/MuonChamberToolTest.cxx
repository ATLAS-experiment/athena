/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#if defined(FLATTEN) && defined(__GNUC__)
// Avoid warning in dbg build
#pragma GCC optimize "-fno-var-tracking-assignments"
#endif

#include "MuonChamberToolTest.h"

#include <StoreGate/ReadCondHandle.h>
#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <GaudiKernel/SystemOfUnits.h>

#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/DiamondVolumeBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"

#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Definitions/Units.hpp"

#include "MuonVisualizationHelpersR4/FileHelpers.h"

#include <format>

using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;

namespace{
    constexpr double tolerance = 10. *Gaudi::Units::micrometer;

    std::vector<std::shared_ptr<Acts::Volume>> chamberVolumes(const ActsTrk::GeometryContext& gctx,
                                                             const MuonGMR4::SpectrometerSector& sector) {
        std::vector<std::shared_ptr<Acts::Volume>> vols{};
        std::ranges::transform(sector.chambers(),std::back_inserter(vols), 
                              [&gctx](const auto& ch){ return ch->boundingVolume(gctx); });
        return vols;
    }
}

namespace MuonGMR4 {

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    template <class EnvelopeType>
#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
    StatusCode MuonChamberToolTest::pointInside(const ActsTrk::GeometryContext& gctx,
                                                const EnvelopeType& chamb,
                                                const Acts::Volume& boundVol,
                                                const Amg::Vector3D& point,
                                                const std::string& descr,
                                                const Identifier& channelId) const {

        if (boundVol.inside(gctx.context(), point, tolerance)) {
            ATH_MSG_VERBOSE("In channel "<<m_idHelperSvc->toString(channelId)
                            <<", point "<<descr <<" is inside of the chamber "<<std::endl<<chamb<<std::endl
                            <<"Local position:" <<Amg::toString(boundVol.globalToLocalTransform(gctx.context()) * point));
            return StatusCode::SUCCESS;
        }
        const Amg::Vector3D locPos{boundVol.globalToLocalTransform(gctx.context()) * point};
        
        StripDesign planeTrapezoid{};
        planeTrapezoid.defineTrapezoid(chamb.halfXShort(), chamb.halfXLong(), chamb.halfY());
        planeTrapezoid.setLevel(MSG::VERBOSE);
        /// Why does the strip design give a different result than the Acts bounds?
        static const Eigen::Rotation2D axisSwap{90. *Gaudi::Units::deg};
        if (std::abs(locPos.z()) - chamb.halfZ() < -tolerance && 
            planeTrapezoid.insideTrapezoid(axisSwap*locPos.block<2,1>(0,0))) {
            return StatusCode::SUCCESS;
        }
        planeTrapezoid.defineStripLayout(locPos.y() * Amg::Vector2D::UnitX(), 1, 1, 1);
        ATH_MSG_FATAL("In channel "<<m_idHelperSvc->toString(channelId) <<", the point "
                     << descr <<" "<<Amg::toString(point)<<" is not part of the chamber volume."
                     <<std::endl<<std::endl<<chamb<<std::endl<<"Local position "<<Amg::toString(locPos)
                     <<", "<<planeTrapezoid
                     <<", box left edge: "<<Amg::toString(planeTrapezoid.leftEdge(1).value_or(Amg::Vector2D::Zero()))
                     <<", box right edge "<<Amg::toString(planeTrapezoid.rightEdge(1).value_or(Amg::Vector2D::Zero())));
        return StatusCode::FAILURE;
    }

    StatusCode MuonChamberToolTest::pointInside(const ActsTrk::GeometryContext& gctx,
                                                const Acts::TrackingVolume& volume,
                                                const Amg::Vector3D& point,
                                                const std::string& descr,
                                                const Identifier& chamberId) const {
        if (volume.inside(gctx.context(), point, tolerance)) {
            return StatusCode::SUCCESS;
        }
        const std::vector<Amg::Vector3D> volumeCorners = cornerPoints(gctx, volume);
        ATH_MSG_FATAL("In channel "<<m_idHelperSvc->toString(chamberId) <<", the point "
                     << descr <<" "<<Amg::toString(volume.globalToLocalTransform(gctx.context())* point)
                     <<" is not part of the chamber volume. The corners of the volume are:");
        for(const auto& corner : volumeCorners) {
            ATH_MSG_FATAL("  "<<Amg::toString(volume.globalToLocalTransform(gctx.context())*corner));
        }
        return StatusCode::FAILURE;
    }

    template <class EnvelopeType>
    StatusCode MuonChamberToolTest::allReadoutInEnvelope(const ActsTrk::GeometryContext& gctx,
                                                         const EnvelopeType& envelope) const {
        std::shared_ptr<Acts::Volume> boundVol = envelope.boundingVolume(gctx);
        const Chamber::ReadoutSet reEles = envelope.readoutEles();
        for(const MuonReadoutElement* readOut : reEles) {
            if constexpr (std::is_same_v<EnvelopeType, SpectrometerSector>) {
                if (readOut->msSector() != &envelope) {
                    ATH_MSG_FATAL("Mismatch in the sector association "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                        <<std::endl<<(*readOut->msSector())<<std::endl<<envelope);
                    return StatusCode::FAILURE;
                }
            } else if constexpr (std::is_same_v<EnvelopeType, Chamber>) {
                if (readOut->chamber() != &envelope) {
                    ATH_MSG_FATAL("Mismatch in the chamber association "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                        <<std::endl<<(*readOut->chamber())<<std::endl<<envelope);
                    return StatusCode::FAILURE;
                }
            }
            switch (readOut->detectorType()) {
                case ActsTrk::DetectorType::Tgc: {
                    const auto* detEle = static_cast<const TgcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, envelope, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::Mdt: {
                    const auto* detEle = static_cast<const MdtReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, envelope, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::Rpc: {
                    const auto* detEle = static_cast<const RpcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, envelope, *boundVol));
                    break; 
                }  case ActsTrk::DetectorType::Mm: {
                    const auto* detEle = static_cast<const MmReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, envelope, *boundVol));
                    break; 
                } case ActsTrk::DetectorType::sTgc: {
                    const auto* detEle = static_cast<const sTgcReadoutElement*>(readOut);
                    ATH_CHECK(testReadoutEle(gctx, *detEle, envelope, *boundVol));
                    break; 
                } default: {
                    ATH_MSG_FATAL("Who came up with putting "<<ActsTrk::to_string(readOut->detectorType())
                                <<" into the MS");
                    return StatusCode::FAILURE;
                }
            }
        }
        ATH_MSG_DEBUG("All "<<reEles.size()<<" readout elements are embedded in "<<envelope);
        return StatusCode::SUCCESS;
    }

    std::vector<Amg::Vector3D> MuonChamberToolTest::cornerPoints(const ActsTrk::GeometryContext& gctx, const Acts::Volume& volume) const {
        
        const auto& bounds = volume.volumeBounds();
        unsigned int edgeIdx{0};
        //diamond volume bounds case - there are 12 edges
        if(bounds.type() == Acts::VolumeBounds::BoundsType::eDiamond){
            const auto& diamondBounds = static_cast<const Acts::DiamondVolumeBounds&>(bounds);
            using BoundEnum = Acts::DiamondVolumeBounds::BoundValues;        
            std::vector<Amg::Vector3D> edges(12, Amg::Vector3D::Zero());
            double xCord{0};
            double yCord{0};
            for(double signX : {-1.,1.}){
                for(double signY : {-1., 0., 1.}){
                    for(double signZ : {-1.,1.}){
                        if(signY == 0){
                            xCord = diamondBounds.get(BoundEnum::eHalfLengthX2);
                        }else if(signY < 0){
                            xCord = diamondBounds.get(BoundEnum::eHalfLengthX1);
                            yCord = diamondBounds.get(BoundEnum::eLengthY1);

                        }else{
                            xCord = diamondBounds.get(BoundEnum::eHalfLengthX3);
                            yCord = diamondBounds.get(BoundEnum::eLengthY2);

                        }

                        const Amg::Vector3D edge{signX*xCord, 
                                                signY*yCord, 
                                                signZ*diamondBounds.get(BoundEnum::eHalfLengthZ)};
                        edges[edgeIdx] = volume.localToGlobalTransform(gctx.context())*edge;
                        edgeIdx++;
                    }
                }
            }

            return edges;
        }

        //trapezoid or rectangular bounds case
        std::vector<Amg::Vector3D> edges(8, Amg::Vector3D::Zero());
        ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(volume.localToGlobalTransform(gctx.context())));
        for (const double signX : {-1., 1.}) {
            for (const double signY : { -1., 1.}) {
                for (const double signZ: {-1., 1.}) {
                    const Amg::Vector3D edge{signX* (signY>0 ? MuonGMR4::halfXhighY(bounds) : MuonGMR4::halfXlowY(bounds)), 
                                             signY*MuonGMR4::halfY(bounds), 
                                             signZ*MuonGMR4::halfZ(bounds)};
                    edges[edgeIdx] = volume.localToGlobalTransform(gctx.context()) * edge;
                    ATH_MSG_VERBOSE("Local edge "<<Amg::toString(edge)<<", global edge: "<<Amg::toString(edges[edgeIdx]));
                    ++edgeIdx;
                }
            }
        }
        return edges;
    }

    std::array<Amg::Vector3D, 8> MuonChamberToolTest::cornerPoints(const ActsTrk::GeometryContext& gctx, const Acts::StrawSurface& surface) const {
        std::array<Amg::Vector3D, 8> edges{make_array<Amg::Vector3D,8>(Amg::Vector3D::Zero())};
        using BoundEnum = Acts::LineBounds::BoundValues;
        const auto& bounds = static_cast<const Acts::LineBounds&>(surface.bounds());
        unsigned int edgeIdx{0};
        
        ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(surface.localToGlobalTransform(gctx.context())));
        for (const double signX : {-1., 1.}) {
            for (const double signY : { -1., 1.}) {
                for (const double signZ: {-1., 1.}) {
                    const Amg::Vector3D edge{signX*bounds.get(BoundEnum::eR),
                                             signY*bounds.get(BoundEnum::eR),
                                             signZ*bounds.get(BoundEnum::eHalfLengthZ)};
                    edges[edgeIdx] = surface.localToGlobalTransform(gctx.context()) * edge;
                    ++edgeIdx;
                }
            }
        }
        return edges;
    }
    
    std::array<Amg::Vector3D, 4> MuonChamberToolTest::cornerPoints(const ActsTrk::GeometryContext& gctx, const Acts::PlaneSurface& surface) const {
        std::array<Amg::Vector3D, 4> edges{make_array<Amg::Vector3D,4>(Amg::Vector3D::Zero())};
        if(surface.bounds().type() == Acts::SurfaceBounds::BoundsType::eRectangle) { //RPC surfaces are rectangles
            const Acts::RectangleBounds& bounds = static_cast<const Acts::RectangleBounds&>(surface.bounds());
            using BoundEnum = Acts::RectangleBounds::BoundValues;
            
            unsigned int edgeIdx{0};
            for(const double signX : {-1., 1.}) {
                for (const double signY : { -1., 1.}) {
                    const Amg::Vector3D edge{signX < 0 ? bounds.get(BoundEnum::eMinX) : bounds.get(BoundEnum::eMaxX), 
                                             signY < 0 ? bounds.get(BoundEnum::eMinY) : bounds.get(BoundEnum::eMaxY), 0.};
                    edges[edgeIdx] = surface.localToGlobalTransform(gctx.context()) * edge;
                    ++edgeIdx;  
                }
            } 
            return edges;
        } else if(surface.bounds().type() == Acts::SurfaceBounds::BoundsType::eTrapezoid) {
            using BoundEnum = Acts::TrapezoidBounds::BoundValues;
            const auto& bounds = static_cast<const Acts::TrapezoidBounds&>(surface.bounds());
            unsigned int edgeIdx{0};

            ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(surface.localToGlobalTransform(gctx.context())));
            for (const double signX : {-1., 1.}) {
                for (const double signY : { -1., 1.}) {
                        const Amg::Vector3D edge{Amg::getRotateZ3D(-1.*bounds.get(BoundEnum::eRotationAngle)) * /// Account for the stereo angle of the micromega geometry
                                                 Amg::Vector3D(signX*bounds.get(signY < 0 ? BoundEnum::eHalfLengthXnegY : BoundEnum::eHalfLengthXposY),
                                                               signY*bounds.get(BoundEnum::eHalfLengthY), 0.)};
                    
                        edges[edgeIdx] = surface.localToGlobalTransform(gctx.context()) * edge;
                        ++edgeIdx;
                }
            }
        
            return edges;
        } else {
                ATH_MSG_FATAL("The surface bounds are neither a rectangle nor a trapezoid, this is not supported yet");
                return edges;
        }
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

        for (unsigned edge1 = 1; edge1 < chamberEdges.size(); ++edge1) {
            for (unsigned edge2 = 0; edge2 < edge1; ++edge2) {
                for (unsigned step = 0 ; step <= m_overlapSamples; ++step) {
                    const double section = stepLength * step;
                    const Amg::Vector3D testPoint = section* chamberEdges[edge1] + (1. -section) *chamberEdges[edge2];
                    if (volume.inside (gctx.context(), testPoint)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    StatusCode MuonChamberToolTest::checkChambers(const ActsTrk::GeometryContext& gctx) const {

        std::vector<const MuonReadoutElement*> allRE = m_detMgr->getAllReadoutElements();
        using ChamberSet = MuonDetectorManager::MuonChamberSet;
        const ChamberSet chambers = m_detMgr->getAllChambers();
        ATH_MSG_INFO("Fetched "<<chambers.size()<<" chambers.");
        std::vector<const Chamber*> chamberVec{chambers.begin(), chambers.end()};
        
        const auto missChamb = std::ranges::find_if(allRE, [&chamberVec](const MuonGMR4::MuonReadoutElement* re){
            return std::ranges::find(chamberVec, re->chamber()) == chamberVec.end();
        });
        if (missChamb != allRE.end()) {
            ATH_MSG_FATAL("The chamber "<<(*(*missChamb)->chamber())<<" is not in the chamber set");
            return StatusCode::FAILURE;
        }

        std::set<const Chamber*> overlapChambers{};
        std::stringstream overlapstream{};
        for (std::size_t chIdx = 0; chIdx< chamberVec.size(); ++chIdx) {
            const Chamber& chamber{*chamberVec[chIdx]};
            if (m_dumpObjs) {
                saveEnvelope(gctx, std::format("Chamber_{:}{:}{:}{:}{:}", 
                                                ActsTrk::to_string(chamber.detectorType()),
                                                chName(chamber.chamberIndex()),
                                                Acts::abs(chamber.stationEta()),
                                                chamber.stationEta() > 0 ? 'A' : 'C',
                                                chamber.stationPhi()), 
                            *chamber.boundingVolume(gctx), chamber.readoutEles());
            }
            ATH_CHECK(allReadoutInEnvelope(gctx, chamber));
            const std::vector<Amg::Vector3D> chambCorners = cornerPoints(gctx, *chamber.boundingVolume(gctx));
            /// Check the overlap with other chambers
            std::vector<const Chamber*> overlaps{};
            for (std::size_t chIdx1 = 0; chIdx1<chamberVec.size(); ++chIdx1) {
                if (chIdx == chIdx1) {
                    continue;
                }
                const Chamber* overlapTest{chamberVec[chIdx1]};
                if (hasOverlap(gctx, chambCorners, *(overlapTest->boundingVolume(gctx)))) {
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
                Acts::GeometryView3D::drawVolume(visualHelper, *hasOverlap->boundingVolume(gctx), gctx.context());
                visualHelper.write(m_overlapChambObj.value());
            }
            if (m_ignoreOverlapCh) {
                ATH_MSG_WARNING(overlapstream.str());
            } else {
                ATH_MSG_FATAL(overlapstream.str());
            }
        }
        return overlapChambers.empty() || m_ignoreOverlapCh ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

    StatusCode MuonChamberToolTest::checkEnvelopes(const ActsTrk::GeometryContext& gctx) const {
 
        std::vector<const MuonReadoutElement*> allREs = m_detMgr->getAllReadoutElements();
        for (const MuonReadoutElement* re : allREs) {
            if (!re->msSector()) {
                ATH_MSG_FATAL("The readout element "<<m_idHelperSvc->toStringDetEl(re->identify())<<" does not have any sector associated ");
                return StatusCode::FAILURE;
            }
            const SpectrometerSector* sectorFromDet = m_detMgr->getSectorEnvelope(re->chamberIndex(), 
                                                                                  m_idHelperSvc->sector(re->identify()),
                                                                                  re->stationEta());
           if (sectorFromDet != re->msSector()) {
                ATH_MSG_FATAL("The sector attached to "<<m_idHelperSvc->toStringDetEl(re->identify())
                          <<", chIdx: "<<chName(re->chamberIndex())<<", sector: "<<m_idHelperSvc->sector(re->identify())
                          <<" is not the one attached to the readout geometry \n"<<(*re->msSector())<<"\n"<<(*sectorFromDet));
                return StatusCode::FAILURE;
           }
        }
        using SectorSet = MuonDetectorManager::MuonSectorSet;
        const SectorSet sectors = m_detMgr->getAllSectors();
        ATH_MSG_INFO("Fetched "<<sectors.size()<<" sectors. ");
        for (const SpectrometerSector* sector : sectors) {
            if (m_dumpObjs) {
                saveEnvelope(gctx, std::format("Sector_{:}{:}{:}",
                                               chName(sector->chamberIndex()),
                                               sector->side()  >0? 'A' :'C', 
                                               sector->stationPhi()  ), 
                            *sector->boundingVolume(gctx), sector->readoutEles(),
                             chamberVolumes(gctx, *sector));
            }
            ATH_CHECK(allReadoutInEnvelope(gctx, *sector));
            const std::shared_ptr<Acts::Volume> secVolume = sector->boundingVolume(gctx);
            for (const SpectrometerSector::ChamberPtr& chamber : sector->chambers()){
                const std::vector<Amg::Vector3D> edges = cornerPoints(gctx, *chamber->boundingVolume(gctx));
                unsigned int edgeCount{0};
                for (const Amg::Vector3D& edge : edges) {
                    ATH_CHECK(pointInside(gctx, *sector, *secVolume, edge, std::format("Edge {:}", ++edgeCount),
                                          chamber->readoutEles().front()->identify()));
                }
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonChamberToolTest::checkTrackingGeometry(const ActsTrk::GeometryContext& gctx,
                                                          std::shared_ptr<const Acts::TrackingGeometry>& trackingGeometry) const {
        //visit the volumes and check the overlaps
        std::vector<const Acts::TrackingVolume*> volumeVec{};
        std::vector<const Acts::TrackingVolume*> overlapVolumes{};
        //keep onyl the chamber volumes - not the cylinders
        trackingGeometry->visitVolumes([&](const Acts::TrackingVolume* vol){
            if(!(vol->volumeBounds().type() == Acts::VolumeBounds::BoundsType::eCylinder)){
                //dump the tracking volumes into obj 
                if(m_dumpObjs){
                    Acts::ObjVisualization3D visualHelper{};   
                    for(const auto& surf : vol->surfaces()){                
                   
                        Acts::GeometryView3D::drawSurface(visualHelper, surf, gctx.context());
                         
                    }
                    std::string volName = vol->volumeName();
                    Acts::GeometryView3D::drawVolume(visualHelper, *vol, gctx.context());
                    ATH_MSG_DEBUG("Save new tracking volume 'MsTrackingVol_"<<volName<<".obj'");
                    visualHelper.write(std::format("MsTrackingVol_{:}.obj", volName));

                }
                volumeVec.push_back(vol);            
            }
        });

        for(std::size_t vIdx = 0; vIdx < volumeVec.size(); vIdx++){
            const Acts::TrackingVolume* testVol{volumeVec[vIdx]};
            std::vector<const Acts::TrackingVolume*> overlaps{};
            const std::vector<Amg::Vector3D> edges = cornerPoints(gctx, *testVol);
            const auto childrenVol = testVol->volumes();
            const auto innerSurfaces = testVol->surfaces();
            
            for(const auto& surface : innerSurfaces){
                //only plane or straw surfaces expected
                std::vector<Amg::Vector3D> surfEdges = {};
                if(const auto* strawSurf = dynamic_cast<const Acts::StrawSurface*>(&surface)){
                   ATH_MSG_VERBOSE("Checking straw surface "<<surface.geometryId()
                   <<" in volume "<<testVol->volumeName());
                   
                  auto edges = cornerPoints(gctx, *strawSurf);
                  surfEdges = {edges.begin(), edges.end()};
                }else if(const auto* planeSurf = dynamic_cast<const Acts::PlaneSurface*>(&surface)){ 
                    ATH_MSG_VERBOSE("Checking plane surface "<<surface.geometryId()
                    <<" in volume "<<testVol->volumeName());
                    
                    auto edges = cornerPoints(gctx, *planeSurf);
                    surfEdges = {edges.begin(), edges.end()};
                } else{
                    ATH_MSG_FATAL("The surface "<< surface.geometryId() 
                    <<" is neither a straw nor a plane surface");
                    return StatusCode::FAILURE;
                }
        
               
                for(const auto& edge : surfEdges){
                    if(!(testVol->inside(gctx.context(), edge,0.01))){
                        ATH_MSG_FATAL("The surface " <<  surface.geometryId() <<" at vertex point " <<Amg::toString(edge)
                        <<" is outside the parent volume" << testVol->volumeName());
                        return m_ignoreOutsideSurf ?  StatusCode::SUCCESS : StatusCode::FAILURE;

                    }
                }
            }

            //check if this test volume overlaps with the other volumes in the tracking geometry
            for(std::size_t vIdx1 = 0 ; vIdx1 < volumeVec.size(); vIdx1++){
            bool isChild = std::ranges::find_if(childrenVol,
                [&](const Acts::TrackingVolume& tv){ return &tv == volumeVec[vIdx1]; }
            ) != childrenVol.end();

            bool isMother = (testVol->motherVolume() == volumeVec[vIdx1]);

            if(vIdx1 == vIdx || isMother){
                    continue;
            }
             //check if the child volume is entirely enclosed by the mother volume
            if(isChild){
                std::vector<Amg::Vector3D> childEdges = cornerPoints(gctx,*volumeVec[vIdx1]);
                for(const auto& edge : childEdges){
                    if(!(testVol->inside(gctx.context(), edge, 0.01))){
                        ATH_MSG_FATAL("The children volume's " << volumeVec[vIdx1]->volumeName() 
                        <<" vertex point " <<Amg::toString(edge)
                        <<" is outside the parent volume" << testVol->volumeName());
                        return StatusCode::FAILURE;
                    }
                }
                continue;

            }
            if(hasOverlap(gctx, edges, *volumeVec[vIdx1])){
                overlaps.push_back(volumeVec[vIdx1]);
            }
        }
        if(overlaps.empty()){
            ATH_MSG_DEBUG("No overlaps detected for the volume "<<testVol->volumeName());
            continue;
        }else{
            overlapVolumes.push_back(testVol);
            ATH_MSG_ALWAYS("The volume " << testVol->volumeName() << " overlaps with: ");
            for(const auto& overlap: overlaps){
                    ATH_MSG_ALWAYS(" Volume: " << overlap->volumeName());
            }
        }
    }
    if(overlapVolumes.empty()){
        ATH_MSG_ALWAYS("No overlaps detected in the tracking geometry!!");
    }
    return overlapVolumes.empty() || m_ignoreOverlapCh ? StatusCode::SUCCESS : StatusCode::FAILURE;    
    }
    
    void MuonChamberToolTest::saveEnvelope(const ActsTrk::GeometryContext& gctx,
                                           const std::string& envName,
                                           const Acts::Volume& envelopeVol,
                                           const std::vector<const MuonGMR4::MuonReadoutElement*>& assocRE,
                                           const std::vector<std::shared_ptr<Acts::Volume>>& subVols) const {
        Acts::ObjVisualization3D visualHelper{};
        std::ranges::for_each(assocRE, [&visualHelper, &gctx](const MuonReadoutElement* re){
            std::ranges::for_each(re->getSurfaces(),[&visualHelper, &gctx](const std::shared_ptr<Acts::Surface>& surface){
                                        Acts::GeometryView3D::drawSurface(visualHelper, *surface, gctx.context());
                                 });
        });
        std::ranges::for_each(subVols, [&visualHelper, &gctx](const std::shared_ptr<Acts::Volume>& subVol ){
                Acts::GeometryView3D::drawVolume(visualHelper,*subVol, gctx.context(), Amg::Transform3D::Identity(),
                                                  Acts::s_viewPassive);
        });
        Acts::GeometryView3D::drawVolume(visualHelper, envelopeVol, gctx.context());
        ATH_MSG_DEBUG("Save new envelope 'MsTrackTest_"<<envName<<".obj'");
        visualHelper.write(std::format("MsTrackTest_{:}.obj", envName));
    }   

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
        /** Check that all chambers covered by their sector envelopes */
        ATH_CHECK(checkChambers(*gctx));
        ATH_CHECK(checkEnvelopes(*gctx));
        ATH_CHECK(checkTrackingGeometry(*gctx, trackingGeometry));
 
        return StatusCode::SUCCESS;
    }
    template <class EnvelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const MdtReadoutElement& mdtMl,
                                                   const EnvelopeType& chamber,
                                                   const Acts::Volume& detVol) const {
        ATH_MSG_VERBOSE("Test whether "<<m_idHelperSvc->toStringDetEl(mdtMl.identify())<<std::endl<<mdtMl.getParameters());

        for (unsigned int layer = 1; layer <= mdtMl.numLayers(); ++layer) {
            for (unsigned int tube = 1; tube <= mdtMl.numTubesInLay(); ++tube) {
                const IdentifierHash idHash = mdtMl.measurementHash(layer, tube);
                if (!mdtMl.isValid(idHash)){
                    continue;
                }
                const Amg::Transform3D& locToGlob{mdtMl.localToGlobalTransform(gctx, idHash)}; 
                const Identifier measId{mdtMl.measurementId(idHash)};

                ATH_CHECK(pointInside(gctx, chamber, detVol, mdtMl.globalTubePos(gctx, idHash), "tube center", measId));

                ATH_CHECK(pointInside(gctx, chamber, detVol, mdtMl.readOutPos(gctx, idHash), "tube readout", measId));
                ATH_CHECK(pointInside(gctx, chamber, detVol, mdtMl.highVoltPos(gctx, idHash), "tube HV", measId));

                ATH_CHECK(pointInside(gctx, chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      "bottom of the tube box", measId));
                ATH_CHECK(pointInside(gctx, chamber, detVol, locToGlob*(mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      "sealing of the tube box", measId));

                ATH_CHECK(pointInside(gctx, chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
                                      "wall to the previous tube", measId));
                ATH_CHECK(pointInside(gctx, chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
                                      "wall to the next tube", measId));
            }
        }
        return StatusCode::SUCCESS;
    }
    template<class EnvelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const RpcReadoutElement& rpc,
                                                   const EnvelopeType& chamber,
                                                   const Acts::Volume& detVol) const {
  
        ATH_MSG_VERBOSE("Test whether "<<m_idHelperSvc->toStringDetEl(rpc.identify())<<std::endl<<rpc.getParameters());
    
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        for (unsigned int gasGap = 1 ; gasGap <= rpc.nGasGaps(); ++gasGap) {
            for (int doubletPhi = rpc.doubletPhi(); doubletPhi <= rpc.doubletPhiMax(); ++doubletPhi){
                for (bool measPhi : {false, true}) {
                    const int nStrips = measPhi ? rpc.nPhiStrips() : rpc.nEtaStrips();
                    for (int strip = 1; strip <= nStrips; ++strip) {
                        const Identifier stripId = idHelper.channelID(rpc.identify(),rpc.doubletZ(), 
                                                                      doubletPhi, gasGap, measPhi, strip);
                        ATH_CHECK(pointInside(gctx, chamber, detVol, rpc.stripPosition(gctx, stripId), "center", stripId));
                        ATH_CHECK(pointInside(gctx, chamber, detVol, rpc.leftStripEdge(gctx, stripId), "right edge", stripId));
                        ATH_CHECK(pointInside(gctx, chamber, detVol, rpc.rightStripEdge(gctx, stripId), "left edge", stripId));
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    template <class EnevelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const TgcReadoutElement& tgc,
                                                   const EnevelopeType& chamber,
                                                   const Acts::Volume& detVol) const {        
        for (unsigned int gasGap = 1; gasGap <= tgc.nGasGaps(); ++gasGap){
            for (bool isStrip : {false}) {
                const IdentifierHash layHash = tgc.constructHash(0, gasGap, isStrip);
                const unsigned int nChannel = tgc.numChannels(layHash);
                for (unsigned int channel = 1; channel <= nChannel ; ++channel) {
                    const IdentifierHash measHash = tgc.constructHash(channel, gasGap, isStrip);
                    ATH_CHECK(pointInside(gctx, chamber, detVol, tgc.channelPosition(gctx, measHash), 
                                          "center", tgc.measurementId(measHash)));
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    template <class EnevelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const MmReadoutElement& mm,
                                                   const EnevelopeType& chamber,
                                                   const Acts::Volume& detVol) const {

        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        for(unsigned int gasGap = 1; gasGap <= mm.nGasGaps(); ++gasGap){
           IdentifierHash gasGapHash =  MmReadoutElement::createHash(gasGap,0);
           unsigned int firstStrip = mm.firstStrip(gasGapHash);
            for(unsigned int strip = firstStrip; strip <= mm.numStrips(gasGapHash); ++strip){
                const Identifier stripId = idHelper.channelID(mm.identify(), mm.multilayer(), gasGap, strip);
                ATH_CHECK(pointInside(gctx, chamber, detVol, mm.stripPosition(gctx, stripId), "center", stripId));
                ATH_CHECK(pointInside(gctx, chamber, detVol, mm.leftStripEdge(gctx, mm.measurementHash(stripId)), "left edge", stripId));
                ATH_CHECK(pointInside(gctx, chamber, detVol, mm.rightStripEdge(gctx, mm.measurementHash(stripId)), "right edge", stripId));
            }
        }

        return StatusCode::SUCCESS;
    }
    template <class EnvelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                                   const sTgcReadoutElement& stgc,
                                                   const EnvelopeType& chamber,
                                                   const Acts::Volume& detVol) const{
        
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        for(unsigned int gasGap = 1; gasGap <= stgc.numLayers(); ++gasGap){
           
            for(unsigned int nch = 1; nch <= stgc.nChTypes(); ++nch){                
                IdentifierHash gasGapHash = sTgcReadoutElement::createHash(gasGap, nch, 0, 0);
                const unsigned int nStrips = stgc.numChannels(gasGapHash);
                sTgcReadoutElement::ReadoutChannelType channelType = static_cast<sTgcReadoutElement::ReadoutChannelType>(nch);
                
                for(unsigned int strip = 1; strip <= nStrips; ++strip){
                    const Identifier stripId = idHelper.channelID(stgc.identify(), stgc.multilayer(), gasGap, nch, strip);
                    const IdentifierHash stripHash = stgc.measurementHash(stripId);
                    ATH_CHECK(pointInside(gctx, chamber, detVol, stgc.globalChannelPosition(gctx, stripHash), "channel position", stripId));
                
                    if(channelType == sTgcReadoutElement::ReadoutChannelType::Wire || channelType == sTgcReadoutElement::ReadoutChannelType::Strip){
                        ATH_CHECK(pointInside(gctx, chamber, detVol, stgc.rightStripEdge(gctx, stripHash), "channel position", stripId));
                        ATH_CHECK(pointInside(gctx, chamber, detVol, stgc.leftStripEdge(gctx, stripHash), "channel position", stripId));
                    }
                }
            }            
        }
        return StatusCode::SUCCESS;

    }
}

