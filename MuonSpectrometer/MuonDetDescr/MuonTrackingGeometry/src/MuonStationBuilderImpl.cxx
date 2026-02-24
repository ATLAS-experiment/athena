/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "MuonTrackingGeometry/MuonStationBuilderImpl.h"
#include "MuonTrackingGeometry/Utils.h"


#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <fstream>
#include <map>
#include <memory>

#include "AthenaKernel/IOVInfiniteRange.h"

#include "GeoModelHelpers/GeoVolumeUtils.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelHelpers/printVolume.h"
#include "GeoModelHelpers/TransformToStringConverter.h"


#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilderCond.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/RotatedDiamondBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/BoundarySurfaceFace.h"
#include "TrkVolumes/CuboidVolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/SimplePolygonBrepVolumeBounds.h"
#include "TrkVolumes/TrapezoidVolumeBounds.h"


namespace {
    std::pair<Amg::Vector3D, Amg::Vector3D> surroundingBox(const PVConstLink vol) {
        Amg::Vector3D min{1.e9 , 1.e9, 1.e9}, max{-1.e9, -1.e9, -1.e9};
        vol->getLogVol()->getShape()->extent(min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
        GeoVolumeCursor amArsch{vol};
        while (!amArsch.atEnd()) {
            const PVConstLink cv = amArsch.getVolume();
            auto [cMin, cMax] = surroundingBox(cv);
            cMin = amArsch.getTransform() * cMin;
            cMax = amArsch.getTransform() * cMax;
            
            min.x() = std::min(min.x(), cMin.x());
            min.y() = std::min(min.y(), cMin.y());
            min.z() = std::min(min.z(), cMin.z());

            max.x() = std::max(max.x(), cMax.x());
            max.y() = std::max(max.y(), cMax.y());
            max.z() = std::max(max.z(), cMax.z());
            amArsch.next();
        }
        return std::make_pair(min, max);
    }
}


namespace Muon{
using GMInfo = MuonStationBuilderImpl::GMInfo;

// Athena standard methods
// initialize
StatusCode MuonStationBuilderImpl::initialize() {
    // Retrieve the tracking volume helper
    // -------------------------------------------------
    ATH_CHECK(m_trackingVolumeHelper.retrieve());
   
    // Retrieve muon station builder tool
    // -------------------------------------------------
    ATH_CHECK(m_muonStationTypeBuilder.retrieve());
    // if no muon materials are declared, take default ones

    // default material properties
    m_muonMaterial = Trk::Material(10e10, 10e10, 0., 0., 0.);

    ATH_MSG_INFO(" initialize() successful");

    ATH_CHECK(m_idHelperSvc.retrieve());

    return StatusCode::SUCCESS;
}

MuonStationBuilderImpl::DetachedVolVec
    MuonStationBuilderImpl::buildDetachedTrackingVolumesImpl(const MuonGM::MuonDetectorManager* detMgr, bool /*blend*/) const {

    DetachedVolVec translatedStations{};

    // retrieve muon station branches from GeoModel tree
    std::vector<std::pair<const GeoVPhysVol*, std::vector<GMInfo>>> stations = retrieveGMsensitive(detMgr);

    for (const auto& [templateVol, volIds] : stations) {
        // build prototype
        std::unique_ptr<Trk::DetachedTrackingVolume> msType = buildDetachedTrackingVolumeType(templateVol,
                                                                                              volIds.front());
        if (!msType) {
            // ATH_MSG_WARNING("There is no prototype for \n"<<printVolume(templateVol));
            continue;
        }
        ATH_MSG_ALWAYS("Prototype constructed "<<templateVol->getLogVol()->getName()<<", distribute over "
                    <<volIds.size()<<" equivalent nodes");
        for (auto [volTrf, stationId] : volIds) {
            switch (m_idHelperSvc->technologyIndex(stationId)) {
                using enum MuonStationIndex::TechnologyIndex;
                case STGC:
                case MM: {                    
                    ATH_MSG_ALWAYS("Sarka wir muessen handeln "<<Amg::toString(msType->trackingVolume()->transform()));
                    Amg::Transform3D alignDelta{Amg::Transform3D::Identity()};
                    const auto* re = detMgr->getReadoutElement(stationId);
                    if (re->detectorType() == Trk::DetectorElemType::MM) {
                        alignDelta = dynamic_cast<const MuonGM::MMReadoutElement*>(re)->getDelta();
                    } else if (re->detectorType() == Trk::DetectorElemType::sTgc) {
                        alignDelta = dynamic_cast<const MuonGM::sTgcReadoutElement*>(re)->getDelta();
                    }
                    ATH_MSG_ALWAYS("Sarka wir muessen handeln "<<Amg::toString(msType->trackingVolume()->transform())
                                 <<", "<<Amg::toString(alignDelta));
                   
                    volTrf = volTrf * alignDelta;
                    std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msType->clone(m_idHelperSvc->toStringChamber(stationId), 
                                                                                       volTrf)};
                    // identify layer representation
                    Trk::Layer* layer = (newStat->layerRepresentation());
                    layer->setLayerType(stationId.get_identifier32().get_compact());
                    // identify layers
                    identifyLayers(*newStat, stationId, detMgr);
                    // collect
                    translatedStations.push_back(std::move(newStat));
                    break;
                } default: {
                    std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msType->clone(m_idHelperSvc->toStringChamber(stationId), volTrf)};
                    ATH_MSG_VERBOSE("Clone volume "<<msType->name()<<", "<<GeoTrf::toString(volTrf)<<" -> "
                                <<newStat->name());
                    
                    // identify layer representation
                    Trk::Layer* layer = newStat->layerRepresentation();
                    
                    layer->setLayerType(stationId.get_identifier32().get_compact());
                    // glue components
                    glueComponents(newStat.get());
                    // identify layers
                    identifyLayers(*newStat, stationId, detMgr);
                    // collect
                    translatedStations.push_back(std::move(newStat));
                }
            }
        }
    }  // end loop over prototypes

