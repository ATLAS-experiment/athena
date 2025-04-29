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

#include <format>
namespace{
    constexpr double tolerance = 10. *Gaudi::Units::micrometer;
}

namespace MuonGMR4 {

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
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

    template <class EnvelopeType>
    StatusCode MuonChamberToolTest::allReadoutInEnvelope(const ActsGeometryContext& gctx,
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

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
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
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsGeometryContext& gctx,
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
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsGeometryContext& gctx,
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
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsGeometryContext& gctx,
                                                   const TgcReadoutElement& tgc,
                                                   const EnevelopeType& chamber,
                                                   const Acts::Volume& detVol) const {        
      
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        for (unsigned int gasGap = 1; gasGap <= tgc.nGasGaps(); ++gasGap){
            for (bool isStrip : {false}) {
                unsigned int nChannel = isStrip ? tgc.numStrips(gasGap) : tgc.numWireGangs(gasGap);
                for (unsigned int channel = 1; channel <= nChannel ; ++channel) {
                    const Identifier stripId = idHelper.channelID(tgc.identify(), gasGap, isStrip, channel);
                    ATH_CHECK(pointInside(chamber, detVol, tgc.channelPosition(gctx, stripId), "center", stripId));
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    template <class EnevelopeType>
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsGeometryContext& gctx,
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
    StatusCode MuonChamberToolTest::testReadoutEle(const ActsGeometryContext& gctx,
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

