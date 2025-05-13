
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "ReadoutGeomCnvAlg.h"

#include <GeoModelKernel/GeoVPhysVol.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>


#include <MuonReadoutGeometryR4/SpectrometerSector.h>

#include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#include <MuonAlignmentDataR4/sTgcAlignmentStore.h>
#include <MuonAlignmentDataR4/MmAlignmentStore.h>


#include <MuonReadoutGeometry/MuonStation.h>
#include <MuonReadoutGeometry/MdtReadoutElement.h>
#include <MuonReadoutGeometry/RpcReadoutElement.h>
#include <MuonReadoutGeometry/sTgcReadoutElement.h>
#include <MuonReadoutGeometry/MMReadoutElement.h>
#include <MuonReadoutGeometry/TgcReadoutElement.h>

#include <MuonReadoutGeometry/MuonChannelDesign.h>

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
    using SubDetAlignment = ActsGeometryContext::AlignmentStorePtr;
    bool  hasStationVolume(const PVConstLink treeTop,
                           const std::set<PVConstLink>& translated) {
        const unsigned int nCh = treeTop->getNChildVols();
        for (unsigned int ch = 0 ; ch < nCh; ++ch) {
            PVConstLink child = treeTop->getChildVol(ch);
            if (translated.count(child) ||
                hasStationVolume(child, translated)){
                return true;
            }            
        }
        return false;
    };
}