    ATH_MSG_DEBUG( "returns " << translatedStations.size() << " stations");
    THROW_EXCEPTION("Aus die Maus");
    return translatedStations;
}

void MuonStationBuilderImpl::glueComponents(Trk::DetachedTrackingVolume* stat) const {
    Trk::TrackingVolumeArray* volArray = stat->trackingVolume()->confinedVolumes();
    if (!volArray || volArray->arrayObjectsNumber() <= 1) {
        return;
    }

    std::span<Trk::TrackingVolume* const> components =  volArray->arrayObjects();
    const Trk::BinUtility* binUtilityX = volArray->binUtility();
    const Trk::CuboidVolumeBounds* cubVolBounds = dynamic_cast<const Trk::CuboidVolumeBounds*>(&(components[0]->volumeBounds()));

    // identify 'lower' and 'upper' boundary surface
    Trk::BoundarySurfaceFace low = Trk::negativeFaceXY;
    Trk::BoundarySurfaceFace up = Trk::positiveFaceXY;

    // rectangular station in x ordering (MDT barrel)
    if (cubVolBounds && binUtilityX) {
        low = Trk::negativeFaceYZ;
        up = Trk::positiveFaceYZ;
    }

    if (low >= 0 && up >= 0) {
        // glue volumes
        for (unsigned int i = 0; i < components.size() - 1; ++i) {
            m_trackingVolumeHelper->glueTrackingVolumes(*(components[i]), up, *(components[i + 1]), low);
        }
    }
}

