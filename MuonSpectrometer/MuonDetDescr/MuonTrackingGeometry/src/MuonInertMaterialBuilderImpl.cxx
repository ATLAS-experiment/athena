/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Muon
#include "MuonTrackingGeometry/MuonInertMaterialBuilderImpl.h"

// MuonSpectrometer include
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrackingGeometry/MuonStationTypeBuilder.h"

// STD
#include <fstream>
#include <iostream>
#include <map>

#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoVolumeCursor.h"

// constructor
Muon::MuonInertMaterialBuilderImpl::MuonInertMaterialBuilderImpl(
    const std::string& t, const std::string& n, const IInterface* p)
    : AthAlgTool(t, n, p), Trk::TrackingVolumeManipulator() {}

// Athena standard methods
// initialize
StatusCode Muon::MuonInertMaterialBuilderImpl::initialize() {

    if (m_simplifyToLayers) {
        ATH_MSG_INFO(" option Simplify(Muon)GeometryToLayers no longer maintained ");
    }
    ATH_MSG_INFO( " initialize() successful");
    return StatusCode::SUCCESS;
}

Muon::MuonInertMaterialBuilderImpl::DetachedVolVec
    Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumesImpl(const PVConstLink treeTop, bool blend) const {
    
    if (!treeTop) {
        throw std::runtime_error("No tree top has been parsed");
    }
    // collect inert material objects
    DetachedVolVec mInert{};
    // retrieve muon station prototypes from GeoModel
    auto msTypes = buildDetachedTrackingVolumeTypes(treeTop, blend);
    ATH_MSG_DEBUG(" obtained " << msTypes.size() << " prototypes");

    
    for (auto& [msTV, transforms]: msTypes) {
        std::string msTypeName = msTV->name();
        for (Amg::Transform3D&  combTr : transforms) {
            std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msTV->clone(msTypeName, combTr)};
            mInert.push_back(std::move(newStat));
        }
    }
    ATH_MSG_DEBUG(" returns  " << mInert.size() << " objects (detached volumes)");

    return mInert;
}


Muon::MuonInertMaterialBuilderImpl::DetachedVolumeVecWithTrfs
    Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumeTypes(const PVConstLink top, bool blend) const {
  
    DetachedVolumeVecWithTrfs objs{};
    /// link to top tree
    GeoVolumeCursor vol(top);
    while (!vol.atEnd()) {
        const GeoVPhysVol* cv = vol.getVolume();
        const GeoLogVol* clv = cv->getLogVol();
        const std::string_view vname = clv->getName();
        if (vname.size() > 7 && vname.substr(vname.size() - 7, 7) ==
                                    "Station") {  // do nothing, active station
        } else {
            bool accepted = true;
            if (vname.substr(0, 3) == "BAR" || vname.substr(0, 2) == "BT" ||
                vname.substr(0, 6) == "EdgeBT" || vname.substr(0, 6) == "HeadBT"){
                accepted = m_buildBT;
            } else if (vname.substr(0, 3) == "ECT") {
                accepted = m_buildECT;
            } else if (vname.substr(0, 4) == "Feet" ||
                     (vname.size() > 7 && (vname.substr(3, 4) == "Feet" || vname.substr(4, 4) == "Feet"))) {
                accepted = m_buildFeets;
            } else if (vname.substr(0, 4) == "Rail") {
                accepted = m_buildRails > 0;
            } else if (vname.substr(0, 1) == "J") {
                accepted = m_buildShields > 0;
            // NSW build inertmaterial for spacer frame, aluminium HUB, NJD disk
            // and A plate
            } else if (vname.substr(0, 3) == "NSW" && vname.substr(1, 6) == "Spacer") {
                accepted = m_buildNSWInert;
            } else if (vname.substr(0, 3) == "NSW" && vname.substr(1, 2) == "Al") {
                accepted = m_buildNSWInert;
            } else if (vname.substr(0, 3) == "NJD"){
                accepted = m_buildNSWInert;
            } else if (vname.substr(0, 1) == "A" && vname.substr(1, 5) == "Plate"){
                accepted = m_buildNSWInert;
            // strange NSW will be anyway build
            } else if (vname.substr(0, 1) != "J") {
                accepted = m_buildSupports > 0;
            }
            if (!accepted) {
                ATH_MSG_VERBOSE(" INERT muon object found and rejected :" << vname);
                vol.next();
                continue;
            }
            ATH_MSG_VERBOSE(" INERT muon object found and accepted :" << vname);

            if (msg().level() == MSG::VERBOSE)
                printInfo(cv);

            std::vector<std::pair<const GeoVPhysVol*, std::vector<Amg::Transform3D>>> vols;

            bool simpleTree = false;
            if (!cv->getNChildVols()) {
                if (!m_gmBrowser.findNamePattern(cv, "Sensitive")) {
                    std::vector<Amg::Transform3D> volTr;
                    volTr.push_back(vol.getTransform());
                    vols.emplace_back(cv, volTr);
                    simpleTree = true;
                }
            } else {
                getObjsForTranslation(cv, Amg::Transform3D::Identity(), vols);
            }

            for (auto& [physVol, physVolTrfs]: vols) {
                std::string protoName(vname);
                if (!simpleTree)
                    protoName += physVol->getLogVol()->getName();
                ATH_MSG_VERBOSE(" check in:"<< protoName << ", made of "
                    << physVol->getLogVol()->getMaterial()->getName()
                    << " x0 " << physVol->getLogVol()->getMaterial()->getRadLength()
                    << "," << physVol->getLogVol()->getShape()->type());

                bool found = false;
                for (auto& obj : objs) {
                    if (protoName == obj.first->name()) {  // found in another branch already ?
                        found = true;
                        if (simpleTree) {
                            obj.second.push_back(vol.getTransform());
                        } else {
                            obj.second.insert(obj.second.end(), 
                                              std::make_move_iterator(physVolTrfs.begin()),
                                              std::make_move_iterator(physVolTrfs.end()));
                        }
                    }
                }
                if (found) {
                    continue;
                }
                // envelope creation & simplification done with
                // TrkDetDescrGeoModelCnv helpers
                auto newType = m_volumeConverter.translate(physVol, m_simplify, blend, m_blendLimit);              
                if (newType) {
                    const std::string volName = newType->volumeName();
                    auto typeDet = std::make_unique<Trk::DetachedTrackingVolume>(volName, newType.release());
                    objs.emplace_back(std::move(typeDet), std::move(physVolTrfs));
                } else {
                    ATH_MSG_WARNING("volume not translated: " << vname);
                }
            }  // end new object
        }
        vol.next();
    }

    int count = 0;
    for (auto& obj : objs)
        count += obj.second.size();

    ATH_MSG_DEBUG(" returns " << objs.size()<< " prototypes, to be cloned into " << count << " objects");

    return objs;
}