namespace MuonGMR4{

StatusCode ReadoutGeomCnvAlg::initialize()  {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_alignStoreKeys.initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}


StatusCode ReadoutGeomCnvAlg::execute(const EventContext& ctx) const {
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
        if (alignStore->geoModelAlignment) {
            alignStore->geoModelAlignment->clearPosCache();
        }
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
    /// Create the cache and populate it with the geoWolds
    ConstructionCache cacheObj;
    cacheObj.detMgr = std::make_unique<MuonGM::MuonDetectorManager>();
    cacheObj.world = createGeoWorld();
    cacheObj.detMgr->addTreeTop(cacheObj.world);
    /// Build the chamber technologies    
    ATH_CHECK(buildMdt(geoContext, cacheObj));
    ATH_CHECK(buildTgc(geoContext, cacheObj));
    ATH_CHECK(buildRpc(geoContext, cacheObj));
    ATH_CHECK(buildSTGC(geoContext, cacheObj));
    ATH_CHECK(buildMM(geoContext, cacheObj));
    /// Finally add the passives
    std::vector<GeoChildNodeWithTrf> treeTops = getChildrenWithRef(m_detMgr->getTreeTop(0), false);
  
    /// Move the passives also onto the new world tree...
    for (const GeoChildNodeWithTrf& treeTop : treeTops) {
        if (hasStationVolume(treeTop.volume, cacheObj.translatedStations)) continue;
        ATH_MSG_VERBOSE("Detected passive volume "<<treeTop.nodeName);

        cacheObj.world->add(const_pointer_cast(treeTop.volume));
    }
    
    /// Final check that all elements are cached properly
    if (m_checkGeo) {
        const std::vector<const MuonGMR4::MuonReadoutElement*> refEles{m_detMgr->getAllReadoutElements()};
        for (const MuonGMR4::MuonReadoutElement* refEle : refEles) {
            ATH_CHECK(checkIdCompability(*refEle, *cacheObj.detMgr->getReadoutElement(refEle->identify())));
        }
    }

    ATH_CHECK(writeHandle.record(std::move(cacheObj.detMgr)));
    return StatusCode::SUCCESS;
}
StatusCode ReadoutGeomCnvAlg::buildStation(const ActsGeometryContext& gctx,
                                           const Identifier& stationId,
                                           ConstructionCache& cacheObj) const {
    const std::string stName{m_idHelperSvc->stationNameString(stationId)};
    const int stEta{m_idHelperSvc->stationEta(stationId)};
    const int stPhi{m_idHelperSvc->stationPhi(stationId)};
    MuonGM::MuonStation* station = cacheObj.detMgr->getMuonStation(stName, stEta, stPhi);
    if (station) {
        ATH_MSG_DEBUG("Station "<<stName<<" "<<stEta<<" "<<stPhi<<" already exists.");
        return StatusCode::SUCCESS;
    }
    /// Fetch the readout element to get its parent volume
    const MuonGMR4::MuonReadoutElement* copyMe = m_detMgr->getReadoutElement(stationId);
    
    /// Retrieve the full phyiscal volume
    const GeoVFullPhysVol* readOutVol = copyMe->getMaterialGeom();
    PVConstLink parentVolume = readOutVol->getParent();   // This is the physical volume that contains the readOutVol as a child. This is a const link to an existing physical volume
    cacheObj.translatedStations.insert(parentVolume);
    /// Copy the full physical volume of the muon station
    PVLink parentPhysVol{make_intrusive<GeoFullPhysVol>(parentVolume->getLogVol())};  // This is a mutable pointer. This creates a new physical volume using the same logical structure (shapes and materials but not position).

    /// Make sure to copy all the children from the original tree that're not FullPhysVols -> represent
    /// They represent the passive material inside the station and are needed for the TrackinGeometry building
    const std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(parentVolume, false);    // we get the list of child nodes attached to the parent volume
    double minX{1.e9}, maxX{-1.e9}, minY1{1.e9}, maxY1{-1.e9}, minY2{1.e9}, maxY2{-1.e9}, minZ{1.e9}, maxZ{-1.e9};
    for (const GeoChildNodeWithTrf& child : children) {
        std::vector<Amg::Vector3D> edges = getPolyShapeEdges(child.volume->getLogVol()->getShape(),                     //we are getting corner points of the edges
                                                             readOutVol->getX().inverse() * child.transform);           //getX() returns the transformation from readout physical volume ref to parent physical volume ref. We take the inverse and we chain it with child.transform, which is from child ref to parent ref
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
    }
    /// To create the muon station, we need to extract the dimensions
    ///  --> Recieve the edge points from the shapes
    const double shortS = (maxY1 - minY1);
    const double longS  = (maxY2 - minY2);
    const double lengthR = (maxX - minX);
    const double lengthZ = (maxZ - minZ);
    
    const GeoAlignableTransform* alignTrf{copyMe->alignableTransform()};
    /// Transformation to reach from the alignable point to the Muon station
    const Amg::Transform3D stationTransform = alignTrf->getDefTransform().inverse()*parentVolume->getX();

    for (const GeoChildNodeWithTrf& child : children) {
        /// Skip the full physical volumes as they represent the readout elements
        const GeoVPhysVol &childVolRef = *child.volume;
        if (typeid(childVolRef) == typeid(GeoFullPhysVol)) {
            continue;
        }
        // Add the beam lines / foams inside the station volume
        PVLink childVol = const_pointer_cast<GeoVPhysVol>(child.volume);
        parentPhysVol->add(cacheObj.newIdTag());
        parentPhysVol->add(cacheObj.makeTransform(stationTransform*child.transform));
        parentPhysVol->add(cloneVolume(childVol));
    }
    /// Fetch the transform of the detector element which is AlignableNode x Station x [relative pos in station]
    const Amg::Transform3D alignedTransform = copyMe->localToGlobalTrans(gctx) *
                                              (stationTransform * readOutVol->getX()).inverse();

    ATH_MSG_VERBOSE("stName "<<stName<<","<<stEta<<","<<stPhi<<" -- shortS: "<<shortS<<", longS: "<<longS
                <<", lengthR: "<<lengthR<<", lengthZ "<<lengthZ
                <<std::endl<<"AlignableNode: "<<GeoTrf::toString(alignedTransform, true)
                <<std::endl<<"Station transform: "<<GeoTrf::toString(stationTransform, true)
                <<std::endl<<"Readout transform: "<<GeoTrf::toString(readOutVol->getX(), true));
    auto newStation = std::make_unique<MuonGM::MuonStation>(stName,
                                                            shortS, lengthR, lengthZ, /// S / R / Z size
                                                            longS, lengthR, lengthZ,  /// S / R / Z size (long)
                                                            stEta, stPhi, false);
    newStation->setPhysVol(parentPhysVol);
    /// Add the physical volume to the world
    cacheObj.world->add(cacheObj.newIdTag());
    GeoIntrusivePtr<GeoAlignableTransform> trf = make_intrusive<GeoAlignableTransform>(alignedTransform);
    newStation->setTransform(trf);
    
    newStation->setNominalAmdbLRSToGlobal( trf->getTransform()); 
    
    cacheObj.detMgr->addMuonStation(std::move(newStation));

    cacheObj.world->add(trf);   
    cacheObj.world->add(parentPhysVol);

    return StatusCode::SUCCESS;
}


StatusCode ReadoutGeomCnvAlg::cloneReadoutVolume(const ActsGeometryContext& gctx,
                                                 const Identifier& reId,
                                                 ConstructionCache& cacheObj,
                                                 GeoIntrusivePtr<GeoVFullPhysVol>& physVol,
                                                 MuonGM::MuonStation* & station) const {
    
    ATH_CHECK(buildStation(gctx, reId, cacheObj));
    const std::string stName{m_idHelperSvc->stationNameString(reId)};
    station = cacheObj.detMgr->getMuonStation(stName, 
                                              m_idHelperSvc->stationEta(reId), 
                                              m_idHelperSvc->stationPhi(reId));
  
    PVLink parentPhysVol{station->getPhysVol()};
    const MuonGMR4::MuonReadoutElement* copyMe = m_detMgr->getReadoutElement(reId);
    GeoIntrusivePtr<const GeoVFullPhysVol> readOutVol{copyMe->getMaterialGeom()};
    parentPhysVol->add(cacheObj.newIdTag());
    /// This is a hack to include the BIL Rpcs into the translation. Recall that the BI-RPCs break the station paradigm
    /// Hence, in the new description they have their own alignable transform. However, the legacy geometry tries to sort
    /// them into the corresponding Mdt station with the same stName, stEta, stPhi. So quite a lot of gymnastics is now needed
    /// to place them accordingly into that frame. 
    ///             getX(Mdt) * delta(Mdt) * X  = getX(Rpc) * delta(Rpc)
    ///                                      X  = [getX(Mdt) * delta(Mdt)]^{-1} * getX(Rpc) * delta(Rpc) 
    /// The second term is the transform of the transform of the Muon station. The second one needs to be rewritten
    ///             Y = getX(Rpc) * delta(Rpc)  * getX(alignNode -> readoutFrame)
    ///             X = Y * getX(alignNode -> readoutFrame)^{-1}
    ///              
    const Amg::Transform3D alignNodeToRE{copyMe->alignableTransform()->getDefTransform().inverse() *
                                         readOutVol->getParent()->getX() * readOutVol->getX()};
    const Amg::Transform3D alignedNode{copyMe->localToGlobalTrans(gctx) * alignNodeToRE.inverse()};
    
    const Amg::Transform3D stationTrf{station->getTransform().inverse() * alignedNode};

    parentPhysVol->add(cacheObj.makeTransform(stationTrf*alignNodeToRE));
    /// Clone the detector element with all of its subvolumes
    PVLink clonedVol{cloneVolume(const_pointer_cast<GeoVFullPhysVol>(readOutVol))};
    physVol = dynamic_pointer_cast<GeoVFullPhysVol>(clonedVol);
    parentPhysVol->add(physVol);
    return StatusCode::SUCCESS;
}

StatusCode ReadoutGeomCnvAlg::buildRpc(const ActsGeometryContext& gctx, ConstructionCache& cacheObj) const {
    
    const std::vector<const MuonGMR4::RpcReadoutElement*> readoutEles = m_detMgr->getAllRpcReadoutElements();
    ATH_MSG_INFO("Copy "<<readoutEles.size()<<" Rpc readout elements to the legacy system");
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    for (const MuonGMR4::RpcReadoutElement* copyMe : readoutEles) {
        const Identifier reId = copyMe->identify();
        const MuonGMR4::RpcReadoutElement::parameterBook& pars{copyMe->getParameters()};
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{};
        MuonGM::MuonStation* station{nullptr};
        ATH_CHECK(cloneReadoutVolume(gctx, reId, cacheObj, physVol, station));
        auto newElement = std::make_unique<MuonGM::RpcReadoutElement>(physVol, 
                                                                      m_idHelperSvc->stationNameString(reId), 
                                                                      1, 1, false, cacheObj.detMgr.get());
        const bool aSide{copyMe->stationEta() > 0};
        newElement->setDoubletPhi(copyMe->doubletPhi());
        newElement->setDoubletR(copyMe->doubletR());
        newElement->setDoubletZ(copyMe->doubletZ());
        newElement->setIdentifier(reId);
        newElement->setParentMuonStation(station);
        station->addMuonReadoutElementWithAlTransf(newElement.get(), nullptr, station->nMuonReadoutElements());

        /// Define the dimensions
        newElement->setLongZsize(2.*pars.halfLength);
        newElement->setLongSsize(2.*pars.halfWidth);
        newElement->setLongRsize(2.*pars.halfThickness);
        newElement->setZsize(2.*pars.halfLength);
        newElement->setSsize(2.*pars.halfWidth);
        newElement->setRsize(2.*pars.halfThickness);

        newElement->m_nlayers = copyMe->nGasGaps();
        newElement->m_phistripwidth = copyMe->stripPhiWidth();
        newElement->m_etastripwidth = copyMe->stripEtaWidth();
        newElement->m_phistrippitch = copyMe->stripPhiPitch();
        newElement->m_etastrippitch =  (aSide > 0 ? 1. : -1.) *copyMe->stripEtaPitch();
        newElement->m_phistriplength = copyMe->stripPhiLength();
        newElement->m_etastriplength = copyMe->stripEtaLength();

        newElement->m_nphistripsperpanel = copyMe->nPhiStrips();
        newElement->m_netastripsperpanel = copyMe->nEtaStrips();
        newElement->m_nphistrippanels = copyMe->nPhiPanels();
        newElement->m_hasDEDontop = true;
        newElement->m_descratzneg = false;

        std::vector<Identifier> gapIds{};
        for (unsigned int gasGap = 1; gasGap <= copyMe->nGasGaps(); ++gasGap) {
            for (int doubPhi = copyMe->doubletPhiMax(); doubPhi >= copyMe->doubletPhi(); --doubPhi) {
                for (bool measPhi : {false, true}) {
                    if (measPhi && copyMe->nPhiStrips()==0) continue;
                    const int channel = 1;
                    const Identifier gapId = idHelper.channelID(copyMe->identify(), 
                                                                copyMe->doubletZ(), 
                                                                doubPhi, gasGap, measPhi, 
                                                                channel);

                    gapIds.push_back(gapId);
                    const Amg::Vector3D locStripPos = copyMe->globalToLocalTrans(gctx) * copyMe->stripPosition(gctx, gapId);
                    ATH_MSG_VERBOSE("GasGap "<<m_idHelperSvc->toString(gapId)<<", local strip position: "<<Amg::toString(locStripPos));
                    newElement->m_gasGap_xPos[gasGap -1] = locStripPos.x();
                    /// Hack to assign the proper strip positions  for REs having doubletPhi =2
                    /// in their Identifier
                    const int dbPIdx = copyMe->doubletPhi() == 2 ? 1 : doubPhi;
                    if (measPhi) {
                        newElement->m_first_phistrip_s[dbPIdx -1] = locStripPos.y();
                        newElement->m_phistrip_z = locStripPos.z();
                    } else{
                        newElement->m_first_etastrip_z = locStripPos.z();
                        newElement->m_etastrip_s[dbPIdx-1] = locStripPos.y();
                    }
                }
            }
        }
        newElement->fillCache();
        /// Resignment in face of the rpc readout geometry. Overwrite all the parameters
        for (const Identifier& gapId : gapIds) {
            const int surfaceHash = newElement->surfaceHash(gapId);
            const int layerHash = newElement->layerHash(gapId);
            const Amg::Transform3D& refTrf{copyMe->localToGlobalTrans(gctx, gapId)};
            newElement->m_surfaceData->m_layerTransforms[surfaceHash] = refTrf;
            newElement->m_surfaceData->m_layerCenters[layerHash] = refTrf.translation();
            newElement->m_surfaceData->m_layerNormals[layerHash] = refTrf.linear() * Amg::Vector3D::UnitZ();
        }       
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newElement));
        cacheObj.detMgr->addRpcReadoutElement(std::move(newElement));
    }
    return StatusCode::SUCCESS;
}