std::vector<const Trk::Surface*> MuonStationBuilderImpl::fetchSurfaces(const Identifier& stationId,
                                                                       const MuonGM::MuonDetectorManager* detMgr) const {
    std::vector<const Trk::Surface*> surfaces{};
    
    auto appendSurfaces = [&surfaces, this](const MuonGM::MuonClusterReadoutElement* stripRE) {
        std::ranges::copy_if(stripRE->surfaces(), std::back_inserter(surfaces), 
                                     [this](const Trk::Surface* surface){
                                        return !m_idHelperSvc->measuresPhi(surface->associatedDetectorElementIdentifier());
                                     });
    };
    
    const MuonGM::MuonReadoutElement* re = detMgr->getReadoutElement(stationId);
    ATH_MSG_ALWAYS("Fetch all eta surfaces for "<<m_idHelperSvc->toStringChamber(stationId));

    if (re->detectorType() == Trk::DetectorElemType::Tgc ||
        re->detectorType() == Trk::DetectorElemType::MM ||
        re->detectorType() == Trk::DetectorElemType::sTgc) {
        appendSurfaces(static_cast<const MuonGM::MuonClusterReadoutElement*>(re));
    } else if (re->parentMuonStation()) {
        for (const MuonGM::MuonReadoutElement* chRE : re->parentMuonStation()->getReadoutElements()) {
            ATH_MSG_ALWAYS("Readout element "<<m_idHelperSvc->toStringDetEl(chRE->identify())
                         <<" is in the same station");
            if (chRE->detectorType() != Trk::DetectorElemType::Mdt) {
                appendSurfaces(static_cast<const MuonGM::MuonClusterReadoutElement*>(chRE));
            } else {
                const auto* tubeRE = static_cast<const MuonGM::MdtReadoutElement*>(chRE);
                for (int tL = 1; tL <= tubeRE->getNLayers(); ++tL) {
                    surfaces.push_back(&tubeRE->surface(tL,1));
                }   
            }
        }
    } else {
        THROW_EXCEPTION("Don't know how to retrieve elements from "<<m_idHelperSvc->toStringChamber(stationId));
    }
    return surfaces;
}


void MuonStationBuilderImpl::identifyLayers(
    Trk::TrackingVolume& station, 
    const Identifier& stationID,  
    const MuonGM::MuonDetectorManager* detMgr) const {

  for (const Trk::Surface* surface : fetchSurfaces(stationID, detMgr)) {
        const Amg::Vector3D& gpi = surface->center();
        const Identifier surfId = surface->associatedDetectorElementIdentifier();
        Trk::TrackingVolume* assocVol = station.associatedSubVolume(gpi);

        if (!assocVol) {
            THROW_EXCEPTION("There is no associated tracking volume for "<<m_idHelperSvc->toString(surfId));
        } 
        ATH_MSG_ALWAYS("Retrieve gas gap for "<<m_idHelperSvc->toStringGasGap(surfId)<<", @"
                        <<Amg::toString(gpi)
                       <<", local: "<<Amg::toString(assocVol->transform().inverse()*gpi));
        Trk::Layer* assocLay = assocVol->associatedLayer(gpi);
        if (!assocLay) {
            if (assocVol->confinedLayers()) {
            for (const auto lay : assocVol->confinedLayers()->arrayObjects()) {
                ATH_MSG_ERROR("Stonjek "<<Amg::toString(lay->surfaceRepresentation().transform().inverse()*gpi)
                            <<", "<<lay->isOnLayer(gpi));
            }
            }
            for (const auto lay : assocVol->confinedArbitraryLayers()) {
                ATH_MSG_ERROR("Stonjek "<<Amg::toString(lay->surfaceRepresentation().transform().inverse()*gpi)
                            <<", "<<lay->isOnLayer(gpi));
            }

            // confinedArbitraryLayers

            THROW_EXCEPTION("There is no associated tracking layer for "<<m_idHelperSvc->toStringGasGap(surfId)
                            <<", "<<GeoTrf::toString(assocVol->transform().inverse() * surface->transform())
                            <<", "<<Amg::toString(assocVol->transform().inverse()* gpi));
        }
        assocLay->setLayerType(surfId.get_identifier32().get_compact());
    }
}