void Muon::MuonInertMaterialBuilderImpl::printInfo(
    const GeoVPhysVol* pv) const {
    const GeoLogVol* lv = pv->getLogVol();
    ATH_MSG_VERBOSE("New Muon Inert Object:"
                    << lv->getName() << ", made of "
                    << lv->getMaterial()->getName() << " x0 "
                    << lv->getMaterial()->getRadLength() << ","
                    << lv->getShape()->type());
    m_geoShapeConverter.decodeShape(lv->getShape());
    printChildren(pv);
}

void Muon::MuonInertMaterialBuilderImpl::printChildren(
    const GeoVPhysVol* pv) const {
    // subcomponents
    unsigned int nc = pv->getNChildVols();
    for (unsigned int ic = 0; ic < nc; ic++) {
        Amg::Transform3D transf = pv->getXToChildVol(ic);

        const GeoVPhysVol* cv = &(*(pv->getChildVol(ic)));
        const GeoLogVol* clv = cv->getLogVol();
        ATH_MSG_VERBOSE("  ");
        ATH_MSG_VERBOSE(
            "subcomponent:"
            << ic << ":" << clv->getName() << ", made of "
            << clv->getMaterial()->getName() << " x0 "
            << clv->getMaterial()->getRadLength() << " , "
            << clv->getShape()->type() << "," << transf.translation().x() << " "
            << transf.translation().y() << " " << transf.translation().z());

        m_geoShapeConverter.decodeShape(clv->getShape());

        printChildren(cv);
    }
}

void Muon::MuonInertMaterialBuilderImpl::getObjsForTranslation(
    const GeoVPhysVol* pv, const Amg::Transform3D& transform,
    std::vector<std::pair<const GeoVPhysVol*, std::vector<Amg::Transform3D>>>&
        vols) const {
    // subcomponents
    unsigned int nc = pv->getNChildVols();
    ATH_MSG_VERBOSE(" INERT getObjsForTranslation from:"
                    << pv->getLogVol()->getName() << ","
                    << pv->getLogVol()->getMaterial()->getName()
                    << ", looping over " << nc << " children");
    for (unsigned int ic = 0; ic < nc; ic++) {
        Amg::Transform3D transf = pv->getXToChildVol(ic);
        const GeoVPhysVol* cv = &(*(pv->getChildVol(ic)));
        const GeoLogVol* clv = cv->getLogVol();
        if (clv->getMaterial()->getDensity() > 0. &&
            m_gmBrowser.findNamePattern(cv, "Sensitive"))
            continue;  // skip sensitive material branches
        if (!cv->getNChildVols()) {
            bool found = false;
            for (auto& vol : vols) {
                if (clv->getName() == vol.first->getLogVol()->getName()) {
                    if (m_gmBrowser.compareGeoVolumes(cv, vol.first, 1.e-3) !=
                        0)
                        ATH_MSG_WARNING(
                            "INERT name branch matching differences detected "
                            "in:"
                            << clv->getName());
                    found = true;
                    vol.second.push_back(transform * transf);
                    break;
                }
            }
            if (!found) {
                std::vector<Amg::Transform3D> volTr;
                volTr.push_back(transform * transf);
                vols.emplace_back(cv, volTr);
                ATH_MSG_VERBOSE("INERT new volume added:"
                                << clv->getName() << ","
                                << clv->getMaterial()->getName());
                if (msg().level() <= MSG::VERBOSE)
                    printInfo(cv);
            }
        } else {
            getObjsForTranslation(cv, transform * transf, vols);
        }
    }
}