StatusCode ReadoutGeomCnvAlg::buildTgc(const ActsGeometryContext& gctx, ConstructionCache& cacheObj) const {

    std::vector<const MuonGMR4::TgcReadoutElement*> tgcReadouts{m_detMgr->getAllTgcReadoutElements()};
    std::stable_sort(tgcReadouts.begin(), tgcReadouts.end(),
                     [](const MuonGMR4::TgcReadoutElement* a, const MuonGMR4::TgcReadoutElement* b){
                            return a->stationEta() > b->stationEta();
                     });
    ATH_MSG_INFO("Copy "<<tgcReadouts.size()<<" Tgc readout elements to the legacy system");
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};    
    
    using TgcReadoutParams = MuonGM::TgcReadoutParams;
    std::map<std::string, std::shared_ptr<TgcReadoutParams>> readoutParMap{};

    for (const MuonGMR4::TgcReadoutElement* copyMe: tgcReadouts) {        
        const Identifier reId = copyMe->identify();
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{};
        MuonGM::MuonStation* station{nullptr};
        ATH_CHECK(cloneReadoutVolume(gctx,reId, cacheObj, physVol, station));

        auto newRE = std::make_unique<MuonGM::TgcReadoutElement>(physVol, m_idHelperSvc->stationNameString(reId), 
                                                                 cacheObj.detMgr.get());
        newRE->setIdentifier(reId);
        newRE->setParentMuonStation(station);
        
        std::shared_ptr<TgcReadoutParams>& readOutPars = readoutParMap[copyMe->chamberDesign()];
        if (!readOutPars) {
            using WiregangArray = TgcReadoutParams::WiregangArray;
            using StripArray = TgcReadoutParams::StripArray;
            using GasGapIntArray = TgcReadoutParams::GasGapIntArray;

            std::array<WiregangArray, 3> wires{};
            GasGapIntArray nWireGangs{}, nStrips{};
            StripArray botMountings{}, topMountings{};
            bool stripSet{false};
            double wirePitch{0.};

            for (unsigned int gasGap =1; gasGap <= copyMe->nGasGaps(); ++gasGap) {
                nWireGangs[gasGap -1] = copyMe->numWireGangs(gasGap);
                nStrips[gasGap -1] = copyMe->numStrips(gasGap);
                if (nWireGangs[gasGap -1]) {
                    const MuonGMR4::WireGroupDesign& design{copyMe->wireGangLayout(gasGap)};
                    wirePitch = design.stripPitch();
                    WiregangArray& fillMe{wires[gasGap-1]};
                    for (int gang = 1; gang <= design.numStrips(); ++gang) {
                        fillMe[gang -1] = design.numWiresInGroup(gang);
                    }
                }
                if (nStrips[gasGap -1] && !stripSet) {
                    const MuonGMR4::RadialStripDesign& design {copyMe->stripLayout(gasGap)};
                    const int nCh = nStrips[gasGap -1];
                    for (int strip = 1; strip <= nCh; ++strip) {
                        botMountings[strip-1] = - design.stripLeftBottom(strip).x();
                        topMountings[strip-1] = - design.stripLeftTop(strip).x();
                    }
                    botMountings[nCh] = - design.stripRightBottom(nCh).x();
                    topMountings[nCh] = - design.stripRightTop(nCh).x();

                    stripSet = true;
                }
            }
            readOutPars = std::make_unique<TgcReadoutParams>(copyMe->chamberDesign(),
                                                             0, 0, wirePitch,
                                                             idHelper.stationPhiMax(reId),
                                                             std::move(nWireGangs),
                                                             std::move(wires[0]), 
                                                             std::move(wires[1]), 
                                                             std::move(wires[2]),
                                                             0,
                                                             std::move(botMountings),
                                                             std::move(topMountings),
                                                             std::move(nStrips));
        }
        
        /// Define the local gasGap positions
        for (unsigned int gasGap = 1; gasGap <= copyMe->nGasGaps(); ++gasGap) {
            const IdentifierHash layHash{ copyMe->constructHash(0, gasGap, false)};
            /// In the sector frame, the gasGap is oriented along the x-axis
            const Amg::Vector3D translation{copyMe->globalToLocalTrans(gctx) * copyMe->center(gctx, layHash)};            
            newRE->setPlaneZ(translation.x(), gasGap);
        }
        newRE->setRsize(copyMe->moduleHeight());
        newRE->setSsize(copyMe->moduleWidthS());
        newRE->setZsize(copyMe->moduleThickness());

        newRE->setLongRsize(copyMe->moduleHeight());
        newRE->setLongSsize(copyMe->moduleWidthL());
        newRE->setLongZsize(copyMe->moduleThickness());

        newRE->setReadOutParams(readOutPars);
        newRE->fillCache();
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newRE));
        cacheObj.detMgr->addTgcReadoutElement(std::move(newRE));
    
    }
    return StatusCode::SUCCESS;
}