void MuonStationBuilderImpl::identifyLayers(
    Trk::DetachedTrackingVolume& station, 
    const Identifier& stationID,  
    const MuonGM::MuonDetectorManager* detMgr) const {
    ATH_MSG_VERBOSE( " Start layer identifiacation ");

    ATH_MSG_VERBOSE(" in station " << station.name());
    identifyLayers(*station.trackingVolume(), stationID, detMgr);
    return;
    for (const Trk::Surface* surface : fetchSurfaces(stationID, detMgr)) {
        const Amg::Vector3D& gpi = surface->center();
        const Identifier surfId = surface->associatedDetectorElementIdentifier();
        Trk::TrackingVolume* assocVol = station.trackingVolume()->associatedSubVolume(gpi);

        if (!assocVol) {
            THROW_EXCEPTION("There is no associated tracking volume for "<<m_idHelperSvc->toString(surfId));
        } 
        ATH_MSG_ALWAYS("Retrieve gas gap for "<<m_idHelperSvc->toStringGasGap(surfId)<<", @"
                        <<Amg::toString(gpi)
                       <<", local: "<<Amg::toString(assocVol->transform().inverse()*gpi));
        Trk::Layer* assocLay = assocVol->associatedLayer(gpi);
        if (!assocLay) {
            THROW_EXCEPTION("There is no associated tracking layer for "<<m_idHelperSvc->toStringGasGap(surfId)
                            <<", "<<GeoTrf::toString(assocVol->transform().inverse() * surface->transform())
                            <<", "<<Amg::toString(assocVol->transform().inverse()* gpi));
        }
        assocLay->setLayerType(surfId.get_identifier32().get_compact());
    }

    // by now, all the layers should be identified - verify
    if (station.trackingVolume()->confinedVolumes()) {
        std::span<Trk::TrackingVolume* const> cVols =
            station.trackingVolume()->confinedVolumes()->arrayObjects();
        for (auto* cVol : cVols) {
            if (cVol->confinedLayers()) {
                std::span<Trk::Layer* const> cLays =
                    cVol->confinedLayers()->arrayObjects();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1)
                        ATH_MSG_DEBUG(station.name()
                                      << "," << cVol->volumeName()
                                      << ", unidentified active layer:" << il);
                    else if (id != 0)
                        checkLayerId("check layer in " + station.name() +
                                         " subvolume " + cVol->volumeName(),
                                     detMgr, id, cLays[il]);
                }
            }
            if (!cVol->confinedArbitraryLayers().empty()) {
                Trk::ArraySpan<Trk::Layer* const> cLays =
                    cVol->confinedArbitraryLayers();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1)
                        ATH_MSG_DEBUG(station.name()
                                      << "," << cVol->volumeName()
                                      << ", unidentified active layer:" << il);
                    else if (id != 0)
                        checkLayerId("check arbitrary layer in " +
                                         station.name() + " subvolume " +
                                         cVol->volumeName(),
                                     detMgr, id, cLays[il]);
                }
            }
        }
    }
    if (station.trackingVolume()->confinedLayers()) {
        std::span<Trk::Layer* const> cLays =
            station.trackingVolume()->confinedLayers()->arrayObjects();
        for (unsigned int il = 0; il < cLays.size(); il++) {
            Identifier id(cLays[il]->layerType());
            if (id == 1)
                ATH_MSG_DEBUG(station.name()                            
                              << ", unidentified active layer:" << il);
            else if (id != 0)
                checkLayerId("check confined layer in " + station.name(),
                             detMgr, id, cLays[il]);
        }
    }
    // end identification check

}

