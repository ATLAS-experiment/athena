/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonChamberToolTest.h"

#include <StoreGate/ReadCondHandle.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4 {
    class ChambBoundaryNote {
        public:
            ChambBoundaryNote(const MuonChamber& chamber, const ActsGeometryContext& gctx):
                m_chamber{chamber},
                m_boundVol{chamber.boundingVolume(gctx)}{}        
        
            StatusCode insideBounds(const Amg::Vector3D& locPoint, 
                                    const Identifier& pointID,
                                    std::string_view description,
                                    MsgStream& msg) {
                if (!m_locBounds->inside(locPoint)) {
                 msg<<MSG::FATAL<<"The point "<<description<<" "<<Amg::toString(locPoint,1 )<<" of channel "
                                <<m_idHelperSvc->toString(pointID)
                                <<" is outside of the chamber bounds "<<std::endl<<m_chamber.parameters()<<endmsg;
                return StatusCode::FAILURE;
                }
                m_minDyFront = std::min(std::abs(-m_chamber.halfY() - locPoint.y()), m_minDyFront);
                m_minDyBack  = std::min(std::abs(m_chamber.halfY() - locPoint.y()), m_minDyBack);
                m_minDzTop = std::min(std::abs(-m_chamber.halfZ() - locPoint.z()), m_minDzTop);
                m_minDzBottom = std::min(std::abs(m_chamber.halfZ() - locPoint.z()), m_minDzBottom);
                m_minDxFront = std::min(std::abs(m_chamber.halfXShort() - locPoint.x()), m_minDxFront);
                m_minDxBack = std::min(std::abs(m_chamber.halfXLong() - locPoint.x()), m_minDxBack);
                return StatusCode::SUCCESS;
            }
       
            const MuonChamber& m_chamber;
            std::shared_ptr<Acts::Volume> m_boundVol{nullptr};
            std::shared_ptr<Acts::Volume> m_locBounds{std::make_shared<Acts::Volume>(Amg::Transform3D::Identity(),
                                                      m_chamber.bounds())};
            const Muon::IMuonIdHelperSvc* m_idHelperSvc{m_chamber.idHelperSvc()};

            double m_minDyFront{1.e5};
            double m_minDyBack{1.e5}; 
            double m_minDzTop{1.e5}; 
            double m_minDzBottom{1.e5};
            double m_minDxFront{1.e5};
            double m_minDxBack{1.e5}; 
    };
    MuonChamberToolTest::MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_chambTool.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_detVolSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        SG::ReadCondHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
        if (!gctx.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Acts alignment "<<m_geoCtxKey.fullKey());
            return StatusCode::FAILURE;
        }

        m_detVolSvc->detector();

        const ChamberSet& chambers = m_chambTool->getAllChambers();
        std::vector<const MuonReadoutElement*> elements = m_detMgr->getAllReadoutElements();
            
        for (const MuonChamber& chamber : chambers) {
            ChambBoundaryNote boundNote{chamber, **gctx};
            for(const MuonReadoutElement* readOut : chamber.readOutElements()) {
                if (readOut->detectorType() == ActsTrk::DetectorType::Mdt) {
                    const MdtReadoutElement* mdtMl = static_cast<const MdtReadoutElement*>(readOut);
                    ATH_CHECK(testMdt(**gctx, *mdtMl, boundNote));
                } else if (readOut->detectorType() == ActsTrk::DetectorType::Rpc) {
                    const RpcReadoutElement* rpc = static_cast<const RpcReadoutElement*>(readOut);
                    ATH_CHECK(testRpc(**gctx, *rpc, boundNote));
                } else if (readOut->detectorType() == ActsTrk::DetectorType::Tgc) {
                   const TgcReadoutElement* tgc = static_cast<const TgcReadoutElement*>(readOut);
                    ATH_CHECK(testTgc(**gctx, *tgc, boundNote)); 
                } else {
                    ATH_MSG_FATAL("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                                <<" is not an Mdt, Rpc, Tgc or Mm");
                    return StatusCode::FAILURE;
                }
            }
        }
        /// Test that all readout elements have a chamber
        for (const MuonReadoutElement* readOut : elements) {
            ChamberSet::const_iterator itr = chambers.find(*readOut);
            if (itr == chambers.end()) {
                ATH_MSG_FATAL("The element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                            <<" is not attributed with any chamber");
                return StatusCode::FAILURE;
            }
        }

        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testMdt(const ActsGeometryContext& gctx,
                                            const MdtReadoutElement& mdtMl,
                                            ChambBoundaryNote& boundNote) const {

        for (unsigned int layer = 1; layer <= mdtMl.numLayers(); layer++) {
            for (unsigned int tube = 1; tube <= mdtMl.numTubesInLay(); tube++) {
                const IdentifierHash idHash = mdtMl.measurementHash(layer, tube);
                const Amg::Vector3D tubeCenter{mdtMl.globalTubePos(gctx, idHash)};
                const Amg::Vector3D tubeRO{mdtMl.readOutPos(gctx, idHash)};                            
                const Amg::Vector3D tubeHV{mdtMl.highVoltPos(gctx, idHash)}; 
                const Amg::Transform3D locToGlob{boundNote.m_chamber.globalToLocalTrans(gctx)*
                                                 mdtMl.localToGlobalTrans(gctx, idHash)};
                const Identifier tubeId = mdtMl.measurementId(idHash);
                /// Check that the generic edge points are inside the chamber
                if (!boundNote.m_boundVol->inside(tubeCenter)) {
                    ATH_MSG_FATAL(__func__<<"() "<<__LINE__<<": Tube center of "
                                <<m_idHelperSvc->toString(mdtMl.measurementId(idHash))<<
                                " is not in chamber "<<boundNote.m_chamber);
                    ATH_MSG_FATAL("Orientation in local frame "<<
                            Amg::toString(boundNote.m_chamber.globalToLocalTrans(gctx)*tubeCenter));
                    return StatusCode::FAILURE;
                }
                if (!boundNote.m_boundVol->inside(tubeRO)) {
                    ATH_MSG_FATAL(__func__<<"() "<<__LINE__<<": Tube readout of "
                                <<m_idHelperSvc->toString(mdtMl.measurementId(idHash))<<
                                " is not in chamber "<<boundNote.m_chamber);                    
                    const Amg::Vector3D locRO = boundNote.m_chamber.globalToLocalTrans(gctx)*tubeRO;
                    ATH_MSG_FATAL("Orientation in local frame "<<Amg::toString(locRO));
                    return StatusCode::FAILURE;
                }
                if (!boundNote.m_boundVol->inside(tubeHV)) {
                    ATH_MSG_FATAL(__func__<<"() "<<__LINE__<<": Tube high voltage of "
                                <<m_idHelperSvc->toString(mdtMl.measurementId(idHash))<<
                                " is not in chamber "<<boundNote.m_chamber);
                    ATH_MSG_FATAL("Orientation in local frame "<<
                            Amg::toString(boundNote.m_chamber.globalToLocalTrans(gctx)*tubeHV));
                    return StatusCode::FAILURE;
                }
                /// Check boundaries of the tube ends
                const Amg::Vector3D tubeCenDzMin = boundNote.m_chamber.globalToLocalTrans(gctx)*tubeHV;
                const Amg::Vector3D tubeCenDzPlus = boundNote.m_chamber.globalToLocalTrans(gctx)*tubeRO;
                /// Check boundaries to the next layer
                const Amg::Vector3D tubeCenDxMin = locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX());
                const Amg::Vector3D tubeCenDxPlus = locToGlob*(mdtMl.innerTubeRadius() * Amg::Vector3D::UnitX());
                /// Check boundaries to the next tube
                const Amg::Vector3D tubeCenDyMin = locToGlob*(-mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY());
                const Amg::Vector3D tubeCenDyPlus = locToGlob*(mdtMl.innerTubeRadius() * Amg::Vector3D::UnitY());
                
                ATH_CHECK(boundNote.insideBounds(tubeCenDzMin, tubeId, "HV socket", msgStream()));
                ATH_CHECK(boundNote.insideBounds(tubeCenDzPlus, tubeId, "tube readout", msgStream()));
                ATH_CHECK(boundNote.insideBounds(tubeCenDxMin, tubeId, "bottom of the tube box", msgStream()));
                ATH_CHECK(boundNote.insideBounds(tubeCenDxPlus, tubeId, "sealing of the tube box", msgStream()));
                ATH_CHECK(boundNote.insideBounds(tubeCenDyMin, tubeId, "wall to previous tube", msgStream()));
                ATH_CHECK(boundNote.insideBounds(tubeCenDyPlus, tubeId, "wall to next tube", msgStream()));
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode MuonChamberToolTest::testRpc(const ActsGeometryContext& gctx,
                                            const RpcReadoutElement& rpc,
                                            ChambBoundaryNote& boundNote) const {
        if (rpc.stationName() < 2) return StatusCode::SUCCESS;
        ATH_MSG_DEBUG("Test chamber "<<m_idHelperSvc->toStringDetEl(rpc.identify())<<std::endl<<rpc.getParameters());
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        const Amg::Transform3D& globToLoc{boundNote.m_chamber.globalToLocalTrans(gctx)};
        for (unsigned int gasGap = 1 ; gasGap <= rpc.nGasGaps(); ++gasGap) {
            for (int doubletPhi = rpc.doubletPhi(); doubletPhi <= rpc.doubletPhiMax(); ++doubletPhi){
                for (bool measPhi : {false, true}) {
                    const int nStrips = measPhi ? rpc.nPhiStrips() : rpc.nEtaStrips();
                    for (int strip = 1; strip <= nStrips; ++strip) {
                        const Identifier stripId = idHelper.channelID(rpc.identify(),rpc.doubletZ(), 
                                                                      doubletPhi, gasGap, measPhi, strip);
                        const Amg::Vector3D stripCenter = globToLoc*rpc.stripPosition(gctx, stripId);
                        const Amg::Vector3D stripLeft   = globToLoc*rpc.leftStripEdge(gctx, stripId);
                        const Amg::Vector3D stripRight  = globToLoc*rpc.rightStripEdge(gctx, stripId);
                        ATH_CHECK(boundNote.insideBounds(stripCenter, stripId, "center", msgStream()));
                        boundNote.insideBounds(stripRight, stripId, "right edge", msgStream()).ignore();
                        boundNote.insideBounds(stripLeft, stripId, "left edge", msgStream()).ignore();
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode MuonChamberToolTest::testTgc(const ActsGeometryContext& gctx,
                                            const TgcReadoutElement& tgc,
                                            ChambBoundaryNote& boundNote) const {        
        const Amg::Transform3D& globToLoc{boundNote.m_chamber.globalToLocalTrans(gctx)};
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        for (unsigned int gasGap = 1; gasGap <= tgc.nGasGaps(); ++gasGap){
            for (bool isStrip : {false, true}) {
                unsigned int nChannel = isStrip ? tgc.numStrips(gasGap) : tgc.numWireGangs(gasGap);
                for (unsigned int channel = 1; channel <= nChannel ; ++channel) {
                    const Identifier stripId = idHelper.channelID(tgc.identify(), gasGap, isStrip, channel);
                    const Amg::Vector3D stripCenter = globToLoc*tgc.channelPosition(gctx, stripId);
                     ATH_CHECK(boundNote.insideBounds(stripCenter, stripId, "center", msgStream()));
                }
            }
        }
        return StatusCode::SUCCESS;
    }

 
}