GeoIntrusivePtr<GeoVFullPhysVol> 
            ReadoutGeomCnvAlg::cloneNswWedge(const ActsGeometryContext& gctx,
                                             const MuonGMR4::MuonReadoutElement* copyMe,
                                             ConstructionCache& cacheObj) const {
    GeoIntrusivePtr<const GeoVFullPhysVol> readOutVol{copyMe->getMaterialGeom()};
    cacheObj.translatedStations.insert(readOutVol->getParent());
        
    PVLink clonedVol{cloneVolume(const_pointer_cast<GeoVFullPhysVol>(readOutVol))};
    GeoIntrusivePtr<GeoFullPhysVol> physVol{dynamic_pointer_cast<GeoFullPhysVol>(clonedVol)};
    cacheObj.world->add(cacheObj.newIdTag());
    cacheObj.world->add(cacheObj.makeTransform(copyMe->localToGlobalTrans(gctx)));
    cacheObj.world->add(physVol);
    return physVol;
}
StatusCode ReadoutGeomCnvAlg::buildMM(const ActsGeometryContext& gctx, ConstructionCache& cacheObj) const {

    SubDetAlignment alignItr = gctx.getStore(ActsTrk::DetectorType::Mm);
    const auto alignStore = alignItr ?
                            static_cast<const MmAlignmentStore*>(alignItr->internalAlignment.get()) : nullptr;

    if (alignStore) {
        cacheObj.detMgr->setMMPassivation(alignStore->passivation);
    }
    const std::vector<const MuonGMR4::MmReadoutElement*> mmReadouts{m_detMgr->getAllMmReadoutElements()};
    ATH_MSG_INFO("Copy "<<mmReadouts.size()<<" Mm readout elements to the legacy system");
    
    for (const MuonGMR4::MmReadoutElement* copyMe : mmReadouts) {
        const Identifier reId = copyMe->identify();
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{cloneNswWedge(gctx, copyMe, cacheObj)};
        auto newRE = std::make_unique<MuonGM::MMReadoutElement>(physVol, 
                                                                m_idHelperSvc->stationNameString(reId),
                                                                copyMe->stationEta(),
                                                                copyMe->stationPhi(),
                                                                copyMe->multilayer(), cacheObj.detMgr.get());
        /// Loop over the gas gaps & efine the 
        for (unsigned int gasGap = 0; gasGap < copyMe->nGasGaps(); ++gasGap) {
            const MuonGMR4::StripLayer& stripLayer{copyMe->stripLayer(MuonGMR4::MmReadoutElement::createHash(gasGap +1, 0))};
            const MuonGMR4::StripDesign& designFrom{stripLayer.design()};
            
            newRE->m_Xlg[gasGap] = stripLayer.toOrigin() * 
                                   Amg::getRotateZ3D(-designFrom.stereoAngle()) * 
                                   Amg::getRotateY3D(90. * Gaudi::Units::deg);
            ATH_MSG_VERBOSE("Layer transform "<<gasGap<<" "<<GeoTrf::toString(newRE->m_Xlg[gasGap], true));
            
            MuonGM::MuonChannelDesign& designTo{newRE->m_etaDesign[gasGap]};
            designTo.defineTrapezoid(designFrom.shortHalfHeight(),
                                     designFrom.longHalfHeight(),
                                     designFrom.halfWidth(), 
                                     designFrom.stereoAngle());
            designTo.type = MuonGM::MuonChannelDesign::ChannelType::etaStrip;
            designTo.detType = MuonGM::MuonChannelDesign::DetType::MM;
            designTo.inputPitch = designFrom.stripPitch();
            designTo.inputWidth = designTo.inputPitch * std::cos(designTo.stereoAngle());
            designTo.nMissedBottomEta  = designTo.nMissedBottomStereo = designFrom.firstStripNumber() - 1;
            designTo.totalStrips = designFrom.numStrips();
            designTo.nch = designFrom.numStrips();
            
            designTo.setFirstPos(designFrom.firstStripPos().x() + 0.5*designTo.inputPitch);
        }

        newRE->fillCache();
        if (alignStore && alignStore->getBLine(reId)) {
            newRE->setBLinePar(*alignStore->getBLine(reId));
        }
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newRE));
        cacheObj.detMgr->addMMReadoutElement(std::move(newRE));
    }
    return StatusCode::SUCCESS;
}