std::vector<std::pair<const GeoVPhysVol*, std::vector<GMInfo>>>
MuonStationBuilderImpl::retrieveGMsensitive(const MuonGM::MuonDetectorManager* detMgr) const {

    // a single loop over GM tree to retrieve all necessary information

    std::vector<std::pair<const GeoVPhysVol*,
                          std::vector<GMInfo>>> sensitive;

    std::vector<PVConstLink> sensitiveVec{};
    std::unordered_map<const GeoVPhysVol*, Identifier> idMap{};
    // Retrieve all elements and fetch the station envelopes
    for (const MuonGM::MuonReadoutElement* re : detMgr->getAllReadoutElements()) {
        PVConstLink station = re->getMaterialGeom()->getParent();
        Identifier chId = m_idHelperSvc->chamberId(re->identify());

        if (re->detectorType() == Trk::DetectorElemType::MM ||
            re->detectorType() == Trk::DetectorElemType::sTgc ||
            re->detectorType() == Trk::DetectorElemType::Tgc) {
            station = re->getMaterialGeom();
            chId = re->identify();
        }
        
        ATH_MSG_DEBUG("Append new station volume "<<station->getLogVol()->getName()<<", "<<m_idHelperSvc->toString(chId));
        if (idMap.insert(std::make_pair(station, chId)).second){
            // Keep a vector to ensure the same layout of the geometry tree
            sensitiveVec.push_back(station);
        }
    }
    ATH_MSG_DEBUG("Fetched "<<idMap.size()<<" stations to embed into the tracking geometry.");

    for (const PVConstLink& stationVol : sensitiveVec) {
        const Amg::Transform3D locToGlob = volumePosInSpace(stationVol);
        /** Check whether there is already a prototype volume stored (i.e. same structure)
            but at a different location. */
        auto it = std::ranges::find_if(sensitive, [stationVol,this](const auto& gmInfo){
            return m_gmBrowser.compareGeoVolumes(stationVol, gmInfo.first, 1.e-3) == 0;
        });

        if (it == sensitive.end()) {
            std::vector<GMInfo> cloneList{std::make_tuple(locToGlob, idMap.at(stationVol))};
            sensitive.emplace_back(stationVol, std::move(cloneList));
        } else {
            // order transforms to position prototype at phi=0/ 0.125 pi
            double phiTr = locToGlob.translation().phi();
            if (phiTr > -0.001 && phiTr < 0.4) {
                it->second.insert(it->second.begin(),
                                  std::make_tuple(locToGlob, idMap.at(stationVol)));
            } else {
                it->second.emplace_back(std::make_tuple(locToGlob, idMap.at(stationVol)));
            }
        }
    }
    if (msgLvl(MSG::DEBUG)) {
        std::stringstream sstr{};
        unsigned  allVols{0};
        for (const auto& [vol, subtree]: sensitive){
            allVols += subtree.size();
            sstr<<"Volume "<<vol->getLogVol()->getName()<<", prototypes: "<<subtree.size()<<"."<<std::endl;
            for (const auto& [trf, id] : subtree) {
                sstr<<" --- trf: "<<Amg::toString(trf)<<", "<<m_idHelperSvc->toString(id)<<std::endl;
            }
        }
        ATH_MSG_DEBUG("Number of muon station types in GeoModel tree: " << sensitive.size()<<", found volumes: "
                    <<allVols<<"\n"<<sstr.str());
    }
    return sensitive;
}

