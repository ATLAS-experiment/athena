/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonChamberToolTest.h"

#include <StoreGate/ReadCondHandle.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4 {

    MuonChamberToolTest::MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_detVolSvc.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::pointInside(const MuonChamber& chamb,
                                                const Acts::Volume& boundVol,
                                                const Amg::Vector3D& point,
                                                const std::string& descr,
                                                const Identifier& channelId) const {
    
        constexpr double tolerance = 10. *Gaudi::Units::micrometer;
        if (boundVol.inside(point,tolerance)) {
            ATH_MSG_VERBOSE("In channel "<<m_idHelperSvc->toString(channelId)
                            <<", point "<<descr <<" is inside of the chamber "<<std::endl<<chamb
                            <<std::endl
                            <<"Local position:" <<Amg::toString(boundVol.itransform() * point));
            return StatusCode::SUCCESS;
        }
        const Amg::Vector3D locPos{boundVol.itransform() * point};
        const MuonChamber::defineArgs& chambPars{chamb.parameters()};
        
        StripDesign planeTrapezoid{};
        planeTrapezoid.defineTrapezoid(chambPars.halfXShort, chambPars.halfXLong, chambPars.halfY);
        planeTrapezoid.setLevel(MSG::VERBOSE);
        /// Why does the strip design give a different result than the Acts bounds?
        static const Eigen::Rotation2D axisSwap{90. *Gaudi::Units::deg};
        if (std::abs(locPos.z()) - chambPars.halfZ < -tolerance && 
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
  

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        SG::ReadHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
        if (!gctx.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Acts alignment "<<m_geoCtxKey.fullKey());
            return StatusCode::FAILURE;
        }

        m_detVolSvc->detector();
        using ChamberSet = MuonDetectorManager::MuonChamberSet;
        const ChamberSet chambers = m_detMgr->getAllChambers();
        std::vector<const MuonReadoutElement*> elements = m_detMgr->getAllReadoutElements();
           
        for (const MuonChamber* chamber : chambers) {
            /// Create the bounting volume 
            std::shared_ptr<Acts::Volume> boundVol = chamber->boundingVolume(*gctx);
            for(const MuonReadoutElement* readOut : chamber->readOutElements()) {                
                if (readOut->detectorType() == ActsTrk::DetectorType::Tgc) {
                   const TgcReadoutElement* tgc = static_cast<const TgcReadoutElement*>(readOut);
                   ATH_CHECK(testTgc(*gctx, *tgc, *chamber, *boundVol)); 
                }else if (readOut->detectorType() == ActsTrk::DetectorType::Mdt) {
                    const MdtReadoutElement* mdtMl = static_cast<const MdtReadoutElement*>(readOut);
                    ATH_CHECK(testMdt(*gctx, *mdtMl, *chamber, *boundVol));
                } else if (readOut->detectorType() == ActsTrk::DetectorType::Rpc) {
                    const RpcReadoutElement* rpc = static_cast<const RpcReadoutElement*>(readOut);
                    ATH_CHECK(testRpc(*gctx, *rpc, *chamber, *boundVol));
                } else if (readOut->detectorType() == ActsTrk::DetectorType::Mm) {
                    const MmReadoutElement* mm = static_cast<const MmReadoutElement*>(readOut);
                    ATH_CHECK(testMm(*gctx, *mm, *chamber, *boundVol));
                } else if (readOut->detectorType() ==  ActsTrk::DetectorType::sTgc) {
                    const sTgcReadoutElement* stgc = static_cast<const sTgcReadoutElement*>(readOut);
                    ATH_CHECK(testStgc(*gctx, *stgc, *chamber, *boundVol));                    
                } else {
                    ATH_MSG_FATAL("Who brought the Cscs back? "<<m_idHelperSvc->toStringDetEl(readOut->identify()));
                    return StatusCode::FAILURE;
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testMdt(const ActsGeometryContext& gctx,
                                            const MdtReadoutElement& mdtMl,
                                            const MuonChamber& chamber,
                                            const Acts::Volume& detVol) const {

        for (unsigned int layer = 1; layer <= mdtMl.numLayers(); layer++) {
            for (unsigned int tube = 1; tube <= mdtMl.numTubesInLay(); tube++) {
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

    StatusCode MuonChamberToolTest::testRpc(const ActsGeometryContext& gctx,
                                            const RpcReadoutElement& rpc,
                                            const MuonChamber& chamber,
                                            const Acts::Volume& detVol) const {
  
        ATH_MSG_DEBUG("Test whether "<<m_idHelperSvc->toStringDetEl(rpc.identify())<<std::endl<<rpc.getParameters());
        
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
    StatusCode MuonChamberToolTest::testTgc(const ActsGeometryContext& gctx,
                                            const TgcReadoutElement& tgc,
                                            const MuonChamber& chamber,
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

    StatusCode MuonChamberToolTest::testMm(const ActsGeometryContext& gctx,
                                           const MmReadoutElement& mm,
                                           const MuonChamber& chamber,
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

    StatusCode MuonChamberToolTest::testStgc(const ActsGeometryContext& gctx,
                                            const sTgcReadoutElement& stgc,
                                            const MuonChamber& chamber,
                                            const Acts::Volume& detVol) const{
        
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        for(unsigned int gasGap = 1; gasGap <= stgc.numLayers(); ++gasGap){
           
            for(unsigned int nch = 1; nch <= stgc.nChTypes(); ++nch){                
                IdentifierHash gasGapHash = sTgcReadoutElement::createHash(gasGap, nch, 0, 0);
                unsigned int nStrips = stgc.numStrips(stgc.measurementId(gasGapHash));
                sTgcReadoutElement::ReadoutChannelType channelType = static_cast<sTgcReadoutElement::ReadoutChannelType>(nch);
                switch (channelType){
                case sTgcReadoutElement::ReadoutChannelType::Pad:
                    nStrips = stgc.numPads(stgc.measurementId(gasGapHash));                  
                    break;
                case sTgcReadoutElement::ReadoutChannelType::Wire:
                    nStrips = stgc.numWires(gasGap);                                       
                    break;                
                default: 
                                  
                    break;
                }
                
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