StatusCode  ReadoutGeomCnvAlg::buildSTGC(const ActsGeometryContext& gctx, ConstructionCache& cacheObj) const{
    SubDetAlignment alignItr = gctx.getStore(ActsTrk::DetectorType::sTgc);
    auto alignStore = alignItr ? static_cast<const sTgcAlignmentStore*>(alignItr->internalAlignment.get()) : nullptr;

    const std::vector<const MuonGMR4::sTgcReadoutElement*> sTgcReadOuts{m_detMgr->getAllsTgcReadoutElements()};
    ATH_MSG_INFO("Copy "<<sTgcReadOuts.size()<<" sTgc readout elements to the legacy system");

    for (const MuonGMR4::sTgcReadoutElement* copyMe : sTgcReadOuts) {
        const Identifier reId = copyMe->identify();
        ATH_MSG_DEBUG("Translate readout element "<<m_idHelperSvc->toStringDetEl(reId)<<".");
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{cloneNswWedge(gctx, copyMe, cacheObj)};

        auto newRE = std::make_unique<MuonGM::sTgcReadoutElement>(physVol, 
                                                                  m_idHelperSvc->stationNameString(reId).substr(1),
                                                                  copyMe->stationEta(),
                                                                  copyMe->stationPhi(),
                                                                  copyMe->multilayer(), 
                                                                  cacheObj.detMgr.get());
        
        if (alignStore && alignStore->getBLine(reId)) {
            newRE->setBLinePar(*alignStore->getBLine(reId));
        }
        for (unsigned int layer = 1; layer <= copyMe->numLayers(); ++layer) {
            using channelType = MuonGMR4::sTgcReadoutElement::ReadoutChannelType;
            using ChannelDesign =  MuonGM::MuonChannelDesign;
            const IdentifierHash layerHash = MuonGMR4::sTgcReadoutElement::createHash(layer,channelType::Strip,0);
            
            const MuonGMR4::StripLayer& stripLayer{copyMe->stripLayer(layerHash)};
            newRE->m_Xlg[layer -1] =  stripLayer.toOrigin() * Amg::getRotateY3D(90. * Gaudi::Units::deg) * Amg::getTranslateX3D( layer%2 ? - 0.01 : 0.01 ); 
           
            const MuonGMR4::StripDesign& copyEtaDesign{stripLayer.design()}; 
            ATH_MSG_VERBOSE("Layer: "<<layer<<" "<<copyEtaDesign);
            /// Initialize the eta design
            ChannelDesign& etaDesign{newRE->m_etaDesign[layer-1]};
            etaDesign.type =  ChannelDesign::ChannelType::etaStrip;
            etaDesign.detType = ChannelDesign::DetType::STGC;
            if (copyEtaDesign.yCutout()) {
                etaDesign.defineDiamond(copyEtaDesign.shortHalfHeight(),
                                        copyEtaDesign.longHalfHeight(),
                                        copyEtaDesign.halfWidth(),
                                        copyEtaDesign.yCutout());
            } else {
                etaDesign.defineTrapezoid(copyEtaDesign.shortHalfHeight(),
                                          copyEtaDesign.longHalfHeight(),
                                          copyEtaDesign.halfWidth());

            }
            etaDesign.firstPitch  = copyEtaDesign.firstStripPos().x() + 0.5*copyEtaDesign.stripPitch() + copyEtaDesign.halfWidth();
            etaDesign.inputPitch  = copyEtaDesign.stripPitch();
            etaDesign.inputWidth  = copyEtaDesign.stripWidth();
            etaDesign.nch = copyEtaDesign.numStrips();
            ATH_MSG_VERBOSE(m_idHelperSvc->toStringDetEl(copyMe->identify())<<", layer: "<<layer<<", eta-design: "<< copyEtaDesign);
            etaDesign.setFirstPos(copyEtaDesign.firstStripPos().x() + 0.5*copyEtaDesign.stripPitch());
            /// Initialize the phi design

            const MuonGMR4::WireGroupDesign& copyPhiDesign{copyMe->wireDesign(layerHash)};
            
            ChannelDesign& phiDesign{newRE->m_phiDesign[layer-1]};
            phiDesign.type = ChannelDesign::ChannelType::phiStrip;
            phiDesign.detType = ChannelDesign::DetType::STGC;
            if (copyPhiDesign.yCutout() == 0.) {
                phiDesign.defineTrapezoid(copyPhiDesign.shortHalfHeight(),
                                          copyPhiDesign.longHalfHeight(),
                                          copyPhiDesign.halfWidth());
            } else { 
            phiDesign.defineDiamond(copyPhiDesign.shortHalfHeight(),
                                    copyPhiDesign.longHalfHeight(),
                                    copyPhiDesign.halfWidth(), 
                                    copyPhiDesign.yCutout());
            }
            phiDesign.inputPitch  = copyPhiDesign.stripPitch();
            phiDesign.inputWidth  = copyPhiDesign.stripWidth();
            ATH_MSG_VERBOSE(m_idHelperSvc->toStringDetEl(copyMe->identify())<<", layer: "<<layer<<", phi-design: "<< copyPhiDesign);
            phiDesign.setFirstPos(copyPhiDesign.firstStripPos().x()); // Position of 1st wire, accounts for staggering
            phiDesign.firstPitch = copyPhiDesign.numWiresInGroup(1);  // Number of Wires in 1st group, group staggering
            phiDesign.groupWidth  = copyPhiDesign.numWiresInGroup(2);                // Number of Wires normal group
            phiDesign.nGroups = copyPhiDesign.numStrips();                           // Number of Wire Groups
            phiDesign.wireCutout = copyPhiDesign.wireCutout();                       // Size of "active" wire region for digits
            phiDesign.nch = copyPhiDesign.nAllWires();
            phiDesign.isConvertedFromPhaseII = true;

            const MuonGMR4::PadDesign& copyPadDesign{copyMe->padDesign(layerHash)};
            MuonGM::MuonPadDesign& padDesign{newRE->m_padDesign[layer-1]};
            padDesign.Length  = copyMe->chamberHeight();
            padDesign.sWidth  = copyMe->sChamberLength();
            padDesign.lWidth  = copyMe->lChamberLength();
            padDesign.Size =  2.*copyPadDesign.halfWidth();
            padDesign.ysFrame = copyMe->sFrameWidth();
            padDesign.ylFrame = copyMe->lFrameWidth();
            padDesign.thickness = copyMe->thickness();
            /// To circumvent the yCutout calculations in padCorner function in MuonPadDesign
            if (copyPadDesign.yCutout()) {
                padDesign.yCutout = copyPadDesign.halfWidth();
            }
            padDesign.setR(copyPadDesign.beamlineRadius());   
            padDesign.sPadWidth = 2.*copyPadDesign.shortHalfHeight(); 
            padDesign.lPadWidth = 2.*copyPadDesign.longHalfHeight(); 
            padDesign.nPadColumns = copyPadDesign.numPadPhi();
            padDesign.firstPhiPos   = copyPadDesign.firstPadPhiDiv();
            padDesign.inputPhiPitch = copyPadDesign.anglePadPhi();      
            padDesign.PadPhiShift   = copyPadDesign.padPhiShift();
            padDesign.nPadH         = copyPadDesign.numPadEta();
            padDesign.padEtaMax     = copyPadDesign.maxPadEta();  
            padDesign.firstRowPos   = copyPadDesign.firstPadHeight();     
            padDesign.inputRowPitch = copyPadDesign.padHeight();          
            padDesign.sectorOpeningAngle = copyPadDesign.sectorAngle();
            padDesign.isConvertedFromPhaseII = true;
        }     
        newRE->fillCache();
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newRE));
        cacheObj.detMgr->addsTgcReadoutElement(std::move(newRE));

    }
    return StatusCode::SUCCESS;
}
StatusCode ReadoutGeomCnvAlg::buildMdt(const ActsGeometryContext& gctx, ConstructionCache& cacheObj) const {    
    /// Access the B-Line and As-built parameters
    SubDetAlignment alignItr = gctx.getStore(ActsTrk::DetectorType::Mdt);
    const MdtAlignmentStore* alignStore = alignItr ?
                                 static_cast<const MdtAlignmentStore*>(alignItr->internalAlignment.get()) : nullptr;

    const std::vector<const MuonGMR4::MdtReadoutElement*> mdtReadOuts{m_detMgr->getAllMdtReadoutElements()};
    ATH_MSG_INFO("Copy "<<mdtReadOuts.size()<<" Mdt readout elements to the legacy system");
    for (const MuonGMR4::MdtReadoutElement* copyMe : mdtReadOuts) {
        const Identifier reId = copyMe->identify();
        ATH_MSG_DEBUG("Translate "<<m_idHelperSvc->toStringDetEl(reId));
        /// Build the mother station
        GeoIntrusivePtr<GeoVFullPhysVol> physVol{};
        MuonGM::MuonStation* station{nullptr};
        ATH_CHECK(cloneReadoutVolume(gctx,reId, cacheObj, physVol, station));
        if (copyMe->multilayer() == 1) {
            const MuonGMR4::MdtReadoutElement* otherRE = copyMe->complementaryRE();
            const double height = std::max(copyMe->moduleHeight(), otherRE->moduleHeight()) - 
                                          (copyMe->tubePitch() - 2. * copyMe->tubeRadius());

            const double modHalTHickO{0.5*otherRE->moduleThickness()},
                         modHalfThick{-0.5*copyMe->moduleThickness()};

            const double thickness = ( (otherRE->asBuiltRefFrame()*(modHalTHickO* Amg::Vector3D::UnitX())) -
                                        (copyMe->asBuiltRefFrame()*(modHalfThick* Amg::Vector3D::UnitX()))).z();
            if (copyMe->isBarrel()) {
                station->setMdtZsize(height);
                station->setMdtRsize(thickness);
            } else {
                station->setMdtRsize(height);
                station->setMdtZsize(thickness);
            }
        }
        const MuonGMR4::MdtReadoutElement::parameterBook& pars{copyMe->getParameters()};
        auto newElement = std::make_unique<MuonGM::MdtReadoutElement>(physVol, 
                                                                      m_idHelperSvc->stationNameString(reId), 
                                                                      cacheObj.detMgr.get());
        newElement->setIdentifier(reId);
        newElement->setMultilayer(copyMe->multilayer());
        newElement->setNMdtInStation(m_idHelperSvc->mdtIdHelper().multilayerMax(reId));
        // cppcheck-suppress invalidLifetime; ok: mgr took ownership.
        newElement->setParentMuonStation(station);

        /// 1 cm is added as safety margin to the Mdt multilayer envelope
        newElement->setLongSsize(2*pars.longHalfX - 1.*Gaudi::Units::cm);
        newElement->setSsize(2*pars.shortHalfX - 1.*Gaudi::Units::cm);
        newElement->setLongRsize(2*pars.halfY);
        newElement->setRsize(2*pars.halfY);
        newElement->setZsize(2*pars.halfHeight);
        newElement->setLongZsize(2*pars.halfHeight);

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
        double lastLength{2.*tubeLay.uncutHalfLength(1)}; 
        for (unsigned tube = 0; tube < copyMe->numTubesInLay(); ++tube) {
            const double currLength = 2.*tubeLay.uncutHalfLength(tube);
            ATH_MSG_VERBOSE(m_idHelperSvc->toString(copyMe->identify())<< ", tube "<<tube<<", length: "<<currLength);
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
        double xOffSet{pars.halfY}, yOffSet{pars.halfHeight};
        if (newElement->barrel())  std::swap(xOffSet, yOffSet);
        for (unsigned lay = 1; lay <= copyMe->numLayers(); ++lay) {
            const IdentifierHash tubeHash{copyMe->measurementHash(lay, 1)};
            const Amg::Vector3D locTube = copyMe->localTubePos(tubeHash);
            newElement->m_firstwire_x[lay-1] = locTube.z() + xOffSet;
            newElement->m_firstwire_y[lay-1] = locTube.x() + yOffSet;
        }
        MdtAlignmentStore::chamberDistortions distort = alignStore ? alignStore->getDistortion(reId) : 
                                                        MdtAlignmentStore::chamberDistortions{};
        
        if (!station->hasMdtAsBuiltParams()){
            station->setMdtAsBuiltParams(distort.asBuilt);
        }
        if (!station->hasBLines()){
            station->setBline(distort.bLine);
        }
        const Amg::Vector3D refPoint = copyMe->bLineReferencePoint();
        station->setBlineFixedPointInAmdbLRS(refPoint.x(), refPoint.y(), refPoint.z());

        newElement->geoInitDone();
        newElement->setBLinePar(distort.bLine);
        newElement->fillCache();
        ATH_CHECK(dumpAndCompare(gctx, *copyMe, *newElement));
        /// Add the readout element to the detector manager
        cacheObj.detMgr->addMdtReadoutElement(std::move(newElement));
    }
    return StatusCode::SUCCESS;
}

StatusCode ReadoutGeomCnvAlg::checkIdCompability(const MuonGMR4::MuonReadoutElement& refEle,
                                                 const MuonGM::MuonReadoutElement& testEle) const{
    
    if (refEle.identify() != testEle.identify()) {
        ATH_MSG_FATAL("Two different elements are compared "
                    <<m_idHelperSvc->toString(refEle.identify())<<" vs. "
                    <<m_idHelperSvc->toString(testEle.identify()));
        return StatusCode::FAILURE;
    }
    if (refEle.identHash() != testEle.detectorElementHash()) {
        ATH_MSG_FATAL("The hashes of the two detector elements "<<m_idHelperSvc->toString(refEle.identify())
                    <<" are completely different "<<refEle.identHash()<<" vs. "<<testEle.detectorElementHash());
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode ReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                             const MuonGMR4::MmReadoutElement& refEle,
                                             const MuonGM::MMReadoutElement& testEle) const {

    if (!m_checkGeo) {
        return StatusCode::SUCCESS;
    }
    ATH_CHECK(checkIdCompability(refEle, testEle));

    ATH_MSG_VERBOSE("Compare basic readout transforms"<<std::endl
                <<GeoTrf::toString(testEle.absTransform(),true)<<std::endl
                <<GeoTrf::toString(refEle.localToGlobalTrans(gctx), true));
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    for (unsigned int gasGap = 1; gasGap <= refEle.nGasGaps(); ++ gasGap) {
        const Identifier gapId = idHelper.channelID(refEle.identify(), refEle.multilayer(),  gasGap, 1);
        
        const Amg::Transform3D& refTrf{refEle.localToGlobalTrans(gctx, gapId)};
        const Amg::Transform3D& testTrf{testEle.transform(gapId)};
        if (!Amg::isIdentity(refTrf.inverse()*testTrf)) {
            ATH_MSG_FATAL("The layer "<<m_idHelperSvc->toStringGasGap(gapId)<<" does not transform equally"
                         <<GeoTrf::toString(refTrf, true) <<" vs. "<<GeoTrf::toString(testTrf, true));
            return StatusCode::FAILURE;
        }
        const MuonGMR4::StripDesign& stripDesign{refEle.stripLayer(gapId).design()};
        
        for (int strip = stripDesign.firstStripNumber(); strip <= stripDesign.numStrips(); ++strip) {
            const Identifier stripId = idHelper.channelID(refEle.identify(), refEle.multilayer(), gasGap, strip);
            const Amg::Vector3D refStripPos{refEle.stripPosition(gctx, stripId)};
            const Amg::Vector3D refStripDir{refEle.localToGlobalTrans(gctx, refEle.layerHash(stripId)).linear() * Amg::Vector3D::UnitX()};

            Amg::Vector3D testStripPos{Amg::Vector3D::Zero()};
            if (!testEle.stripGlobalPosition(stripId, testStripPos)) {
                ATH_MSG_FATAL("Failed to retrieve strip position "<<m_idHelperSvc->toString(stripId));
                return StatusCode::FAILURE;
            }
            const double dist = refStripDir.dot(refStripPos - testStripPos);
            if (std::abs(dist) > 10. * Gaudi::Units::micrometer) {
                ATH_MSG_FATAL("The strip "<<Amg::toString(testStripPos)<<" is not describing the same strip as "
                            <<Amg::toString(refStripPos)<<". Channel "<<m_idHelperSvc->toString(stripId)
                            <<" distance: "<<dist<<" "<<(dist / testEle.m_etaDesign[gasGap -1].inputWidth));
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Channel postion "<<m_idHelperSvc->toString(stripId)<<" match between legacy & new");
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode ReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                             const MuonGMR4::MdtReadoutElement& refEle,
                                             const MuonGM::MdtReadoutElement& testEle) const {
    
    if (!m_checkGeo) {
        return StatusCode::SUCCESS;
    }
    ATH_CHECK(checkIdCompability(refEle, testEle));
    
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
                            <<" reference: "<<GeoTrf::toString(globToLocal*refPos)<<" vs. test: "
                            <<GeoTrf::toString(globToLocal*tubePos) <<" delta: "<<(refPos - tubePos).mag()
                            <<" Transforms "<<std::endl
                            <<" **** "<< GeoTrf::toString(globToLocal.inverse())<<std::endl
                            <<" **** "<< GeoTrf::toString(testEle.transform(lay, tube)));
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Tube positions layer: "<<lay<<", tube: "<<tube
                <<std::endl<<"reference: "<<GeoTrf::toString(refPos)
                <<std::endl<<"test:      "<<GeoTrf::toString(tubePos)
                <<std::endl<<testEle.tubeLength(lay, tube)<<"/"<<testEle.getActiveTubeLength(lay, tube)<<"/"
                            <<testEle.getWireLength(lay,tube)<<" vs. "<<refEle.tubeLength(tubeHash)
                            <<"/"<<refEle.activeTubeLength(tubeHash)<<"/"<<refEle.wireLength(tubeHash)
                            <<"/"<<refEle.uncutTubeLength(tubeHash));
            if (std::abs(testEle.getTubeLengthForCaching(lay,tube) - refEle.uncutTubeLength(tubeHash)) >
                std::numeric_limits<float>::epsilon() ) {
                ATH_MSG_FATAL("Different tube length's detected for "<<m_idHelperSvc->toStringDetEl(refEle.identify())
                                << " layer: "<<lay<<", tube: "<<tube<<" -- "<<testEle.getTubeLengthForCaching(lay,tube)<<" (new) vs. "
                                <<refEle.uncutTubeLength(tubeHash)<<" (ref)");
                return StatusCode::FAILURE;
            }
        }
    }

    return StatusCode::SUCCESS;
}
StatusCode ReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                             const MuonGMR4::RpcReadoutElement& refEle,
                                             const MuonGM::RpcReadoutElement& testEle) const {
    
    if (!m_checkGeo) {
        return StatusCode::SUCCESS;
    }
    ATH_CHECK(checkIdCompability(refEle, testEle));

    ATH_MSG_VERBOSE("Compare basic readout transforms"<<std::endl
                 <<"  ref: "<<GeoTrf::toString(refEle.localToGlobalTrans(gctx), true)<<std::endl
                 <<" test: "<<GeoTrf::toString(testEle.absTransform(),true)<<std::endl
                 <<"delta: "<<GeoTrf::toString(testEle.absTransform().inverse()*refEle.localToGlobalTrans(gctx), true ));
    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    for (unsigned int gasGap = 1; gasGap <= refEle.nGasGaps(); ++gasGap) {
        for (int doubPhi = refEle.doubletPhi(); doubPhi <= refEle.doubletPhiMax(); ++doubPhi) {
            for (bool measPhi : {false, true}) {
                if (measPhi && !refEle.nPhiStrips()) continue;
                for (int strip = 1; strip <= testEle.Nstrips(measPhi); ++strip) {
                    const Identifier stripId = idHelper.channelID(refEle.identify(), 
                                                                  refEle.doubletZ(), 
                                                                  doubPhi, gasGap, measPhi, strip);
                    
                    const Amg::Transform3D& refTrans{refEle.localToGlobalTrans(gctx, stripId)};
                    const Amg::Transform3D& testTrans{testEle.transform(stripId)};
                    if (strip == 1 && !Amg::isIdentity(refTrans.inverse()*testTrans)) {
                        ATH_MSG_ERROR("Transformation for "<<m_idHelperSvc->toString(stripId)<<" - "<<refEle.identHash()<<std::endl
                            <<" *** ref:  "<<GeoTrf::toString(refTrans)<<std::endl
                            <<" *** test: "<<GeoTrf::toString(testTrans)<<std::endl
                            <<" -> delta: "<<GeoTrf::toString(refTrans.inverse()*testTrans));                            
                        return StatusCode::FAILURE;
                    }

                    const Amg::Vector3D refStripPos = refEle.stripPosition(gctx, stripId);
                    const Amg::Vector3D testStripPos = testEle.stripPos(stripId);
                    // The threshold here used to be epsilon for a float.
                    // But this was then giving a failure in aarch64,
                    // with a difference of almost exactly 1e-4.
                    // It turned out that that was coming from GeoDeDuplicator,
                    // where to consider two transforms equivalent,
                    // the translations must match to 1e-4.  But if the
                    // difference is almost exactly 1e-4, then small FP
                    // differences can be magnified to just about 1e-4.
                    if ((refStripPos - testStripPos).mag() > 2e-4){
                        ATH_MSG_ERROR("Mismatch in strip positions "<<m_idHelperSvc->toString(stripId)
                                <<" ref: "<<Amg::toString(refStripPos)<<" test: "<<Amg::toString(testStripPos)
                                <<" local coordinates -- ref: "<<Amg::toString(refTrans.inverse()*refStripPos)
                                <<" test: "<<Amg::toString(refTrans.inverse()*testStripPos));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_VERBOSE("Agreement between new and old geometry for channel "<<m_idHelperSvc->toString(stripId)
                                    <<" strip position "<<Amg::toString(refStripPos)
                                    <<", local: "<<Amg::toString(refTrans.inverse()*refStripPos));
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode ReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                             const MuonGMR4::TgcReadoutElement& refEle,
                                             const MuonGM::TgcReadoutElement& testEle) const {
    
    if (!m_checkGeo) {
        return StatusCode::SUCCESS;
    }
    ATH_CHECK(checkIdCompability(refEle, testEle));

    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    
    ATH_MSG_VERBOSE("Detector element "<<m_idHelperSvc->toString(refEle.identify())
                <<std::endl<<GeoTrf::toString(refEle.localToGlobalTrans(gctx), true)                        
                <<std::endl<<GeoTrf::toString(testEle.getMaterialGeom()->getAbsoluteTransform(), true)
                <<std::endl<<"r-size: "<<testEle.getRsize()<<"/"<<testEle.getLongRsize()
                           <<" s-size: "<<testEle.getSsize()<<"/"<<testEle.getLongSsize()
                           <<" z-size: "<<testEle.getZsize()<<"/"<<testEle.getLongZsize());
 
    for (unsigned int gasGap = 1; gasGap <= refEle.nGasGaps(); ++gasGap) {
        for (bool isStrip : {false, true}) {
            const IdentifierHash layHash = refEle.constructHash(0, gasGap, isStrip);
            const Identifier layId = idHelper.channelID(refEle.identify(), gasGap, isStrip, 1);
            ATH_MSG_VERBOSE("Test layer "<<m_idHelperSvc->toString(layId)<<" "<<refEle.numChannels(layHash)<<" "<<layHash);
            if (!refEle.numChannels(layHash)) continue;
            const Amg::Transform3D& refLayerTrf = refEle.localToGlobalTrans(gctx, layHash);
            const Amg::Transform3D& testLayerTrf = testEle.transform(layId);
            if (!Amg::isIdentity(refLayerTrf.inverse()* testLayerTrf)) {
                ATH_MSG_FATAL("The transformations in "<<m_idHelperSvc->toString(layId)
                            <<std::endl<<"ref : "<<GeoTrf::toString(refLayerTrf,true)
                            <<std::endl<<"test: "<<GeoTrf::toString(testLayerTrf,true)
                            <<" are not identical. ");
                return StatusCode::FAILURE;
            }
            ATH_MSG_VERBOSE("Transformations in "<<m_idHelperSvc->toString(layId)
                            <<std::endl<<"ref : "<<GeoTrf::toString(refLayerTrf,true)
                            <<std::endl<<"test: "<<GeoTrf::toString(testLayerTrf,true));

            for (unsigned int ch = 1; ch <= refEle.numChannels(layHash); ++ch) {
                const IdentifierHash measHash = refEle.constructHash(ch, gasGap, isStrip);
                const Identifier measId = refEle.measurementId(measHash);
                const Amg::Vector3D refChannel = refEle.channelPosition(gctx, measHash);
                const Amg::Vector3D testChannel = testEle.channelPos(measId);
                if ((refChannel - testChannel).mag() < std::numeric_limits<float>::epsilon()){
                    continue;
                }
                std::stringstream msg{};
                msg<<"The channel "<<m_idHelperSvc->toString(measId)
                        << " is not at the same position "<<Amg::toString(refChannel)
                        <<" vs. "<<Amg::toString(testChannel)<<". Difference: "
                        <<(refChannel - testChannel).mag();
                if (!isStrip) {
                    msg<<std::endl<<"*** Test *** - wirePitch: "<<testEle.wirePitch()
                                  <<", tot wires "<<testEle.nWires(gasGap)
                                  <<", wires to reach "<<testEle.nPitchesToGang(gasGap, ch)
                                  <<", wires in gang "<<testEle.nWires(gasGap, ch);
                    const MuonGMR4::WireGroupDesign& design{refEle.wireGangLayout(gasGap)};
                    msg<<std::endl<<"*** Ref  *** - wirePitch: "<<design.stripPitch()
                                  <<", tot wires "<<testEle.nWires(gasGap)
                                  <<", wires to reach "<<design.numPitchesToGroup(ch)
                                  <<", wires in gang "<<design.numWiresInGroup(ch);
                } else {
                    const Amg::Vector3D locRefPos{refLayerTrf.inverse() * refChannel};
                    const Amg::Vector3D locTestPos{refLayerTrf.inverse()* testChannel};
                    msg<<std::endl<<"*** Ref  **** - "<<Amg::toString(locRefPos)<<std::endl;
                    msg<<std::endl<<"*** Test **** - "<<Amg::toString(locTestPos)<<std::endl;
                }
                ATH_MSG_FATAL(msg.str());
                return StatusCode::FAILURE;
            }

        }
    }
    return StatusCode::SUCCESS;
}
StatusCode ReadoutGeomCnvAlg::dumpAndCompare(const ActsGeometryContext& gctx,
                                             const MuonGMR4::sTgcReadoutElement& refEle,
                                             const MuonGM::sTgcReadoutElement& testEle) const {
    if (!m_checkGeo) {
        return StatusCode::SUCCESS;
    }
    ATH_CHECK(checkIdCompability(refEle, testEle));

    ATH_MSG_VERBOSE("Compare basic readout transforms"<<std::endl
                <<GeoTrf::toString(testEle.absTransform(),true)<<std::endl
                <<GeoTrf::toString(refEle.localToGlobalTrans(gctx), true));
    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    for (unsigned int gasGap = 1; gasGap <= refEle.numLayers(); ++gasGap) {
        for (int chType : {sTgcIdHelper::sTgcChannelTypes::Pad , sTgcIdHelper::sTgcChannelTypes::Strip, sTgcIdHelper::sTgcChannelTypes::Wire}) {
            const Identifier layID = idHelper.channelID(refEle.identify(),
                                                    refEle.multilayer(),
                                                    gasGap, chType, 1);
            const unsigned int numChannel = refEle.numChannels(layID);
            constexpr unsigned firstCh = 1;
            for (unsigned int channel = firstCh; channel < numChannel ; ++channel) {
                const Identifier chID = idHelper.channelID(refEle.identify(),
                                                           refEle.multilayer(),
                                                           gasGap, chType, channel);
            
                const Amg::Transform3D& refTrans{refEle.localToGlobalTrans(gctx, chID)};
                const Amg::Transform3D& testTrans{testEle.transform(chID)};
                if (channel == firstCh && (!Amg::doesNotDeform(testTrans.inverse()*refTrans)
                                        || (testTrans.inverse()*refTrans).translation().perp() > std::numeric_limits<float>::epsilon() ) ) {
                    ATH_MSG_ERROR(__func__<<"() "<<__LINE__<<" - Transformation for "<<m_idHelperSvc->toString(chID)<<std::endl
                        <<" *** ref:  "<<GeoTrf::toString(refTrans, true)<<std::endl
                        <<" *** test: "<<GeoTrf::toString(testTrans, true));
                        return StatusCode::FAILURE;
                }
                if (chType == sTgcIdHelper::sTgcChannelTypes::Pad) {
                    /// R4 Geometry function accepts the channelID as input identifier, whereas, R3 geometry needs to be provided
                    /// padID explicitly.
                    bool isValid = false;
                    const int padEta = refEle.padEta(chID);
                    const int padPhi = refEle.padPhi(chID);
                    const Identifier padID = idHelper.padID(refEle.identify(),
                                                            refEle.multilayer(),
                                                            gasGap, chType, padEta, padPhi, isValid);
                    if(!isValid) {
                        ATH_MSG_WARNING("The following pad ID is not valid: " << padID);
                    }    
                    const Amg::Transform3D& testPadTrans{testEle.transform(padID)};
                    const Amg::Vector3D refChannelPos = refEle.globalChannelPosition(gctx, chID);
                    Amg::Vector3D testChannelPos(Amg::Vector3D::Zero()); 
                    testEle.stripGlobalPosition(padID, testChannelPos);
                    
                    const std::array<Amg::Vector3D,4> refPadCorners = refEle.globalPadCorners(gctx, chID);
                    std::array<Amg::Vector3D,4> testPadCorners{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};
                    testEle.padGlobalCorners(padID, testPadCorners);              
                    for (unsigned int cornerIdx = 0; cornerIdx < refPadCorners.size(); ++cornerIdx) {
                        const double padCornerDiff = (refPadCorners[cornerIdx] - testPadCorners[cornerIdx]).mag();
                        if (padCornerDiff - 25. > 1. * Gaudi::Units::micrometer){
                            ATH_MSG_ERROR("Mismatch in pad Corner " << cornerIdx << ": " <<m_idHelperSvc->toString(padID)
                                    <<" ref: "<<Amg::toString(refPadCorners[cornerIdx])<<" test: "<<Amg::toString(testPadCorners[cornerIdx])
                                    <<" difference: " << padCornerDiff
                                    <<" local coordinates -- ref: "<<Amg::toString(refTrans.inverse()*refPadCorners[cornerIdx])
                                    <<" test: "<<Amg::toString(testPadTrans.inverse()*testPadCorners[cornerIdx]));
                            return StatusCode::FAILURE;
                        }   
                    }
                    const double padChannelDiff = (refChannelPos - testChannelPos).mag();
                    if (padChannelDiff - 25. > 1. * Gaudi::Units::micrometer){
                        ATH_MSG_ERROR("Mismatch in pad positions "<<m_idHelperSvc->toString(padID)
                                <<" ref: "<<Amg::toString(refChannelPos)<<" test: "<<Amg::toString(testChannelPos)
                                <<" difference: " << padChannelDiff
                                <<" local coordinates -- ref: "<<Amg::toString(refTrans.inverse()*refChannelPos)
                                <<" test: "<<Amg::toString(testPadTrans.inverse()*testChannelPos));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_VERBOSE("Agreement between new and old geometry for channel "<<m_idHelperSvc->toString(padID)
                                    <<" channel position "<<Amg::toString(refChannelPos));
                }
                else if (chType == sTgcIdHelper::sTgcChannelTypes::Strip){
                    const Amg::Vector3D refChannelPos = refEle.globalChannelPosition(gctx, chID);
                    Amg::Vector3D testChannelPos{Amg::Vector3D::Zero()}; 
                    testEle.stripGlobalPosition(chID, testChannelPos);
                    if ((refChannelPos - testChannelPos).mag() > 1. * Gaudi::Units::micrometer){
                        ATH_MSG_ERROR("Mismatch in strip positions "<<m_idHelperSvc->toString(chID)
                                <<" ref: "<<Amg::toString(refChannelPos)<<" test: "<<Amg::toString(testChannelPos)
                                <<" local coordinates -- ref: "<<Amg::toString(testTrans.inverse()*refChannelPos)
                                <<" test: "<<Amg::toString(testTrans.inverse()*testChannelPos));
                        return StatusCode::FAILURE;
                    }
                    ATH_MSG_VERBOSE("Agreement between new and old geometry for channel "<<m_idHelperSvc->toString(chID)
                                    <<" channel position "<<Amg::toString(refChannelPos));
                } else { // wire
                    const Amg::Vector3D refChannelPos = refEle.globalChannelPosition(gctx, chID);
                    Amg::Vector3D testChannelPos{Amg::Vector3D::Zero()}; 
                    testEle.stripGlobalPosition(chID, testChannelPos);
                    Amg::Vector3D localRefPos {testTrans.inverse()*refChannelPos};
                    Amg::Vector3D localTestPos{testTrans.inverse()*testChannelPos};
                    /// wireGroup center is defined to be half a wire pitch off in R4 compared to R3 convention
                    if((std::abs(localRefPos.x() -localTestPos.x()) > 1.* Gaudi::Units::micrometer)
                        || (std::abs(refChannelPos.z() -testChannelPos.z()) > 15.* Gaudi::Units::micrometer)){
                            ATH_MSG_ERROR("Mismatch in wire positions "<<m_idHelperSvc->toString(chID)
                            <<" ref: "<<Amg::toString(refChannelPos)<<" test: "<<Amg::toString(testChannelPos)
                            <<" local coordinates -- ref: "<<Amg::toString(testTrans.inverse()*refChannelPos)
                            <<" test: "<<Amg::toString(testTrans.inverse()*testChannelPos)
                            <<"delta X: "<<std::abs(localRefPos.x() -localTestPos.x())
                            <<", delta Z: "<<std::abs(localRefPos.z() -localTestPos.z()));
                        return StatusCode::FAILURE;  
                    }
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
}
