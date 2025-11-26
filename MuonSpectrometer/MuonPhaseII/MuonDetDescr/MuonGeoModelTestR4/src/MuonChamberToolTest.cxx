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
#include "Acts/Surfaces/TrapezoidBounds.hpp"

#include <format>
namespace{
    constexpr double tolerance = 10. *Gaudi::Units::micrometer;
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
    StatusCode MuonChamberToolTest::pointInside(const EnvelopeType& chamb,
                                                const Acts::Volume& boundVol,
                                                const Amg::Vector3D& point,
                                                const std::string& descr,
                                                const Identifier& channelId) const {

        // Explicitly inline Volume::inside here so that it gets
        // flattened in debug builds.
        Acts::Vector3 posInVolFrame((boundVol.transform().inverse()) * point);
        if (boundVol.volumeBounds().inside(posInVolFrame,tolerance)) {
            ATH_MSG_VERBOSE("In channel "<<m_idHelperSvc->toString(channelId)
                            <<", point "<<descr <<" is inside of the chamber "<<std::endl<<chamb<<std::endl
                            <<"Local position:" <<Amg::toString(boundVol.itransform() * point));
            return StatusCode::SUCCESS;
        }
        const Amg::Vector3D locPos{boundVol.itransform() * point};
        
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
                     <<", box left edge: "<<Amg::toString(planeTrapezoid.leftEdge(1).value_or(Amg::Vector2D::Zero()))
                     <<", box right edge "<<Amg::toString(planeTrapezoid.rightEdge(1).value_or(Amg::Vector2D::Zero())));
        return StatusCode::FAILURE;
    }

    StatusCode MuonChamberToolTest::pointInside(const Acts::TrackingVolume& volume,
                                                const Amg::Vector3D& point,
                                                const std::string& descr,
                                                const Identifier& chamberId) const {
        if (volume.inside(point, tolerance)) {
            return StatusCode::SUCCESS;
        }
        const auto& volumeCorners = cornerPoints(volume);
        ATH_MSG_FATAL("In channel "<<m_idHelperSvc->toString(chamberId) <<", the point "
                     << descr <<" "<<Amg::toString(volume.itransform()* point)<<" is not part of the chamber volume. The corners of the volume are:");
        for(const auto& corner : volumeCorners) {
            ATH_MSG_FATAL("  "<<Amg::toString(volume.itransform()*corner));
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

    std::array<Amg::Vector3D, 8> MuonChamberToolTest::cornerPoints(const Acts::Volume& volume) const {
        std::array<Amg::Vector3D, 8> edges{make_array<Amg::Vector3D,8>(Amg::Vector3D::Zero())};
        unsigned int edgeIdx{0};
        using BoundEnum = Acts::TrapezoidVolumeBounds::BoundValues;
        const auto& bounds = static_cast<const Acts::TrapezoidVolumeBounds&>(volume.volumeBounds());
        ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(volume.transform()));
        for (const double signX : {-1., 1.}) {
            for (const double signY : { -1., 1.}) {
                for (const double signZ: {-1., 1.}) {
                    const Amg::Vector3D edge{signX* (signY>0 ? bounds.get(BoundEnum::eHalfLengthXposY) :
                                                               bounds.get(BoundEnum::eHalfLengthXnegY)), 
                                             signY*bounds.get(BoundEnum::eHalfLengthY), 
                                             signZ*bounds.get(BoundEnum::eHalfLengthZ)};
                    edges[edgeIdx] = volume.transform() * edge;
                    ATH_MSG_VERBOSE("Local edge "<<Amg::toString(edge)<<", global edge: "<<Amg::toString(edges[edgeIdx]));
                    ++edgeIdx;
                }
            }
        }
        return edges;
    }

    std::array<Amg::Vector3D, 8> MuonChamberToolTest::cornerPoints(const Acts::GeometryContext& gctx, const Acts::StrawSurface& surface) const {
        std::array<Amg::Vector3D, 8> edges{make_array<Amg::Vector3D,8>(Amg::Vector3D::Zero())};
        using BoundEnum = Acts::LineBounds::BoundValues;
        const auto& bounds = static_cast<const Acts::LineBounds&>(surface.bounds());
        unsigned int edgeIdx{0};
        
        ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(surface.transform(gctx)));
        for (const double signX : {-1., 1.}) {
            for (const double signY : { -1., 1.}) {
                for (const double signZ: {-1., 1.}) {
                    const Amg::Vector3D edge{signX*bounds.get(BoundEnum::eR),
                                             signY*bounds.get(BoundEnum::eR),
                                             signZ*bounds.get(BoundEnum::eHalfLengthZ)};
                    edges[edgeIdx] = surface.transform(gctx) * edge;
                    ++edgeIdx;
                }
            }
        }
        
        return edges;
    }
    
    std::array<Amg::Vector3D, 4> MuonChamberToolTest::cornerPoints(const Acts::GeometryContext& gctx, const Acts::PlaneSurface& surface) const {
        std::array<Amg::Vector3D, 4> edges{make_array<Amg::Vector3D,4>(Amg::Vector3D::Zero())};
        if(surface.bounds().type() == Acts::SurfaceBounds::BoundsType::eRectangle) { //RPC surfaces are rectangles
            const Acts::RectangleBounds& bounds = static_cast<const Acts::RectangleBounds&>(surface.bounds());
            using BoundEnum = Acts::RectangleBounds::BoundValues;
            
            unsigned int edgeIdx{0};
            for(const double signX : {-1., 1.}) {
                for (const double signY : { -1., 1.}) {
                    const Amg::Vector3D edge{ signX < 0 ? bounds.get(BoundEnum::eMinX) : bounds.get(BoundEnum::eMaxX), 
                                              signY < 0 ? bounds.get(BoundEnum::eMinY) : bounds.get(BoundEnum::eMaxY), 
                                              0};
                    edges[edgeIdx] = surface.transform(gctx) * edge;
                    ++edgeIdx;  
                }
            } 
            return edges;

        } else if(surface.bounds().type() == Acts::SurfaceBounds::BoundsType::eTrapezoid) {
            using BoundEnum = Acts::TrapezoidBounds::BoundValues;
            const auto& bounds = static_cast<const Acts::TrapezoidBounds&>(surface.bounds());
            unsigned int edgeIdx{0};
        

            ATH_MSG_VERBOSE("Fetch volume bounds "<<Amg::toString(surface.transform(gctx)));
            for (const double signX : {-1., 1.}) {
                for (const double signY : { -1., 1.}) {
                        const Amg::Vector3D edge  {    
                                                Amg::getRotateZ3D(-1*bounds.get(BoundEnum::eRotationAngle))*

                                                Amg::Vector3D(signX*bounds.get(signY < 0 ? BoundEnum::eHalfLengthXnegY : BoundEnum::eHalfLengthXposY),
                                                signY*bounds.get(BoundEnum::eHalfLengthY),
                                                0)};
                    
                        edges[edgeIdx] = surface.transform(gctx) * edge;
                        ++edgeIdx;
                }
            }
        
            return edges;
        } else {
                ATH_MSG_FATAL("The surface bounds are neither a rectangle nor a trapezoid, this is not supported yet");
                return edges;
        }
        
    }


    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
        /** Check that all chambers covered by their sector envelopes */
        using SectorSet = MuonDetectorManager::MuonSectorSet;
        const SectorSet sectors = m_detMgr->getAllSectors();
        ATH_MSG_INFO("Fetched "<<sectors.size()<<" sectors. ");
        for (const SpectrometerSector* sector : sectors) {
            ATH_CHECK(allReadoutInEnvelope(*gctx, *sector));
            const std::shared_ptr<Acts::Volume> secVolume = sector->boundingVolume(*gctx);
            for (const SpectrometerSector::ChamberPtr& chamber : sector->chambers()){
                const std::array<Amg::Vector3D, 8> edges = cornerPoints(*chamber->boundingVolume(*gctx));
                unsigned int edgeCount{0};
                for (const Amg::Vector3D& edge : edges) {
                    ATH_CHECK(pointInside(*sector,*secVolume,edge, 
                              std::format("Edge {:}", edgeCount++),
                              chamber->readoutEles().front()->identify()));
                }
                const Acts::TrackingVolume* trkGeoVol = trackingGeometry->lowestTrackingVolume(gctx->context(), chamber->boundingVolume(*gctx)->center());
                ATH_MSG_DEBUG("Found "<<trkGeoVol->volumeName()<< " with " << trkGeoVol->volumes().size() <<" sub volumes for chamber "<<m_idHelperSvc->toString(chamber->readoutEles().front()->identify()));

                if(trkGeoVol->volumes().size() > 2) continue; // temporary workaround for overlaps in the tracking geometry
                
                trkGeoVol->visitVolumes([&](const Acts::TrackingVolume* volume) {
                    ATH_MSG_DEBUG("Found sub volume "<<volume->volumeName());
                });

            
                std::map<Identifier, const std::array<Amg::Vector3D, 8>> strawCorners;
                std::map<Identifier, const std::array<Amg::Vector3D, 4>> trapezoidCorners;


                trkGeoVol->visitSurfaces([&](const Acts::Surface* surface) {
                    if(surface->type() == Acts::Surface::SurfaceType::Plane){
                        const auto* planeSurface = static_cast<const Acts::PlaneSurface*>(surface);
                        ATH_MSG_DEBUG("Found plane surface  with id" << planeSurface->geometryId());
                        if(planeSurface->associatedDetectorElement()){ // skip the material surfaces
                            const ActsTrk::IDetectorElementBase* iDetElement =  static_cast<const ActsTrk::IDetectorElementBase*> (planeSurface->associatedDetectorElement());
                            trapezoidCorners.emplace(iDetElement->identify(), cornerPoints(gctx->context(), *planeSurface));
                        }
                    } else if (surface->type() == Acts::Surface::SurfaceType::Straw) {
                        const auto* strawSurface = static_cast<const Acts::StrawSurface*>(surface);
                        if(strawSurface->associatedDetectorElement() ) { 
                            const ActsTrk::IDetectorElementBase* iDetElement =  static_cast<const ActsTrk::IDetectorElementBase*> (strawSurface->associatedDetectorElement());
                            strawCorners.emplace(iDetElement->identify(), cornerPoints(gctx->context(), *strawSurface));
                        }
                    } else {

                        ATH_MSG_FATAL("Got non muon surface");
                    }

                }
                , true);

                ATH_MSG_DEBUG("Found "<<strawCorners.size()<<" straw corners and "<<trapezoidCorners.size()<<" trapezoid corners in chamber "<<m_idHelperSvc->toString(chamber->readoutEles().front()->identify()));

                for (const auto& [id, corners] : strawCorners) {
                    edgeCount = 0;
                    for (unsigned int i = 0; i < corners.size(); ++i) {
                        ATH_CHECK(pointInside(*trkGeoVol, corners[i], 
                              std::format("Straw corner {:}", i),
                              chamber->readoutEles().front()->identify()));
                    }
                }
                
                unsigned int surfaceCount{0};
                for (const auto& [id, corners] : trapezoidCorners) {
                    ATH_MSG_DEBUG("Found "<<corners.size()<<" corners for straw chamber "<<m_idHelperSvc->toString(id));
                    if(strawCorners.size() != 0){
                        break; // this is a straw chamber and I dont know were the plane surface comes from
                    }

                    edgeCount = 0;
                    for (unsigned int i = 0; i < corners.size(); ++i) {
                        ATH_CHECK(pointInside(*trkGeoVol, corners[i], 
                              std::format("Trapezoid corner {:} of surface {:}", i, ++surfaceCount),
                              id));
                    }
                }
            
        
            }           
        }
        using ChamberSet = MuonDetectorManager::MuonChamberSet;
        const ChamberSet chambers = m_detMgr->getAllChambers();
        for (const Chamber* chamber : chambers) {
            ATH_CHECK(allReadoutInEnvelope(*gctx, *chamber));
        }
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
                const Amg::Transform3D& locToGlob{mdtMl.localToGlobalTrans(gctx, idHash)}; 
                const Identifier measId{mdtMl.measurementId(idHash)};

                ATH_CHECK(pointInside(chamber, detVol, mdtMl.globalTubePos(gctx, idHash), "tube center", measId));

                ATH_CHECK(pointInside(chamber, detVol, mdtMl.readOutPos(gctx, idHash), "tube readout", measId));
                ATH_CHECK(pointInside(chamber, detVol, mdtMl.highVoltPos(gctx, idHash), "tube HV", measId));

                ATH_CHECK(pointInside(chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      "bottom of the tube box", measId));
                ATH_CHECK(pointInside(chamber, detVol, locToGlob*(mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX()), 
                                      "sealing of the tube box", measId));

                ATH_CHECK(pointInside(chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
                                      "wall to the previous tube", measId));
                ATH_CHECK(pointInside(chamber, detVol, locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY()), 
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
                        ATH_CHECK(pointInside(chamber, detVol, rpc.stripPosition(gctx, stripId), "center", stripId));
                        ATH_CHECK(pointInside(chamber, detVol, rpc.leftStripEdge(gctx, stripId), "right edge", stripId));
                        ATH_CHECK(pointInside(chamber, detVol, rpc.rightStripEdge(gctx, stripId), "left edge", stripId));
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
                    ATH_CHECK(pointInside(chamber, detVol, tgc.channelPosition(gctx, measHash), "center", tgc.measurementId(measHash)));
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
                ATH_CHECK(pointInside(chamber, detVol, mm.stripPosition(gctx, stripId), "center", stripId));
                ATH_CHECK(pointInside(chamber, detVol, mm.leftStripEdge(gctx, mm.measurementHash(stripId)), "left edge", stripId));
                ATH_CHECK(pointInside(chamber, detVol, mm.rightStripEdge(gctx, mm.measurementHash(stripId)), "right edge", stripId));
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
                     ATH_CHECK(pointInside(chamber, detVol, stgc.globalChannelPosition(gctx, stripId), "channel position", stripId));
                
                    if(channelType == sTgcReadoutElement::ReadoutChannelType::Wire || channelType == sTgcReadoutElement::ReadoutChannelType::Strip){
                        
                        ATH_CHECK(pointInside(chamber, detVol, stgc.rightStripEdge(gctx, stgc.measurementHash(stripId)), "channel position", stripId));
                        ATH_CHECK(pointInside(chamber, detVol, stgc.leftStripEdge(gctx, stgc.measurementHash(stripId)), "channel position", stripId));

                    }

                }

            }            

        }

        return StatusCode::SUCCESS;

    }
}