std::unique_ptr<Trk::DetachedTrackingVolume>
MuonStationBuilderImpl::buildDetachedTrackingVolumeType(const GeoVPhysVol* cv,
                                                        const GMInfo& gmInfo) const {
    const GeoLogVol* clv = cv->getLogVol();
    const std::string& vname = clv->getName();
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    MuonStationTypeBuilder::Cache cache{};

    const auto& [locToGlob, volId] = gmInfo;
    const std::string stName = m_idHelperSvc->stationNameString(volId);
    
    if (stName == "BIS" && m_idHelperSvc->stationEta(volId) == 7 ) {
        return nullptr;
    } 
    // if (stName == "BMS" && std::abs(m_idHelperSvc->stationEta(volId)) == 4) {
    //     return nullptr;
    // }
    ATH_MSG_ALWAYS(" Building station prototype for " << vname<<", "<<m_idHelperSvc->toStringChamber(volId)
                <<"\n"<<printVolume(cv) );

    switch (m_idHelperSvc->technologyIndex(volId)) {
        using enum MuonStationIndex::TechnologyIndex;
        case STGC:
        case MM:
           return m_muonStationTypeBuilder->process_NSW(volId, cv, locToGlob);
        case TGC: {
            if (!m_buildTgc) {
                return nullptr;
            } 
            return nullptr;
            std::unique_ptr<Trk::TrackingVolume> tgc_station{m_muonStationTypeBuilder->processTgcStation(cv, cache)};
            // create layer representation
            auto layerRepr =  m_muonStationTypeBuilder->createLayerRepresentation(*tgc_station);
            // create prototype as detached tracking volume
            auto layerVec = std ::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
            return std::make_unique<Trk::DetachedTrackingVolume>(stName, std::move(tgc_station),
                                                                 std::move(layerRepr.first), std::move(layerVec));
       
        } case CSC:{
            if (!m_buildCsc) {
                return nullptr;
            }
            auto csc_station = m_muonStationTypeBuilder->processCscStation(cv, stName, cache);
            // create layer representation
            auto layerRepr = m_muonStationTypeBuilder->createLayerRepresentation(*csc_station);
            // create prototype as detached tracking volume
            auto layerVec = std ::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
            return std::make_unique<Trk::DetachedTrackingVolume>(stName, std::move(csc_station),
                                                                 std::move(layerRepr.first), std::move(layerVec));

        } case RPC:
          case MDT: {
            if ( (!m_buildBarrel && !m_idHelperSvc->isEndcap(volId)) ||
                 (!m_buildEndcap &&  m_idHelperSvc->isEndcap(volId))){
                return nullptr;
            }
            break;
        } default:
            return nullptr;
    }

    auto shapeS = compressShift(clv->getShape());

    double halfX1{0.}, halfX2{0.}, halfY1{0.}, halfY2{0.}, halfZ{0.};

    while (shapeS->typeID() != GeoTrd::getClassTypeID() &&
           shapeS->typeID() != GeoBox::getClassTypeID()) {
        if (shapeS->typeID() == GeoShapeShift::getClassTypeID()) {
            const auto shift = dynamic_pointer_cast<const GeoShapeShift>(shapeS);
            shapeS = shift->getOp();
        } else if (shapeS->typeID() == GeoShapeSubtraction::getClassTypeID()) {
            const auto sub = dynamic_pointer_cast<const GeoShapeSubtraction>(shapeS);
            shapeS = sub->getOpA();
        } else if (shapeS->typeID() == GeoShapeUnion::getClassTypeID()) {
            const auto uni = dynamic_pointer_cast<const GeoShapeUnion>(shapeS);
            shapeS = uni->getOpA();
        } else {
            break;
        }
    }


   
    const auto [min, max] = surroundingBox(cv);
 
    halfX1 = halfX2 = 0.5*(max.x() - min.x());
    halfY1 = halfY2 = 0.5*(max.y() - min.y());
    halfZ = 0.5*(max.z() - min.z());
    ATH_MSG_ALWAYS("Surrounding box :"<<halfX1<<", "<<halfY1<<", "<<halfZ<<"  -> "<<
                    Amg::toString(Amg::getTranslate3D(0.5*(min+max))));
    if (shapeS->typeID() == GeoTrd::getClassTypeID()) {
        const auto trd = dynamic_pointer_cast<const GeoTrd>(shapeS);
        halfX1 = trd->getXHalfLength1();
        halfX2 = trd->getXHalfLength2();
        halfY1 = trd->getYHalfLength1();
        halfY2 = trd->getYHalfLength2();
        halfZ = trd->getZHalfLength();
    } else if (shapeS->typeID() == GeoBox::getClassTypeID()) {
        const auto box = dynamic_pointer_cast<const GeoBox>(shapeS);
        halfX1 = halfX2 = box->getXHalfLength();
        halfY1 = halfY2 = box->getYHalfLength();
        halfZ  = box->getZHalfLength();
                

    } else {
        ATH_MSG_WARNING("The shape from volume neither decays to a box or trd \n"
                        <<printGeoShape(clv->getShape()));
        return nullptr;
    }
    // define enveloping volume
    std::unique_ptr<Trk::TrackingVolumeArray> confinedVolumes{};
    std::vector<std::unique_ptr<Trk::Layer>> confinedLayers{};
    std::unique_ptr<Trk::Volume> envelope;
    std::string shape = "Trd";
    if (halfX1 == halfX2 && halfY1 == halfY2) {
        shape = "Box";
    }

    if (shape == "Box") {
        auto envBounds = std::make_shared<Trk::CuboidVolumeBounds>(halfX1, halfY1, halfZ);
        ATH_MSG_ALWAYS("Stonjek "<<(*envBounds));
        // station components
        confinedVolumes = m_muonStationTypeBuilder->processBoxStationComponents(cv, *envBounds, cache);
        if (!confinedVolumes) {
            confinedLayers = m_muonStationTypeBuilder->processBoxComponentsArbitrary(cv, *envBounds, cache);
        }
        // enveloping volume
        envelope = std::make_unique<Trk::Volume>(nullptr, std::move(envBounds));
    } else if (shape == "Trd") {
        std::unique_ptr<Trk::TrapezoidVolumeBounds> envBounds{};
        Amg::Transform3D transf{Amg::Transform3D::Identity()};
        if (halfY1 == halfY2) {
            envBounds = std::make_unique<Trk::TrapezoidVolumeBounds>(halfX1, halfX2, halfY1, halfZ);
            ATH_MSG_VERBOSE("CAUTION!!!: this trapezoid volume does not require XY -> YZ switch");
        }
        if (halfY1 != halfY2 && halfX1 == halfX2) {
            transf = Amg::getRotateY3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
            envBounds = std::make_unique<Trk::TrapezoidVolumeBounds>(halfY1, halfY2, halfZ, halfX1);
        }
        if (halfX1 != halfX2 && halfY1 != halfY2) {
            ATH_MSG_WARNING("station envelope arbitrary trapezoid?"<< stName);
        }
        if (envBounds) {
            // station components
            confinedVolumes = m_muonStationTypeBuilder->processTrdStationComponents(cv, *envBounds, cache);
            // enveloping volume
            envelope = std::make_unique<Trk::Volume>(makeTransform(transf), std::move(envBounds));
        }
    }
    if (!envelope) {
        ATH_MSG_WARNING("Failed to construct a tracking volume from \n"<<printVolume(cv));
        return nullptr;
    }

       
    // ready to build the station prototype
    std::unique_ptr<Trk::TrackingVolume> newType{};
    if (!confinedLayers.empty()) {
        auto confinedLayerPtr = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(confinedLayers));
        newType = std::make_unique<Trk::TrackingVolume>(*envelope, m_muonMaterial, std::move(confinedLayerPtr), stName);
    } else {
        newType = std::make_unique<Trk::TrackingVolume>(*envelope, m_muonMaterial, nullptr, std::move(confinedVolumes), stName);
    }
    // create layer representation
    auto layerRepr = m_muonStationTypeBuilder->createLayerRepresentation(*newType);

    // create prototype as detached tracking volume
    auto layerVec = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
    ATH_MSG_DEBUG(" station prototype built for " << vname);

    return std::make_unique<Trk::DetachedTrackingVolume>(stName, std::move(newType),
                                                                 std::move(layerRepr.first),
                                                                 std::move(layerVec));
}

