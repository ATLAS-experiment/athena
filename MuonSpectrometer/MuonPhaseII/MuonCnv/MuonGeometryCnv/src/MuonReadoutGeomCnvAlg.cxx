
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeomCnvAlg.h"

#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#include <MuonReadoutGeometry/MuonStation.h>
#include <MuonReadoutGeometry/MdtReadoutElement.h>
#include <AthenaKernel/IOVInfiniteRange.h>


#include <map>

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
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}


StatusCode MuonReadoutGeomCnvAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<MuonGM::MuonDetectorManager> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("The current readout geometry is still valid.");
        return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle<ActsGeometryContext> geoContext{m_geoCtxKey, ctx};
    if (!geoContext.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve "<<m_geoCtxKey.fullKey());
        return StatusCode::FAILURE;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());
    writeHandle.addDependency(geoContext);
    std::unique_ptr<MuonGM::MuonDetectorManager> detMgr = std::make_unique<MuonGM::MuonDetectorManager>();
    PVLink world{new GeoFullPhysVol(nullptr)};
    detMgr->addTreeTop(world);
    ATH_CHECK(buildMdt(**geoContext, detMgr.get(), world));
    ATH_CHECK(writeHandle.record(std::move(detMgr)));
    return StatusCode::SUCCESS;
}
StatusCode MuonReadoutGeomCnvAlg::buildMdt(const ActsGeometryContext& gctx,
                                           MuonGM::MuonDetectorManager* mgr,
                                           PVLink world) const {    
    /// Access the B-Line and As-built parameters
    using SubDetAlignments = ActsGeometryContext::SubDetAlignments;
    SubDetAlignments::const_iterator alignItr = gctx.alignmentStores.find(ActsTrk::DetectorType::Mdt);
    const MdtAlignmentStore* alignStore = alignItr != gctx.alignmentStores.end() ? 
                             static_cast<const MdtAlignmentStore*>(alignItr->second.get()) : nullptr;

    const std::vector<const MuonGMR4::MdtReadoutElement*> mdtReadOuts{m_detMgr->getAllMdtReadoutElements()};
    ATH_MSG_INFO("Copy "<<mdtReadOuts.size()<<" Mdt readout elements to the legacy system");
    for (const MuonGMR4::MdtReadoutElement* copyMe : mdtReadOuts) {
        const Identifier reId = copyMe->identify();
        /// Retrieve the full phyiscal volume
        const GeoVFullPhysVol* readOutVol = copyMe->getMaterialGeom();
        
        const std::string stName{m_idHelperSvc->stationNameString(reId)};
        MuonGM::MuonStation* station = mgr->getMuonStation(stName, m_idHelperSvc->stationEta(reId), m_idHelperSvc->stationPhi(reId));
        
        if (!station) {
            const MuonGMR4::MuonChamber* chamber = m_detMgr->getChamber(reId);
            if (!chamber) {
                ATH_MSG_FATAL("No chamber is available for "<<m_idHelperSvc->toStringDetEl(reId));
                return StatusCode::FAILURE;
            }
            std::unique_ptr<MuonGM::MuonStation> newStation = std::make_unique<MuonGM::MuonStation>(stName,
                                                                                                    chamber->halfXShort(),   /// S-size
                                                                                                    chamber->halfY(),        /// R-size
                                                                                                    chamber->halfZ(),        /// Z-size
                                                                                                    chamber->halfXLong(),    /// S-size (long)
                                                                                                    chamber->halfY(),        /// R-size (long)
                                                                                                    chamber->halfZ(),        /// Z-size (long)
                                                                                                    m_idHelperSvc->stationEta(reId),
                                                                                                    m_idHelperSvc->stationPhi(reId),
                                                                                                    false);
            station = newStation.get();
            mgr->addMuonStation(std::move(newStation));
            PVConstLink parentVolume = readOutVol->getParent();
            /// Construct the aligned station transformation 
            const Amg::Transform3D stationTransform =  copyMe->localToGlobalTrans(gctx) *
                                                       readOutToStation(readOutVol);
            
            /// Assign the new phyiscal volume for the muon station
            PVLink parentPhysVol{new GeoFullPhysVol(parentVolume->getLogVol())};
            world->add(new GeoTransform(stationTransform));
            world->add(parentPhysVol.operator->());
            station->setPhysVol(parentPhysVol);
        }
        const MuonGMR4::MdtReadoutElement::parameterBook& pars{copyMe->getParameters()};
        // cppcheck-suppress invalidLifetime; ok: mgr took ownership.
        PVLink parentPhysVol{station->getPhysVol()};
        GeoFullPhysVol* physVol{new GeoFullPhysVol(readOutVol->getLogVol())};
        parentPhysVol->add(new GeoTransform(readOutToStation(readOutVol).inverse()));
        parentPhysVol->add(physVol);

        std::unique_ptr<MuonGM::MdtReadoutElement> newElement = std::make_unique<MuonGM::MdtReadoutElement>(physVol, stName, mgr);
        newElement->setIdentifier(reId);
        // cppcheck-suppress invalidLifetime; ok: mgr took ownership.
        newElement->setParentMuonStation(station);
        /// Define the dimensions
        newElement->setLongRsize(2*pars.halfY);
        newElement->setLongSsize(2*pars.longHalfX);
        newElement->setLongZsize(2*pars.halfHeight);
        newElement->setRsize(2*pars.halfY);
        newElement->setSsize(2*pars.shortHalfX);
        newElement->setZsize(2*pars.halfHeight);

        newElement->m_nlayers = copyMe->numLayers();
        newElement->m_ntubesperlayer = copyMe->numTubesInLay();
        newElement->m_deadlength = pars.deadLength;
        newElement->m_innerRadius = pars.tubeInnerRad;
        newElement->m_tubeWallThickness = pars.tubeWall;
        newElement->m_tubepitch = pars.tubePitch;
        /// Need to check how to obtain this parameter from the new geometry
        /// newElement->m_cutoutShift;

        /// Determine the tube length's 
        const MuonGMR4::MdtTubeLayer& tubeLay{*pars.tubeLayers[0]};
        unsigned int step{0};
        double lastLength{2.*tubeLay.tubeHalfLength(1)}; 
        for (unsigned tube = 0; tube < copyMe->numTubesInLay(); ++tube) {
            const double currLength = 2.*tubeLay.tubeHalfLength(tube);
            if (std::abs(lastLength - currLength) > std::numeric_limits<float>::epsilon() ||
                tube == copyMe->numTubesInLay() -1) {
                newElement->m_tubelength[step] = lastLength;
                if (step == 0) newElement->m_ntubesinastep = tube -1;
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
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_ALWAYS("Detector element "<<m_idHelperSvc->toString(copyMe->identify())
                            <<std::endl<<station->getPhysVol()->getNChildVols()<<"  "<<(station->getPhysVol().operator->())
                            <<std::endl<<Amg::toString(copyMe->localToGlobalTrans(gctx))                        
                            <<std::endl<<Amg::toString(newElement->getMaterialGeom()->getAbsoluteTransform())
                            <<std::endl<<"r-size: "<<newElement->getRsize()<<"/"<<newElement->getLongRsize()
                                       <<" s-size: "<<newElement->getSsize()<<"/"<<newElement->getLongSsize()
                                       <<" z-size: "<<newElement->getZsize()<<"/"<<newElement->getLongZsize());
            for (unsigned int lay = 1; lay <= copyMe->numLayers(); ++lay){
                for (unsigned int tube = 1; tube <= copyMe->numTubesInLay(); ++tube) {
                    const IdentifierHash tubeHash {copyMe->measurementHash(lay,tube)};
                    ATH_MSG_ALWAYS("Tube positions layer: "<<lay<<", tube: "<<tube
                        <<std::endl<<"reference: "<<Amg::toString(copyMe->globalTubePos(gctx, tubeHash))
                        <<std::endl<<"test:      "<<Amg::toString(newElement->tubePos(lay, tube)));
                }
            }
        }
        mgr->addMdtReadoutElement(std::move(newElement));
    }
    return StatusCode::SUCCESS;
}





