
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeomCnvAlg.h"

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>

#include <MuonAlignmentDataR4/MdtAlignmentStore.h>

#include <MuonReadoutGeometry/MuonStation.h>
#include <MuonReadoutGeometry/MdtReadoutElement.h>
#include <MuonReadoutGeometry/RpcReadoutElement.h>

#include <AthenaKernel/IOVInfiniteRange.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <GeoModelHelpers/defineWorld.h>
#include <GeoModelHelpers/cloneVolume.h>
#include <GeoModelHelpers/getChildNodesWithTrf.h>
#include <GeoModelHelpers/TransformToStringConverter.h>
#include <GeoModelHelpers/GeoShapeUtils.h>
#include <map>

#include <GaudiKernel/SystemOfUnits.h>

namespace {
    Amg::Transform3D readOutToStation(const GeoVFullPhysVol*  readOutVol) {
        return  readOutVol->getAbsoluteTransform().inverse() *
                readOutVol->getParent()->getX();
    }
}

MuonReadoutGeomCnvAlg::MuonReadoutGeomCnvAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm{name, pSvcLocator} {}

StatusCode MuonReadoutGeomCnvAlg::initialize()  {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_alignStoreKeys.initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}


StatusCode MuonReadoutGeomCnvAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<MuonGM::MuonDetectorManager> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("The current readout geometry is still valid.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
    /// Prepare the Geometry context
    ActsGeometryContext geoContext{};
    using TrackingAlignment = ActsTrk::DetectorAlignStore::TrackingAlignStore;
    for (const SG::ReadCondHandleKey<ActsTrk::DetectorAlignStore>& key : m_alignStoreKeys) {
        SG::ReadCondHandle<ActsTrk::DetectorAlignStore> readHandle{key, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve alignment store "<<key.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        auto alignStore = std::make_unique<ActsTrk::DetectorAlignStore>(**readHandle);
        /// Ensure that the position & tracking alignment caches are split from the conditions object
        alignStore->geoModelAlignment->clearPosCache();
        alignStore->trackingAlignment = std::make_unique<TrackingAlignment>(alignStore->detType);
        geoContext.setStore(std::move(alignStore));
    }
    /// Check that for every detector technology there's an DetectorAlignStore in the geometry context
    /// Otherwise create an empty one.
    std::vector<ActsTrk::DetectorType> presentTechs = m_detMgr->getDetectorTypes();
    for (const ActsTrk::DetectorType detType : presentTechs) {
        if (geoContext.getStore(detType)) {
            continue;
        }
        ATH_MSG_WARNING("No external detector alignment has been defined for technology "<<ActsTrk::to_string(detType));
        geoContext.setStore(std::make_unique<ActsTrk::DetectorAlignStore>(detType));
    }

    std::unique_ptr<MuonGM::MuonDetectorManager> detMgr = std::make_unique<MuonGM::MuonDetectorManager>();
    PVLink world{createGeoWorld()};
    detMgr->addTreeTop(world);
    ATH_CHECK(buildMdt(geoContext, detMgr.get(), world));
    ATH_CHECK(buildRpc(geoContext, detMgr.get(), world));

    ATH_CHECK(writeHandle.record(std::move(detMgr)));
    return StatusCode::SUCCESS;
}
StatusCode MuonReadoutGeomCnvAlg::buildStation(const ActsGeometryContext& gctx,
                                               MuonGM::MuonDetectorManager& mgr,
                                               const Identifier& stationId,
                                               PVLink world) const {
    const std::string stName{m_idHelperSvc->stationNameString(stationId)};
    const int stEta{m_idHelperSvc->stationEta(stationId)};
    const int stPhi{m_idHelperSvc->stationPhi(stationId)};
    MuonGM::MuonStation* station = mgr.getMuonStation(stName, stEta, stPhi);
    if (station) {
        ATH_MSG_DEBUG("Station "<<stName<<" "<<stEta<<" "<<stPhi<<" already exists.");
        return StatusCode::SUCCESS;
    }
    /// Fetch the readout element to get its parent volume
    const MuonGMR4::MuonReadoutElement* copyMe = m_detMgr->getReadoutElement(stationId);
    /// Retrieve the full phyiscal volume
    const GeoVFullPhysVol* readOutVol = copyMe->getMaterialGeom();
    PVConstLink parentVolume = readOutVol->getParent();
    /// Construct the aligned station transformation 
    const Amg::Transform3D stationTransform =  copyMe->localToGlobalTrans(gctx) *
                                                readOutToStation(readOutVol);
    
    /// Copy the full physical volume of the muon station
    PVLink parentPhysVol{make_intrusive<GeoFullPhysVol>(parentVolume->getLogVol())};    
    /// Make sure to copy all the children from the original tree that're not FullPhysVols -> represent
    /// They represent the passive material inside the station and are needed for the TrackinGeometry building
    const std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(parentVolume, false);
    double minX{1.e9}, maxX{-1.e9}, minY1{1.e9}, maxY1{-1.e9}, minY2{1.e9}, maxY2{-1.e9}, minZ{1.e9}, maxZ{-1.e9};
    for (const GeoChildNodeWithTrf& child : children) {
        GeoVPhysVol* childVol = const_pointer_cast<GeoVPhysVol>(child.volume);
        std::vector<Amg::Vector3D> edges = getPolyShapeEdges(childVol->getLogVol()->getShape(),
                                                             child.transform);
        for (const Amg::Vector3D& edge : edges) {
            minX = std::min(minX, edge.x());
            maxX = std::max(maxX, edge.x());
            minZ = std::min(minZ, edge.z());
            maxZ = std::max(maxZ, edge.z());
            if (edge.z() < 0) {
                minY1 = std::min(minY1, edge.y());
                maxY1 = std::max(maxY1, edge.y());
            } else {
                minY2 = std::min(minY2, edge.y());
                maxY2 = std::max(maxY2, edge.y());
            }
        }

        /// Skip the full physical volumes as they represent the readout elements
        if (typeid(*childVol) == typeid(GeoFullPhysVol)) {
            continue;
        }
        world->add(make_intrusive<GeoTransform>(child.transform));
        world->add(cloneVolume(childVol));
    }
    /// Add the physical volume to the world
    world->add(make_intrusive<GeoTransform>(stationTransform));
    world->add(parentPhysVol);
    /// To create the muon station, we need to extract the dimensions
    ///  --> Recieve the edge points from the shapes
    const double shortS = (maxY1 - minY1);
    const double longS  = (maxY2 - minY2);
    const double lengthR = (maxX - minX);
    const double lengthZ = (maxZ - minZ);
    auto newStation = std::make_unique<MuonGM::MuonStation>(stName,
                                                            shortS, lengthR, lengthZ, /// S / R / Z size
                                                            longS, lengthR, lengthZ,  /// S / R / Z size (long)
                                                            stEta, stPhi, false);
    newStation->setPhysVol(parentPhysVol);
    mgr.addMuonStation(std::move(newStation));

    return StatusCode::SUCCESS;
}

StatusCode MuonReadoutGeomCnvAlg::buildRpc(const ActsGeometryContext& gctx,
                                           MuonGM::MuonDetectorManager* mgr,
                                           PVLink world) const {
    
    const std::vector<const MuonGMR4::RpcReadoutElement*> readoutEles = m_detMgr->getAllRpcReadoutElements();
    ATH_MSG_DEBUG("Going to build "<<readoutEles.size()<<" Rpc readout elements.");
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    for (const MuonGMR4::RpcReadoutElement* copyMe : readoutEles) {
        const Identifier reId = copyMe->identify();
        const MuonGMR4::RpcReadoutElement::parameterBook& pars{copyMe->getParameters()};
        /// Build the mother station if it's not already existing
        ATH_CHECK(buildStation(gctx, *mgr, reId, world));

        const std::string stName{m_idHelperSvc->stationNameString(reId)};
        MuonGM::MuonStation* station = mgr->getMuonStation(stName, 
                                                           m_idHelperSvc->stationEta(reId), 
                                                           m_idHelperSvc->stationPhi(reId));
        
        PVLink parentPhysVol{station->getPhysVol()};
        GeoIntrusivePtr<const GeoVFullPhysVol> readOutVol{copyMe->getMaterialGeom()};
        parentPhysVol->add(make_intrusive<GeoTransform>(readOutToStation(readOutVol).inverse()));
        PVLink clonedVol{cloneVolume(const_pointer_cast<GeoVFullPhysVol>(readOutVol))};
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{dynamic_pointer_cast<GeoVFullPhysVol>(clonedVol)};
        parentPhysVol->add(physVol);

        std::unique_ptr<MuonGM::RpcReadoutElement> newElement = std::make_unique<MuonGM::RpcReadoutElement>(physVol, stName, 1, 1, false, mgr);
        newElement->setDoubletPhi(copyMe->doubletPhi());
        newElement->setDoubletR(copyMe->doubletR());
        newElement->setDoubletZ(copyMe->doubletZ());
        newElement->setIdentifier(reId);
        newElement->setParentMuonStation(station);

        /// Define the dimensions
        newElement->setLongRsize(pars.halfLength);
        newElement->setLongSsize(pars.halfWidth);
        newElement->setLongZsize(pars.halfThickness);
        newElement->setRsize(pars.halfLength);
        newElement->setSsize(pars.halfWidth);
        newElement->setZsize(pars.halfThickness);

        newElement->m_nlayers = copyMe->nGasGaps();
        newElement->m_phistripwidth = copyMe->stripPhiWidth();
        newElement->m_etastripwidth = copyMe->stripEtaWidth();
        newElement->m_phistrippitch = copyMe->stripPhiPitch();
        newElement->m_etastrippitch = copyMe->stripEtaPitch();
        newElement->m_phistriplength = copyMe->stripPhiLength();
        newElement->m_etastriplength = copyMe->stripEtaLength();

        newElement->m_nphistripsperpanel = copyMe->nPhiStrips();
        newElement->m_netastripsperpanel = copyMe->nEtaStrips();
        newElement->m_nphistrippanels = copyMe->nPhiPanels();
        newElement->m_hasDEDontop = true;
         for (unsigned int gasGap = 1; gasGap <= copyMe->nGasGaps(); ++gasGap) {
            for (int doubPhi = copyMe->doubletPhi(); doubPhi <= copyMe->doubletPhiMax(); ++doubPhi) {
                for (bool measPhi : {false, true}) {
                    const Identifier gapId = idHelper.channelID(copyMe->identify(), 
                                                                copyMe->doubletZ(), 
                                                                doubPhi, gasGap, measPhi, 1);

                    const Amg::Vector3D locStripPos = copyMe->globalToLocalTrans(gctx) * copyMe->stripPosition(gctx, gapId);
                    ATH_MSG_VERBOSE("GasGap "<<m_idHelperSvc->toString(gapId)<<", local strip position: "<<Amg::toString(locStripPos));
                    newElement->m_gasGap_xPos[gasGap -1] = locStripPos.x();
                    if (measPhi) {
                        newElement->m_first_phistrip_s[doubPhi -1] = locStripPos.y();
                        newElement->m_phistrip_z = locStripPos.z();
                    } else{
                        newElement->m_first_etastrip_z = locStripPos.z();
                        newElement->m_etastrip_s[doubPhi-1] = locStripPos.y();
                    }
                }
            }
         }

        newElement->m_mirrored = true;
        newElement->fillCache();
        newElement->m_mirrored = false;
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newElement));
        mgr->addRpcReadoutElement(std::move(newElement));
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                                 const MuonGMR4::RpcReadoutElement& refEle,
                                                 const MuonGM::RpcReadoutElement& testEle) const {
    if (!msgLvl(MSG::VERBOSE)) {
        return StatusCode::SUCCESS;
    }
    ATH_MSG_VERBOSE("Compare basic readout transforms"<<std::endl
                <<GeoTrf::toString(testEle.absTransform(),true)<<std::endl
                <<GeoTrf::toString(refEle.localToGlobalTrans(gctx), true));
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    for (unsigned int gasGap = 1; gasGap <= refEle.nGasGaps(); ++gasGap) {
        for (int doubPhi = refEle.doubletPhi(); doubPhi <= refEle.doubletPhiMax(); ++doubPhi) {
            for (bool measPhi : {false, true}) {
                for (int strip = 1; strip <= testEle.Nstrips(measPhi); ++strip) {
                    const Identifier stripId = idHelper.channelID(refEle.identify(), 
                                                                refEle.doubletZ(), 
                                                                doubPhi, gasGap, measPhi, strip);
                    
                    const Amg::Vector3D refStripPos = refEle.stripPosition(gctx, stripId);
                    const Amg::Vector3D testStripPos = testEle.stripPos(stripId);
                    if ((refStripPos - testStripPos).mag() > std::numeric_limits<float>::epsilon()){
                        ATH_MSG_ERROR("Mismatch in strip positions "<<m_idHelperSvc->toString(stripId)
                                <<" ref: "<<Amg::toString(refStripPos)<<" test: "<<Amg::toString(testStripPos)
                                <<" local coordinates -- ref: "<<Amg::toString(testEle.absTransform().inverse()*refStripPos)
                                <<"test: "<<Amg::toString(testEle.absTransform().inverse()*testStripPos));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_VERBOSE("Agreement between new and old geometry for channel "<<m_idHelperSvc->toString(stripId)
                                    <<" strip position "<<Amg::toString(refStripPos));
                    const Amg::Transform3D& refTrans{refEle.localToGlobalTrans(gctx, stripId)};
                    const Amg::Transform3D& testTrans{testEle.transform(stripId)};
                    if (Amg::doesNotDeform(refTrans.inverse()*testTrans)) continue;
                    if (strip == 1) {
                        ATH_MSG_VERBOSE("Transformation for "<<m_idHelperSvc->toString(stripId)<<std::endl
                            <<" *** ref:  "<<GeoTrf::toString(refTrans, true)<<std::endl
                            <<" *** test: "<<GeoTrf::toString(testTrans, true));
                    }  
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode MuonReadoutGeomCnvAlg::buildMdt(const ActsGeometryContext& gctx,
                                           MuonGM::MuonDetectorManager* mgr,
                                           PVLink world) const {    
    /// Access the B-Line and As-built parameters
    using SubDetAlignment = ActsGeometryContext::AlignmentStorePtr;
    SubDetAlignment alignItr = gctx.getStore(ActsTrk::DetectorType::Mdt);
    const MdtAlignmentStore* alignStore = alignItr ?
                                 static_cast<const MdtAlignmentStore*>(alignItr->internalAlignment.get()) : nullptr;

    const std::vector<const MuonGMR4::MdtReadoutElement*> mdtReadOuts{m_detMgr->getAllMdtReadoutElements()};
    ATH_MSG_INFO("Copy "<<mdtReadOuts.size()<<" Mdt readout elements to the legacy system");
    for (const MuonGMR4::MdtReadoutElement* copyMe : mdtReadOuts) {
        const Identifier reId = copyMe->identify();
        /// Build the mother station
        ATH_CHECK(buildStation(gctx, *mgr, reId, world));

        const std::string stName{m_idHelperSvc->stationNameString(reId)};
        MuonGM::MuonStation* station = mgr->getMuonStation(stName, m_idHelperSvc->stationEta(reId), m_idHelperSvc->stationPhi(reId));
        
        // cppcheck-suppress invalidLifetime; ok: mgr took ownership.
        PVLink parentPhysVol{station->getPhysVol()};
        GeoIntrusivePtr<const GeoVFullPhysVol> readOutVol{copyMe->getMaterialGeom()};
        PVLink clonedVol{cloneVolume(const_pointer_cast<GeoVFullPhysVol>(readOutVol))};
        GeoIntrusivePtr<GeoFullPhysVol> physVol{dynamic_pointer_cast<GeoFullPhysVol>(clonedVol)};
        parentPhysVol->add(make_intrusive<GeoTransform>(readOutToStation(readOutVol).inverse()));
        parentPhysVol->add(physVol);

        const MuonGMR4::MdtReadoutElement::parameterBook& pars{copyMe->getParameters()};

        std::unique_ptr<MuonGM::MdtReadoutElement> newElement = std::make_unique<MuonGM::MdtReadoutElement>(physVol, stName, mgr);
        newElement->setIdentifier(reId);
        // cppcheck-suppress invalidLifetime; ok: mgr took ownership.
        newElement->setParentMuonStation(station);
        /// Define the dimensions
        newElement->setLongRsize(2*pars.halfY);
        /// 1 cm is added as safety margin to the Mdt multilayer envelope
        newElement->setLongSsize(2*pars.longHalfX - 1.*Gaudi::Units::cm);
        newElement->setLongZsize(2*pars.halfHeight);
        newElement->setRsize(2*pars.halfY);
        newElement->setSsize(2*pars.shortHalfX - 1.*Gaudi::Units::cm);
        newElement->setZsize(2*pars.halfHeight);

        newElement->m_nlayers = copyMe->numLayers();
        newElement->m_ntubesperlayer = copyMe->numTubesInLay();
        newElement->m_deadlength = pars.deadLength;
        newElement->m_endpluglength = pars.endPlugLength;
        newElement->m_innerRadius = pars.tubeInnerRad;
        newElement->m_tubeWallThickness = pars.tubeWall;
        newElement->m_tubepitch = pars.tubePitch;
        /// Need to check how to obtain this parameter from the new geometry
        /// newElement->m_cutoutShift;

        /// Determine the tube length's 
        const MuonGMR4::MdtTubeLayer& tubeLay{*pars.tubeLayers[0]};
        unsigned int step{1};
        double lastLength{2.*tubeLay.tubeHalfLength(1)}; 
        for (unsigned tube = 0; tube < copyMe->numTubesInLay(); ++tube) {
            const double currLength = 2.*tubeLay.tubeHalfLength(tube);
            if (std::abs(lastLength - currLength) > std::numeric_limits<float>::epsilon() ||
                tube == copyMe->numTubesInLay() -1) {
                newElement->m_tubelength[step-1] = lastLength;
                newElement->m_tubelength[step] = currLength;                
                if (step == 1) {
                    newElement->m_ntubesinastep = tube;
                }
                lastLength = currLength;
                ++step;
            }
        }
        newElement->m_nsteps = step;
        
        /// Define the tube staggering
        const Amg::Transform3D globToLoc{copyMe->globalToLocalTrans(gctx)};
        double xOffSet{pars.halfY}, yOffSet{pars.halfHeight};
        if (newElement->barrel())  std::swap(xOffSet, yOffSet);
        for (unsigned lay = 1; lay <= copyMe->numLayers(); ++lay) {
            const IdentifierHash tubeHash{copyMe->measurementHash(lay, 1)};
            const Amg::Vector3D locTube = globToLoc * copyMe->globalTubePos(gctx, tubeHash);
            newElement->m_firstwire_x[lay-1] = locTube.z() + xOffSet;
            newElement->m_firstwire_y[lay-1] = locTube.x() + yOffSet;
        }
        MdtAlignmentStore::chamberDistortions distort = alignStore ? alignStore->getDistortion(reId) : 
                                                        MdtAlignmentStore::chamberDistortions{};
        
        newElement->setBLinePar(distort.bLine);
        station->setMdtAsBuiltParams(distort.asBuilt);
        newElement->geoInitDone();
        newElement->fillCache();
        /// Add the readout element to the manager
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newElement));
        mgr->addMdtReadoutElement(std::move(newElement));
    }
    return StatusCode::SUCCESS;
}

StatusCode MuonReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                                 const MuonGMR4::MdtReadoutElement& refEle,
                                                 const MuonGM::MdtReadoutElement& testEle) const {
    if (!msgLvl(MSG::VERBOSE)) {
        return StatusCode::SUCCESS;
    }
    
    ATH_MSG_VERBOSE("Detector element "<<m_idHelperSvc->toString(refEle.identify())
                <<std::endl<<GeoTrf::toString(refEle.localToGlobalTrans(gctx))                        
                <<std::endl<<GeoTrf::toString(testEle.getMaterialGeom()->getAbsoluteTransform())
                <<std::endl<<"r-size: "<<testEle.getRsize()<<"/"<<testEle.getLongRsize()
                            <<" s-size: "<<testEle.getSsize()<<"/"<<testEle.getLongSsize()
                            <<" z-size: "<<testEle.getZsize()<<"/"<<testEle.getLongZsize());
    for (unsigned int lay = 1; lay <= refEle.numLayers(); ++lay){
        for (unsigned int tube = 1; tube <= refEle.numTubesInLay(); ++tube) {
            const IdentifierHash tubeHash {refEle.measurementHash(lay,tube)};
            if (!refEle.isValid(tubeHash)) {
                ATH_MSG_VERBOSE("SKip layer / tube "<<lay <<","<<tube);
                continue;
            }
            const Amg::Transform3D globToLocal = refEle.globalToLocalTrans(gctx, tubeHash);

            const Amg::Vector3D refPos = refEle.globalTubePos(gctx, tubeHash);
            const Amg::Vector3D tubePos = testEle.tubePos(lay, tube);

            
            if ( (refPos - tubePos).mag() > Gaudi::Units::micrometer &&
                 (globToLocal*refPos - globToLocal * tubePos).perp() > Gaudi::Units::micrometer) {
                ATH_MSG_ERROR("Tube positions differ for "<<m_idHelperSvc->toString(refEle.measurementId(tubeHash))
                            <<" reference: "<<GeoTrf::toString(refPos)<<" vs. test: "
                            <<GeoTrf::toString(tubePos) <<" delta: "<<(refPos - tubePos).mag());
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Tube positions layer: "<<lay<<", tube: "<<tube
                <<std::endl<<"reference: "<<GeoTrf::toString(refPos)
                <<std::endl<<"test:      "<<GeoTrf::toString(tubePos)
                <<std::endl<<testEle.tubeLength(lay, tube)<<"/"
                            <<testEle.getActiveTubeLength(lay, tube)<<"/"
                            <<testEle.getWireLength(lay,tube)
                            <<" vs. "<<refEle.tubeLength(tubeHash)<<"/"<<refEle.activeTubeLength(tubeHash)
                            <<"/"<<refEle.wireLength(tubeHash));
            if (std::abs(testEle.tubeLength(lay,tube) - refEle.tubeLength(tubeHash)) >
                std::numeric_limits<float>::epsilon() ) {
                ATH_MSG_WARNING("Different tube length's detected for "<<m_idHelperSvc->toStringDetEl(refEle.identify())
                                << " layer: "<<lay<<", tube: "<<tube<<" "<<testEle.tubeLength(lay,tube)<<" (new) vs. "
                                <<refEle.tubeLength(tubeHash)<<" (ref)");
            }
        }
    }

    return StatusCode::SUCCESS;
}
