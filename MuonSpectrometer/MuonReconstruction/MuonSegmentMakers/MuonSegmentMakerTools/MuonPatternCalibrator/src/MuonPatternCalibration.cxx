/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternCalibrator/MuonPatternCalibration.h"

#include <iostream>
#include <set>
#include <sstream>

#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
namespace Muon {

MuonPatternCalibration::MuonPatternCalibration(const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t, n, p) {}

StatusCode
MuonPatternCalibration::initialize()
{
    ATH_MSG_VERBOSE("MuonPatternCalibration::Initializing");
    ATH_CHECK(m_mdtCreator.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_keyRpc.initialize(!m_keyRpc.empty()));
    ATH_CHECK(m_keyTgc.initialize(!m_keyTgc.empty()));
    ATH_CHECK(m_clusterCreator.retrieve(EnableTool{!m_keyRpc.empty() || !m_keyTgc.empty()}));
    return StatusCode::SUCCESS;
}



StatusCode MuonPatternCalibration::calibrate(const EventContext& ctx, const MuonPatternCombination& pattern, ROTsPerRegion& hitsPerRegion) const {
    MuonPatternCalibration::RegionMap regionMap;
    bool hasPhiMeasurements = checkForPhiMeasurements(pattern);
    ATH_CHECK(createRegionMap(ctx, pattern, regionMap, hasPhiMeasurements));
    // calibrate hits
    calibrateRegionMap(regionMap, hitsPerRegion);
    return StatusCode::SUCCESS;
}


int MuonPatternCalibration::getRegionId(const Identifier& id) const{

    // simple division of MuonSpectrometer in regions using barrel/endcap seperation plus
    // inner/middle/outer seperation
    using namespace MuonStationIndex;
    return toInt(m_idHelperSvc->stationIndex(id)) * (m_idHelperSvc->stationEta(id) > 0 ? 1 : -1);
}


bool
MuonPatternCalibration::checkForPhiMeasurements(const MuonPatternCombination& pat) const {

    for (const  MuonPatternChamberIntersect& intersect : pat.chamberData()) {
        for (const Trk::PrepRawData* prd : intersect.prepRawDataVec()){
            const Identifier id = prd->identify();
            /// Exclude the sTGC hits. MM hits do not have phi measurements
            if (!m_idHelperSvc->issTgc(id) &&
                m_idHelperSvc->measuresPhi(id)) return true;
        }
    }
    return false;
}

StatusCode
MuonPatternCalibration::createRegionMap(const EventContext& ctx,const MuonPatternCombination& pat, 
                                        RegionMap& regionMap, bool hasPhiMeasurements ) const
{
    if (hasPhiMeasurements)
        ATH_MSG_DEBUG("pattern has phi measurements using extrapolation to determine second coordinate");
    else
        ATH_MSG_DEBUG("No phi measurements using center tubes");

    
    const TgcPrepDataContainer* tgcPrdCont{nullptr};
    const RpcPrepDataContainer* rpcPrdCont{nullptr};
    ATH_CHECK(loadFromStoreGate(ctx, m_keyRpc, rpcPrdCont));
    ATH_CHECK(loadFromStoreGate(ctx, m_keyTgc, tgcPrdCont)); 
   
    for (const MuonPatternChamberIntersect& isect : pat.chamberData()) {

        if (isect.prepRawDataVec().empty()) continue;

        const Amg::Vector3D& patpose = isect.intersectPosition();
        const Amg::Vector3D  patdire = isect.intersectDirection().unit();

        /** Try to recover missing phi clusters:
            - loop over the clusters in the region and sort them by collection
            - count the number of eta and phi clusters per collection

        */
        std::map<int, EtaPhiHits> etaPhiHitsPerChamber;
        std::set<Identifier>      clusterIds;


        const Trk::PrepRawData* prd = isect.prepRawDataVec().front();
        const Identifier id = prd->identify();

        // apply cut on the opening angle between pattern and chamber phi
        // do some magic to avoid problems at phi = 0 and 2*pi
        double phiStart  = patdire.phi();
        double chPhi     = prd->detectorElement()->center().phi();
        constexpr double phiRange  = 0.75 * M_PI;
        constexpr double phiRange2 = 0.25 * M_PI;
        double phiOffset = 0.;
        if (phiStart > phiRange || phiStart < -phiRange)
            phiOffset = 2 * M_PI;
        else if (phiStart > -phiRange2 && phiStart < phiRange2)
            phiOffset = M_PI;

        if (phiOffset > 1.5 * M_PI) {
            if (phiStart < 0) phiStart += phiOffset;
            if (chPhi < 0) chPhi += phiOffset;
        } else if (phiOffset > 0.) {
            phiStart += phiOffset;
            chPhi += phiOffset;
        }
        double dphi = std::abs(phiStart - chPhi);

        if (dphi > m_phiAngleCut) {
            ATH_MSG_DEBUG("Large angular phi difference between pattern and chamber, phi pattern "
                          << patdire.phi() << "   phi chamber " << prd->detectorElement()->center().phi());
            continue;
        }

        // map to find duplicate hits in the chamber
        std::map<Identifier, const MdtPrepData*> idMdtMap{};

        for (const Trk::PrepRawData* isect_prd : isect.prepRawDataVec()) {

            if (isect_prd->type(Trk::PrepRawDataType::MdtPrepData)) {
                 const MdtPrepData* mdt = dynamic_cast<const MdtPrepData*>(isect_prd);           
                if (m_removeDoubleMdtHits) {
                    const MdtPrepData*& previousMdt = idMdtMap[mdt->identify()];
                    if (!previousMdt || previousMdt->tdc() > mdt->tdc())
                        previousMdt = mdt;
                    else
                        continue;
                }
                insertMdt(*mdt, regionMap, patpose, patdire, hasPhiMeasurements);
                continue;
            } /// Remove the NSW hits from the segment building 
            else if (isect_prd->type(Trk::PrepRawDataType::MMPrepData) || isect_prd->type(Trk::PrepRawDataType::sTgcPrepData)){
                continue;
            }   
            const MuonCluster* clus = dynamic_cast<const MuonCluster*>(isect_prd);
            if (!clus) continue;
            const Identifier id = clus->identify();
            if (!clusterIds.insert(id).second) continue;
            
            if (m_recoverTriggerHits) {
                bool measuresPhi = m_idHelperSvc->measuresPhi(id);
                int  colHash     = clus->collectionHash();
                EtaPhiHits& hitsPerChamber = etaPhiHitsPerChamber[colHash];
                if (measuresPhi)
                    ++hitsPerChamber.nphi;
                else
                    ++hitsPerChamber.neta;
            }
            insertCluster(*clus, regionMap, patpose, patdire, hasPhiMeasurements);            
        }
        for (const auto& [coll_hash, hits] : etaPhiHitsPerChamber) {
            if ((hits.neta > 0 && hits.nphi == 0) || (hits.nphi > 0 && hits.neta == 0)) {
                if (m_idHelperSvc->isRpc(id) && rpcPrdCont) {

                    const RpcPrepDataCollection* prd_coll = rpcPrdCont->indexFindPtr(coll_hash);
                    if (!prd_coll) {
                        ATH_MSG_VERBOSE("RpcPrepDataCollection not found in container!!"<< m_keyRpc);
                        continue;
                    }                   
                    for (const RpcPrepData* rpc_prd : *prd_coll) {
                        if (!clusterIds.insert(rpc_prd->identify()).second) continue;
                        insertCluster(*rpc_prd, regionMap, patpose, patdire, hasPhiMeasurements);
                    }                    
                } else if (m_idHelperSvc->isTgc(id) && tgcPrdCont) {
                     const TgcPrepDataCollection* prd_coll = tgcPrdCont->indexFindPtr(coll_hash);
                     if (!prd_coll) {
                        ATH_MSG_DEBUG("TgcPrepDataCollection not found in container!! "<< m_keyTgc);
                        continue;
                    }
                   
                    for (const TgcPrepData* tgc_prd : *prd_coll) {
                        if (!clusterIds.insert(tgc_prd->identify()).second) continue;
                        insertCluster(*tgc_prd, regionMap, patpose, patdire, hasPhiMeasurements);                        
                    }                    
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}

void
MuonPatternCalibration::insertCluster(const MuonCluster& clus, RegionMap& regionMap, const Amg::Vector3D& patpose,
                                      const Amg::Vector3D& patdire, bool hasPhiMeasurements) const
{

    const Identifier id = clus.identify();
    // check whether we are measuring phi or eta
    const bool measuresPhi = m_idHelperSvc->measuresPhi(id);

    Amg::Vector3D globalpos = clus.globalPosition();
    Amg::Vector3D intersect{Amg::Vector3D::Zero()};

    if (hasPhiMeasurements) {
        // if there is a phi measurement in the pattern use the global direction to calculate the intersect with
        // measurement plane and use the intersect to calculate the position along the strip

        // calculate intersect pattern measurement plane
        const Trk::Surface& surf         = clus.detectorElement()->surface(id);
        Amg::Vector3D       planepostion = surf.center();
        Amg::Vector3D       planenormal  = surf.normal();
        double              denom        = patdire.dot(planenormal);
        double              u            = (planenormal.dot(planepostion - patpose)) / (denom);
        Amg::Vector3D       piOnPlane    = (patpose + u * patdire);

        // transform to local plane coordiantes
        const Amg::Transform3D gToLocal = clus.detectorElement()->surface().transform().inverse();
        Amg::Vector3D          ilpos    = gToLocal * piOnPlane;
        Amg::Vector3D          glpos    = gToLocal * globalpos;

        // strip length
        double striplen(0.);

        // projective strips
        bool hasPointingPhiStrips = false;

        // detector specific stuff
        const RpcPrepData* rpc = dynamic_cast<const RpcPrepData*>(&clus);
        if (rpc) {
            striplen = rpc->detectorElement()->StripLength(measuresPhi);
        } else {
            const TgcPrepData* tgc = dynamic_cast<const TgcPrepData*>(&clus);
            if (!tgc) return;

            int gasGap = m_idHelperSvc->tgcIdHelper().gasGap(id);
            if (measuresPhi) {
                hasPointingPhiStrips = true;
                striplen             = tgc->detectorElement()->stripLength();
            } else {
                int wire = m_idHelperSvc->tgcIdHelper().channel(id);
                striplen = tgc->detectorElement()->gangCentralWidth(gasGap, wire);
            }
        }

        // set the position along the strip
        if (!measuresPhi) {
            glpos[0] = ilpos.x();
        } else {
            if (hasPointingPhiStrips) {
                // do some special for tgcs
                glpos[1] = ilpos.y();
            } else {
                glpos[1] = ilpos.y();
            }
        }

        // transform back to global coordinates
        intersect         = gToLocal.inverse() * glpos;
        Amg::Vector3D dif = globalpos - intersect;
        if ((intersect - piOnPlane).mag() > m_dropDistance || dif.mag() > 0.5 * striplen + m_dropDistance) {

            ATH_MSG_VERBOSE(">>>> extrapolated position far outside volume, dropping hit "
                            << m_idHelperSvc->toString(id) << ". dist along strip " << dif.mag() << " 1/2 strip len "
                            << 0.5 * striplen << " dist measurement plane " << (intersect - piOnPlane).mag());
            return;
        }
        if (dif.mag() > 0.5 * striplen) {
            Amg::Vector3D newpos = globalpos - dif * (0.5 * striplen / dif.mag());

            ATH_MSG_VERBOSE(">>>> extrapolated position outside volume, shifting position "
                            << m_idHelperSvc->toString(id) << ". position along strip " << dif.mag() << " 1/2 tube len "
                            << 0.5 * striplen << " dist To strip " << (intersect - piOnPlane).mag()
                            << ". dist to newpos " << (newpos - globalpos).mag() << " pos " << newpos);

            intersect = newpos;
        }
    } else {
        // no phi measurements, use strip center
        intersect = globalpos;
    }

    // enter hit in map
    int regionId = getRegionId(id);

    Region& region = regionMap[regionId];
    if (!region.init) {
        region.regionDir = patdire;
        region.regionPos = patpose;
        region.init = true;
    }   
    region.triggerPrds.emplace_back(intersect, &clus);
}


void
MuonPatternCalibration::insertMdt(const MdtPrepData& mdt, RegionMap& regionMap, const Amg::Vector3D& patpose,
                                  const Amg::Vector3D& patdire, bool hasPhiMeasurements) const
{

    Amg::Vector3D     intersect{Amg::Vector3D::Zero()};
    const Identifier& id = mdt.identify();

    const MuonGM::MdtReadoutElement* detEl   = mdt.detectorElement();
    const Amg::Vector3D&             tubePos = mdt.globalPosition();

    if (hasPhiMeasurements) {
        // if there is a phi measurement in the pattern use the global direction to calculate the intersect with the
        // tube use the intersect to calculate the second coordinate

        const Amg::Transform3D amdbToGlobal = detEl->AmdbLRSToGlobalTransform();


        // calculate intersect pattern measurement plane
        const Amg::Vector3D& planepostion = tubePos;

        // always project on plane with normal in radial direction
        Amg::Vector3D planenormal = m_idHelperSvc->mdtIdHelper().isBarrel(id)
                                        ? amdbToGlobal.linear() * Amg::Vector3D(0., 0., 1.)
                                        : amdbToGlobal.linear() * Amg::Vector3D(0., 1., 0.);

        double        denom     = patdire.dot(planenormal);
        double        u         = (planenormal.dot(planepostion - patpose)) / (denom);
        Amg::Vector3D piOnPlane = (patpose + u * patdire);

        Amg::Vector3D lpiOnPlane = amdbToGlobal.inverse() * piOnPlane;
        Amg::Vector3D ltubePos   = amdbToGlobal.inverse() * tubePos;

        intersect = amdbToGlobal * Amg::Vector3D(lpiOnPlane.x(), ltubePos.y(), ltubePos.z());

        Amg::Vector3D dif     = tubePos - intersect;
        double        tubelen = detEl->getActiveTubeLength(m_idHelperSvc->mdtIdHelper().tubeLayer(id),
                                                    m_idHelperSvc->mdtIdHelper().tube(id));

        if (dif.mag() > 0.5 * tubelen) {
            Amg::Vector3D newpos = tubePos - dif * (0.5 * tubelen / dif.mag());

            ATH_MSG_VERBOSE(">>>> extrapolated position outside volume, shifting position "
                            << m_idHelperSvc->toString(id) << ". position along strip " << dif.mag() << " 1/2 tube len "
                            << 0.5 * tubelen << " dist To Wire " << (piOnPlane - intersect).mag() << ". dist to newpos "
                            << (newpos - tubePos).mag() << " pos " << newpos);

            intersect = newpos;
        }
    } else {
        // not phi measurement, use tube center
        intersect = tubePos;
    }

    // enter hit in map
    Identifier elId = m_idHelperSvc->mdtIdHelper().elementID(id);

    MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(elId);
    int                       chFlag  = elId.get_identifier32().get_compact();
    if (m_doMultiAnalysis) {
        if (m_idHelperSvc->isSmallChamber(id)) {
            ATH_MSG_VERBOSE(" Small chamber " << m_idHelperSvc->toString(elId));
            chFlag = 0;
            if (chIndex == MuonStationIndex::BIS) {
                int eta = m_idHelperSvc->stationEta(elId);
                if (std::abs(eta) == 8) {
                    ATH_MSG_VERBOSE(" BIS8 chamber " << m_idHelperSvc->toString(elId));
                    chFlag = 3;
                }
            }
        } else {
            ATH_MSG_VERBOSE(" Large chamber " << m_idHelperSvc->toString(elId));
            chFlag = 1;
            if (chIndex == MuonStationIndex::BIL) {
                std::string stName = m_idHelperSvc->chamberNameString(id);
                if (stName[2] == 'R') {
                    ATH_MSG_VERBOSE(" BIR chamber " << m_idHelperSvc->toString(elId));
                    chFlag = 2;
                }
            } else if (chIndex == MuonStationIndex::BOL) {
                if (std::abs(m_idHelperSvc->stationEta(id)) == 7) {
                    ATH_MSG_VERBOSE(" BOE chamber " << m_idHelperSvc->toString(elId));
                    chFlag = 4;
                }
            }
        }
        int phi = m_idHelperSvc->mdtIdHelper().stationPhi(id);

        chFlag += 10 * phi;
    }
    // use center tube for region assignment
    int regionId = getRegionId(id);

    Region& region = regionMap[regionId];
    if (!region.init) {
        region.regionPos = patpose;
        region.regionDir = patdire;
        region.init = true;        
    }
    region.mdtPrdsPerChamber[chFlag].emplace_back(intersect, &mdt);
}

void
MuonPatternCalibration::printRegionMap(const RegionMap& regionMap) const
{

    ATH_MSG_INFO("Summarizing input");

    for (const auto& [detRegionId, chamberData] : regionMap) {
        ATH_MSG_INFO("new region " << detRegionId << " trigger " << chamberData.triggerPrds.size() << " mdt ch "
                                   << chamberData.mdtPrdsPerChamber.size());
        if (!chamberData.triggerPrds.empty()) ATH_MSG_INFO("trigger hits " << chamberData.triggerPrds.size());

        for (const auto& [globalPos, prd] : chamberData.triggerPrds) {
            ATH_MSG_INFO("  " << m_printer->print(*prd)<<" "<<globalPos);
        }
        for (const auto& [statId, MdtChamHits]: chamberData.mdtPrdsPerChamber) {
            ATH_MSG_INFO("new MDT chamber with " << MdtChamHits.size() << " hits");
            for (const auto& [globalPos, prd] : MdtChamHits) {
                ATH_MSG_INFO("  " << m_printer->print(*prd)<<" "<<globalPos);
            }
        }
    }
}

void
MuonPatternCalibration::calibrateRegionMap(const RegionMap& regionMap,
                                           IMuonPatternCalibration::ROTsPerRegion& hitsPerRegion) const {

    
    for (const auto& [regionId, regMeasColl] : regionMap) {

        ROTRegion rotRegion{};
        rotRegion.regionId  = regionId;
        rotRegion.regionPos = regMeasColl.regionPos;
        rotRegion.regionDir = regMeasColl.regionDir;
        for (const auto& [globalPos, prd] : regMeasColl.triggerPrds) {
            std::unique_ptr<const MuonClusterOnTrack> cluster{m_clusterCreator->createRIO_OnTrack(*prd, globalPos)};
            if (!cluster) continue;
            rotRegion.push_back(std::move(cluster));
        }
        for (const auto& [regionId, MdtsWithIsect] :regMeasColl.mdtPrdsPerChamber) {
            ATH_MSG_VERBOSE("Run over region id "<<regionId);
            MdtVec mdtROTs{};
            for (const auto& [globalPos, prd] : MdtsWithIsect) {  
                ATH_MSG_VERBOSE("Calibrate prd"<<m_idHelperSvc->toString(prd->identify())
                                <<",tdc: "<<prd->tdc()<<",adc: "<<prd->adc()<<" at "<<Amg::toString(globalPos));
                const MdtDriftCircleOnTrack* mdt = m_mdtCreator->createRIO_OnTrack(*prd, globalPos, &globalPos);
                if (!mdt) {
                    ATH_MSG_VERBOSE("Failed to calibrate " << m_idHelperSvc->toString(prd->identify()));
                    continue;
                }
                mdtROTs.push_back(mdt);
            }
            if (!mdtROTs.empty()) rotRegion.push_back(std::move(mdtROTs));
        }
        hitsPerRegion.push_back(std::move(rotRegion));
    }
}

template <class ContType> StatusCode MuonPatternCalibration::loadFromStoreGate(const EventContext& ctx,
                                                           const SG::ReadHandleKey<ContType>& key,
                                                           const ContType* & cont_ptr) const {
    if (key.empty()){
        ATH_MSG_VERBOSE("Empty key given for "<<typeid(ContType).name()<<".");
        cont_ptr = nullptr;
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle<ContType> readHandle{key, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve "<<key.fullKey()<<" from store gate");
        return StatusCode::FAILURE;
    }
    cont_ptr = readHandle.cptr();        
    return StatusCode::SUCCESS;
}

}  // namespace Muon