void MuonStationBuilderImpl::checkLayerId(std::string_view comment, const MuonGM::MuonDetectorManager* detMgr,
    Identifier id, const Trk::Layer* lay) const {

    // RE
    if (m_idHelperSvc->isMdt(id)) {
        const MuonGM::MdtReadoutElement* mdtRE = detMgr->getMdtReadoutElement(id);
        constexpr double tol = 0.5*Gaudi::Units::mm;
        if (mdtRE && !lay->surfaceRepresentation().isOnSurface(mdtRE->transform(id).translation(), tol, tol)) {
            ATH_MSG_DEBUG(__FILE__<<":"<<__LINE__<<" "<<comment << ":tube(id) "
                        <<m_idHelperSvc->toString(id)<<" "<<Amg::toString(mdtRE->transform(id).translation())
                        <<" not on surface:"<<lay->surfaceRepresentation()<<std::endl<<
                        " "<<Amg::toString(lay->surfaceRepresentation().transform().inverse()*(mdtRE->transform(id).translation())) );
        }
    } else if (m_idHelperSvc->isRpc(id)) {
        const MuonGM::RpcReadoutElement* rpcRE = detMgr->getRpcReadoutElement(id);
        Amg::Transform3D trid = rpcRE->transform(id);
        Amg::Transform3D check_layer_identity = lay->surfaceRepresentation().transform().inverse() * trid;
        if (!Amg::doesNotDeform(check_layer_identity)) {
            ATH_MSG_DEBUG(__FILE__<<":"<<__LINE__<<" "<<comment<<" "<<Amg::toString(check_layer_identity));
        }

    } else if (m_idHelperSvc->isTgc(id)) {
        const Amg::Transform3D trid = detMgr->getTgcReadoutElement(id)->transform(id);
        Amg::Transform3D check_layer_identity = lay->surfaceRepresentation().transform().inverse() * trid;
        if (!Amg::doesNotDeform(check_layer_identity)) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<comment <<" "<<Amg::toString(check_layer_identity));
        }
    }
}
}
