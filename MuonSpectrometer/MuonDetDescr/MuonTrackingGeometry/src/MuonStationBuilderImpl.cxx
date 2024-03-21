/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackingGeometry/MuonStationBuilderImpl.h"
#include "MuonTrackingGeometry/Utils.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include <fstream>
#include <map>

#include "AthenaKernel/IOVInfiniteRange.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "GeoPrimitives/GeoPrimitives.h"
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
#include "TrkDetDescrUtils/SharedObject.h"
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


// constructor
Muon::MuonStationBuilderImpl::MuonStationBuilderImpl(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
    : AthAlgTool(t, n, p) {}

// Athena standard methods
// initialize
StatusCode Muon::MuonStationBuilderImpl::initialize() {
    // Retrieve the tracking volume helper
    // -------------------------------------------------
    ATH_CHECK(m_trackingVolumeHelper.retrieve());
    ATH_MSG_INFO("Retrieved tool " << m_trackingVolumeHelper);

    // Retrieve muon station builder tool
    // -------------------------------------------------
    ATH_CHECK(m_muonStationTypeBuilder.retrieve());
    ATH_MSG_INFO("Retrieved tool " << m_muonStationTypeBuilder);

    // if no muon materials are declared, take default ones

    // default material properties
    m_muonMaterial = Trk::Material(10e10, 10e10, 0., 0., 0.);  

    ATH_MSG_INFO(" initialize() successful");

    ATH_CHECK(m_idHelperSvc.retrieve());

    return StatusCode::SUCCESS;
}

Muon::MuonStationBuilderImpl::DetachedVolVec
    Muon::MuonStationBuilderImpl::buildDetachedTrackingVolumesImpl(const MuonGM::MuonDetectorManager* muonMgr, bool /*blend*/) const {

    DetachedVolVec translatedStations{};

    // retrieve muon station branches from GeoModel tree
    std::vector<std::pair<const GeoVPhysVol*, std::vector<Muon::GMInfo>>> stations = retrieveGMsensitive(muonMgr);

    for (const auto& mstation : stations) {
        // build prototype
        std::unique_ptr<Trk::DetachedTrackingVolume> msType =  buildDetachedTrackingVolumeType(muonMgr, mstation.first,
                                                                                               mstation.second[0]);
        if (!msType) {
            continue;
        }
        // clone prototype
        std::string name = mstation.first->getLogVol()->getName();
        // NSW prototypes built at position
        if (name.substr(0, 4) == "sTGC" || name.substr(0, 2) == "MM") {
            for (unsigned int i = 0; i < mstation.second.size(); i++) {
                std::string sName = name.substr(name.find('-') + 1);
                Identifier nswId = m_muonStationTypeBuilder->identifyNSW(sName, mstation.second[i].first);
                // clone station from prototype
                Amg::Transform3D trdef(mstation.second[i].first *
                                       mstation.second[0].first.inverse());
                std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msType->clone(name, trdef)};
                // identify layer representation
                Trk::Layer* layer = (newStat->layerRepresentation());
                layer->setLayerType(nswId.get_identifier32().get_compact());
                // identify layers
                identifyNSWLayers(*newStat, nswId);
                // collect
                translatedStations.push_back(std::move(newStat));
            }  // end clone NSW stations
        } else {
            for (auto gminfo : mstation.second) {
                // clone station from prototype
                std::unique_ptr<Trk::DetachedTrackingVolume> newStat{msType->clone(name, gminfo.first)};
                // identify layer representation
                Trk::Layer* layer = newStat->layerRepresentation();
                int eta{0}, phi{0};
                Identifier stId = resolveId(name, gminfo, eta, phi, muonMgr);
                layer->setLayerType(stId.get_identifier32().get_compact());
                // glue components
                glueComponents(newStat.get());
                // identify layers
                identifyLayers(newStat.get(), stId, eta, phi, muonMgr);
                // collect
                translatedStations.push_back(std::move(newStat));
            }  // end clone non-NSW
        }
    }  // end loop over prototypes

    ATH_MSG_DEBUG( "returns " << translatedStations.size() << " stations");
    return translatedStations;
}

void Muon::MuonStationBuilderImpl::glueComponents(Trk::DetachedTrackingVolume* stat) const {
    Trk::TrackingVolumeArray* volArray = stat->trackingVolume()->confinedVolumes();
    if (!volArray || volArray->arrayObjectsNumber() <= 1) {
        return;
    }

    Trk::BinnedArraySpan<Trk::TrackingVolume* const> components =  volArray->arrayObjects();
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

void Muon::MuonStationBuilderImpl::identifyLayers(
    Trk::DetachedTrackingVolume* station, Identifier /*stationID*/, int eta,
    int phi, const MuonGM::MuonDetectorManager* muonMgr) const {
    ATH_MSG_VERBOSE(name() << " identifying layers ");

    const std::string stationName = station->trackingVolume()->volumeName();
    ATH_MSG_VERBOSE(" in station " << station->name());
    const std::string stationStr = stationName.substr(0, 3);
    bool is_valid{false};
    if (m_idHelperSvc->hasCSC() && stationStr[0] == 'C') {
        const int cscEtaSt = eta - MuonGM::MuonDetectorManager::NCscStEtaOffset;
        const Identifier readout_id = muonMgr->cscIdHelper()->channelID(
            stationStr, cscEtaSt, phi + 1, 1, 1, 0, 1, is_valid);
        const MuonGM::CscReadoutElement* cscRE =
            is_valid ? muonMgr->getCscReadoutElement(readout_id) : nullptr;
        if (!cscRE) {
            const Identifier backup_id = muonMgr->cscIdHelper()->channelID(
                stationStr, cscEtaSt, phi + 1, 2, 1, 0, 1, is_valid);
            cscRE =
                is_valid ? muonMgr->getCscReadoutElement(backup_id) : nullptr;
        }
        if (cscRE) {
            for (int gasgap = 0; gasgap < cscRE->Ngasgaps(); gasgap++) {
                Identifier idi = m_idHelperSvc->cscIdHelper().channelID(
                    cscRE->identify(), cscRE->ChamberLayer(), gasgap + 1, 0, 1,
                    is_valid);
                if (!is_valid)
                    continue;
                const Trk::PlaneSurface* stripSurf =
                    dynamic_cast<const Trk::PlaneSurface*>(
                        &(cscRE->surface(idi)));
                const Amg::Vector3D& gpi = stripSurf->center();
                Trk::TrackingVolume* assocVol =
                    station->trackingVolume()->associatedSubVolume(gpi);
                Trk::Layer* assocLay = nullptr;
                if (assocVol)
                    assocLay = assocVol->associatedLayer(gpi);
                unsigned int iD = idi.get_identifier32().get_compact();
                if (assocVol && assocLay) {
                    assocLay->setLayerType(iD);
                }
                if (assocLay) {
                    assocLay->setRef((assocLay->surfaceRepresentation()
                                          .transform()
                                          .inverse() *
                                      gpi)[1]);
                }
            }
        } else {
            ATH_MSG_DEBUG("cscRE not found:" << stationName << "," << eta << ","
                                             << phi);
        }
    } else if (stationStr[0] == 'T') {
        int st = 7;
        if (stationStr == "T1F") {
            st = 0;
        } else if (stationStr == "T1E") {
            st = 1;
        } else if (stationStr == "T2F") {
            st = 2;
        } else if (stationStr == "T2E") {
            st = 3;
        } else if (stationStr == "T3F") {
            st = 4;
        } else if (stationStr == "T3E") {
            st = 5;
        } else if (stationStr == "T4F") {
            st = 6;
        }

        const int stationName =
            st - MuonGM::MuonDetectorManager::NTgcStatTypeOff;
        const int zi = eta - MuonGM::MuonDetectorManager::NTgcStEtaOffset;
        const int stationEta =
            zi + (eta >= MuonGM::MuonDetectorManager::NTgcStEtaOffset);
        auto getReadout = [stationName, stationEta, muonMgr](int phi) {
            const int stationPhi = phi + 1;
            bool is_valid{false};
            const Identifier id = muonMgr->tgcIdHelper()->elementID(
                stationName, stationEta, stationPhi, is_valid);
            return is_valid ? muonMgr->getTgcReadoutElement(id) : nullptr;
        };
        const MuonGM::TgcReadoutElement* tgc = getReadout(phi - 1);

        if (!tgc || !(station->trackingVolume()->inside(tgc->center(), 0.))) {
            unsigned int phit = 0;

            while (phit < 48) {
                const MuonGM::TgcReadoutElement* tgct = getReadout(phit);
                if (tgct &&
                    station->trackingVolume()->inside(tgct->center(), 0.)) {
                    tgc = tgct;
                    phi = phit;
                    // update station identity
                    Identifier oldId(
                        station->layerRepresentation()->layerType());
                    int stationName =
                        m_idHelperSvc->tgcIdHelper().stationName(oldId);
                    int stationEta =
                        m_idHelperSvc->tgcIdHelper().stationEta(oldId);
                    Identifier stId = m_idHelperSvc->tgcIdHelper().elementID(
                        stationName, stationEta, phi, is_valid);
                    station->layerRepresentation()->setLayerType(
                        stId.get_identifier32().get_compact());
                    break;
                }
                phit++;
            }
        }

        if (tgc) {

            int etaSt = tgc->getStationEta();
            int phiSt = tgc->getStationPhi();

            bool validId{false};
            Identifier wireId = m_idHelperSvc->tgcIdHelper().channelID(
                stationStr, etaSt, phiSt, 1, 0, 1, validId);
            if (!validId)
                ATH_MSG_ERROR("invalid TGC channel:" << wireId);
            const Amg::Vector3D gp = tgc->channelPos(wireId);
            Trk::TrackingVolume* assocVol =
                station->trackingVolume()->associatedSubVolume(gp);
            if (!assocVol)
                ATH_MSG_DEBUG("wrong tgcROE?" << stationStr << "," << etaSt
                                              << "," << phiSt);
            if (assocVol && assocVol->confinedLayers()) {
                Trk::BinnedArraySpan<Trk::Layer* const> layers =
                    assocVol->confinedLayers()->arrayObjects();

                for (unsigned int il = 0; il < layers.size(); il++) {
                    wireId = m_idHelperSvc->tgcIdHelper().channelID(
                        stationStr, etaSt, phiSt, il + 1, 0, 1, validId);
                    if (!validId) {
                        if (!validId)
                            ATH_MSG_ERROR("invalid TGC channel:" << wireId);
                        layers[il]->setLayerType(1);
                    } else {
                        unsigned int id =
                            wireId.get_identifier32().get_compact();
                        layers[il]->setLayerType(id);
                        // validation
                        Identifier checkId(layers[il]->layerType());
                        const Trk::PlaneSurface* stripSurf =
                            dynamic_cast<const Trk::PlaneSurface*>(
                                &(tgc->surface(checkId)));
                        if ((layers[il]
                                 ->surfaceRepresentation()
                                 .transform()
                                 .inverse() *
                             stripSurf->center())
                                .mag() > 0.001)
                            ATH_MSG_DEBUG("TGC strip plane shifted:"
                                          << st << "," << eta << "," << phi
                                          << ":"
                                          << layers[il]
                                                     ->surfaceRepresentation()
                                                     .transform()
                                                     .inverse() *
                                                 stripSurf->center());
                    }
                }
            }
        } else {
            ATH_MSG_WARNING(name() << "tgcROE not found for :" << stationName
                                   << "," << eta << "," << phi);
        }
    } else if (m_idHelperSvc->hasMDT() &&
               (stationName[0] == 'B' || stationName[0] == 'E')) {
        // recalculate id
        Identifier stId(station->layerRepresentation()->layerType());

        const int nameIndex = m_idHelperSvc->mdtIdHelper().stationNameIndex(
            stationName.substr(0, 3));
        if (station->trackingVolume()->confinedVolumes()) {
            Trk::BinnedArraySpan<Trk::TrackingVolume* const> cVols =
                station->trackingVolume()->confinedVolumes()->arrayObjects();
            for (auto* cVol : cVols) {
                if (cVol->confinedLayers()) {
                    Trk::BinnedArraySpan<Trk::Layer* const> cLays =
                        cVol->confinedLayers()->arrayObjects();
                    const MuonGM::MdtReadoutElement* mdtROE = nullptr;
                    bool is_valid{false};
                    for (auto* cLay : cLays) {
                        Identifier id(cLay->layerType());
                        if (id.get_compact() > 0 && m_idHelperSvc->isMdt(id)) {
                            Identifier newId =
                                m_idHelperSvc->mdtIdHelper().channelID(
                                    nameIndex, eta, phi,
                                    m_idHelperSvc->mdtIdHelper().multilayer(id),
                                    m_idHelperSvc->mdtIdHelper().tubeLayer(id),
                                    m_idHelperSvc->mdtIdHelper().tube(id),
                                    is_valid);
                            if (!mdtROE)
                                mdtROE =
                                    is_valid
                                        ? muonMgr->getMdtReadoutElement(newId)
                                        : nullptr;
                            unsigned int newid =
                                newId.get_identifier32().get_compact();
                            cLay->setLayerType(newid);
                            // check reference position
                            if (mdtROE) {
                                double ref = cLay->getRef();
                                // double loc = mdtROE->localROPos(newId)[2]; //
                                // this does not take into account ROE shift wrt
                                // TG station/layer
                                double loc = (cLay->surfaceRepresentation()
                                                  .transform()
                                                  .inverse() *
                                              mdtROE->tubePos(newId))[1];
                                if (std::abs(ref) > 10e6) {
                                    double sign = (ref > 0.) ? 1. : -1.;
                                    int dec = int(ref / 1e5);
                                    ref = ref - dec * 1e5 -
                                          0.5 * (sign + 1) * 1e5;
                                    if (std::abs(ref - loc) >
                                        0.001) {  // re-pack
                                        cLay->setRef(loc + dec * 1e5 +
                                                     0.5 * (sign + 1) * 1e5);
                                    }
                                } else if (std::abs(ref - loc) > 0.001) {
                                    cLay->setRef(loc);
                                }
                            }
                        }
                    }
                }
                if (!cVol->confinedArbitraryLayers().empty()) {
                    Trk::ArraySpan<Trk::Layer* const> cLays =
                        cVol->confinedArbitraryLayers();
                    for (auto* cLay : cLays) {
                        Identifier id(cLay->layerType());
                        bool is_valid{false};
                        if (m_idHelperSvc->hasRPC() && id.get_compact() > 0 &&
                            m_idHelperSvc->isRpc(id)) {
                            Identifier newId =
                                m_idHelperSvc->rpcIdHelper().channelID(
                                    nameIndex, eta, phi,
                                    m_idHelperSvc->rpcIdHelper().doubletR(id),
                                    m_idHelperSvc->rpcIdHelper().doubletZ(id),
                                    m_idHelperSvc->rpcIdHelper().doubletPhi(id),
                                    m_idHelperSvc->rpcIdHelper().gasGap(id),
                                    m_idHelperSvc->rpcIdHelper().measuresPhi(
                                        id),
                                    m_idHelperSvc->rpcIdHelper().strip(id),
                                    is_valid);
                            int newid = newId.get_identifier32().get_compact();
                            cLay->setLayerType(newid);
                        }
                    }
                }
            }
        }
    }
    // by now, all the layers should be identified - verify
    if (station->trackingVolume()->confinedVolumes()) {
        Trk::BinnedArraySpan<Trk::TrackingVolume* const> cVols =
            station->trackingVolume()->confinedVolumes()->arrayObjects();
        for (auto* cVol : cVols) {
            if (cVol->confinedLayers()) {
                Trk::BinnedArraySpan<Trk::Layer* const> cLays =
                    cVol->confinedLayers()->arrayObjects();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1)
                        ATH_MSG_DEBUG(station->name()
                                      << "," << cVol->volumeName()
                                      << ", unidentified active layer:" << il);
                    else if (id != 0)
                        checkLayerId("check layer in " + station->name() +
                                         " subvolume " + cVol->volumeName(),
                                     muonMgr, id, cLays[il]);
                }
            }
            if (!cVol->confinedArbitraryLayers().empty()) {
                Trk::ArraySpan<Trk::Layer* const> cLays =
                    cVol->confinedArbitraryLayers();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1)
                        ATH_MSG_DEBUG(station->name()
                                      << "," << cVol->volumeName()
                                      << ", unidentified active layer:" << il);
                    else if (id != 0)
                        checkLayerId("check arbitrary layer in " +
                                         station->name() + " subvolume " +
                                         cVol->volumeName(),
                                     muonMgr, id, cLays[il]);
                }
            }
        }
    }
    if (station->trackingVolume()->confinedLayers()) {
        Trk::BinnedArraySpan<Trk::Layer* const> cLays =
            station->trackingVolume()->confinedLayers()->arrayObjects();
        for (unsigned int il = 0; il < cLays.size(); il++) {
            Identifier id(cLays[il]->layerType());
            if (id == 1)
                ATH_MSG_DEBUG(station->name()
                              << "," << station->name()
                              << ", unidentified active layer:" << il);
            else if (id != 0)
                checkLayerId("check confined layer in " + station->name(),
                             muonMgr, id, cLays[il]);
        }
    }
    // end identification check
}

void Muon::MuonStationBuilderImpl::identifyNSWLayers(Trk::DetachedTrackingVolume& station, 
                                                     const Identifier& id) const {


    if (!station.trackingVolume()->confinedLayers()) return;
    Trk::BinnedArraySpan<Trk::Layer* const> lays = station.trackingVolume()->confinedLayers()->arrayObjects();
    const bool isStgc{station.name().substr(0, 4) == "sTGC"};
    const bool isMm{station.name().substr(0, 2) == "MM"};
    if (!isMm && !isStgc) return;
    for (unsigned int il = 0; il < lays.size(); il++) {
        if (isStgc) {
            const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
            const Identifier lid = idHelper.channelID(id, idHelper.multilayer(id), il + 1, 2, 1);
            lays[il]->setLayerType(lid.get_identifier32().get_compact());

        } else if (isMm) {
            const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
            const Identifier lid = idHelper.channelID(id, idHelper.multilayer(id), il + 1, 1);
            lays[il]->setLayerType(lid.get_identifier32().get_compact());
        }
    }
}

void Muon::MuonStationBuilderImpl::identifyPrototype(Trk::TrackingVolume& station, int eta, int phi,
                                                     const Amg::Transform3D& transf,
                                                     const MuonGM::MuonDetectorManager* muonMgr) const {
    ATH_MSG_VERBOSE(name() << " identifying prototype ");

    const std::string& stationName = station.volumeName();
    ATH_MSG_VERBOSE(" for station " << stationName);
    const std::string stationNameShort = stationName.substr(0, 3);
    const char stationFirstChar = stationName[0];
    bool is_valid{false};

    if (m_idHelperSvc->hasMDT() && (stationFirstChar == 'B' || stationFirstChar == 'E')) {
        // MDT
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        const int nameIndex = idHelper.stationNameIndex(stationNameShort);
        for (int multi = 1; multi <= 2; ++multi) {
            const Identifier ele_id = idHelper.channelID(nameIndex, eta, phi, multi, 1, 1, is_valid);
            if (!is_valid) {
                continue;
            }
            const MuonGM::MdtReadoutElement* multilayer = muonMgr->getMdtReadoutElement(ele_id);
            if (!multilayer) {
                continue;
            }
            Trk::TrackingVolume* assocVol = station.associatedSubVolume(transf.inverse() * multilayer->center());
            if (!assocVol) {
                ATH_MSG_WARNING("valid multilayer outside station:" << stationName);
                continue;
            }
            int nLayers = multilayer->getNLayers();
            for (int layer = 1; layer <= nLayers; ++layer) {
                Identifier id = idHelper.channelID(nameIndex, eta, phi, multi, layer, 1, is_valid);
                if (!is_valid) {
                    continue;
                }
                // retrieve associated layer
                Trk::Layer* assocLay = assocVol->associatedLayer(transf.inverse() * multilayer->tubePos(id));
                if (assocLay) {
                    assocLay->setLayerType(id.get_identifier32().get_compact());
                }
            }
        }

        // RPC ?
        Trk::BinnedArray<Trk::TrackingVolume>* confinedVolumes = station.confinedVolumes();
        if (confinedVolumes) {
            Trk::BinnedArraySpan<Trk::TrackingVolume* const> vols = confinedVolumes->arrayObjects();
            for (auto* vol : vols) {
                if (!m_idHelperSvc->hasRPC() || vol->volumeName() != "RPC") {
                    break;
                }
                
                // for active layers do a search of associated ROE
                Trk::ArraySpan<Trk::Layer* const> layers = vol->confinedArbitraryLayers();
                int nameIndex = m_idHelperSvc->rpcIdHelper().stationNameIndex(stationNameShort);
                ///
                if (stationNameShort == "BME" || stationNameShort == "BMG") {
                    continue;
                }                
                // the following checks are not necessarily needed, since
                // calling channelID with check=true would catch them
                // However, since these validity checks are slow, let's
                // manually skip the obvious non-existant ones
                //
                // only BOG7/8 and BMS2/4 have doubletZ=3, the remaing BOG
                // and BOF4 have doubletZ=1
                int doubletZMax = 2;
                if (stationNameShort.find("BMS") != std::string::npos && (std::abs(eta) == 2 || std::abs(eta) == 4)) {
                    doubletZMax = 3;
                }
                else if (stationNameShort.find("BOG") != std::string::npos) {
                    if (std::abs(eta) == 7 || std::abs(eta) == 8) {
                        doubletZMax = 3;
                    } else {
                        doubletZMax = 1;
                    }
                } else if (stationNameShort.find("BOF") != std::string::npos && std::abs(eta) == 4){
                    doubletZMax = 1;
                }
                // all BOL/BOS and BOF1 have doubletR=1
                const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                for (int doubletR = 1; doubletR <= 2; ++doubletR) {
                    bool isValid{false};
                    const Identifier stationId = idHelper.elementID(nameIndex, eta, phi, doubletR, isValid);
                    if (!isValid) {
                        continue;
                    }
                    for (int doubletZ = 1; doubletZ <= doubletZMax; ++doubletZ) {
                        for (int doubletPhi = 1; doubletPhi <= 2; doubletPhi++) {
                        
                            const Identifier id = idHelper.channelID(stationId, doubletZ, doubletPhi, 
                                                                        1, 1, 1, isValid);                               
                            if (!isValid) {                                    
                                continue;
                            }
                            const MuonGM::RpcReadoutElement* rpc = muonMgr->getRpcReadoutElement(id);
                            if (!rpc) {
                                continue;
                            }                                    
                            for (int gasGap = 1; gasGap <= rpc->numberOfLayers(); ++gasGap) {
                                        
                                Identifier etaId = idHelper.channelID(id, doubletZ, doubletPhi,
                                                                        gasGap, 0, 1, isValid);
                                
                                const Amg::Vector3D stripLocPos = transf.inverse() * rpc->stripPos(etaId);
                                for (auto* layer : layers) {
                                    if (layer->layerType() == 0 ||
                                        !layer->surfaceRepresentation().isOnSurface(stripLocPos, false, 0.5* layer->thickness())){
                                        continue;
                                    }
                                    const Amg::Vector3D locPos1 =layer->surfaceRepresentation().transform().inverse() * stripLocPos;                                                        
                                    const Amg::Vector3D locPos2 = rpc->surface(etaId).transform().inverse() * rpc->stripPos(etaId);
                                    
                                    
                                    double swap = (std::abs(locPos1[1] - locPos2[0]) > 0.001) ? 20000. : 0.;

                                    layer->setLayerType(etaId.get_identifier32() .get_compact());
                                    
                                    
                                    const Amg::Vector3D locPos = layer->surfaceRepresentation().transform().inverse() *
                                                                    transf.inverse() * rpc->surface(etaId).center();
                                    layer->setRef(swap + locPos[0]);
                                        
                                }
                            }
                        }
                    }
                }                
            }
        }
    }

    // by now, all the layers should be identified - verify
    if (station.confinedVolumes()) {
        Trk::BinnedArraySpan<Trk::TrackingVolume* const> cVols = station.confinedVolumes()->arrayObjects();
        for (auto* cVol : cVols) {
            if (cVol->confinedLayers()) {
                Trk::BinnedArraySpan<Trk::Layer* const> cLays = cVol->confinedLayers()->arrayObjects();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1) {
                        ATH_MSG_DEBUG(station.volumeName()<< "," << cVol->volumeName()  << ", unidentified active layer:" << il);
                    }
                }
            }
            if (!cVol->confinedArbitraryLayers().empty()) {
                Trk::ArraySpan<Trk::Layer* const> cLays = cVol->confinedArbitraryLayers();
                for (unsigned int il = 0; il < cLays.size(); il++) {
                    Identifier id(cLays[il]->layerType());
                    if (id == 1) {
                        ATH_MSG_DEBUG(station.volumeName() << "," << cVol->volumeName() << ", unidentified active layer:" << il);
                    }
                }
            }
        }
    }
    if (station.confinedLayers()) {
        Trk::BinnedArraySpan<Trk::Layer* const> cLays = station.confinedLayers()->arrayObjects();
        for (unsigned int il = 0; il < cLays.size(); il++) {
            Identifier id(cLays[il]->layerType());
            if (id == 1) {
                ATH_MSG_DEBUG(station.volumeName() << ", unidentified active layer:" << il);
            }
        }
    }
    // end identification check
}

void Muon::MuonStationBuilderImpl::getNSWStationsForTranslation(const GeoVPhysVol* pv, const std::string& name,
                                                               const Amg::Transform3D& transform,
    std::vector<std::pair<std::pair<const GeoLogVol*, Trk::MaterialProperties*>,
                          std::vector<Amg::Transform3D>>>& vols,
    std::vector<std::string>& volNames) const {
    // special code to get the Sensitive volume of the sTGC and MM (so the gas
    // volume) throught the Frame

    // subcomponents
    unsigned int nc = pv->getNChildVols();
    ATH_MSG_DEBUG("getNSWStationsForTranslation from:"
                  << pv->getLogVol()->getName() << ","
                  << pv->getLogVol()->getMaterial()->getName()
                  << ", looping over " << nc << " children");

    for (unsigned int ic = 0; ic < nc; ic++) {
        Amg::Transform3D transf = pv->getXToChildVol(ic);
        const GeoVPhysVol* cv = pv->getChildVol(ic);
        const GeoLogVol* clv = cv->getLogVol();
        std::string childName = clv->getName();
        ATH_MSG_DEBUG("getNSWStationsForTranslation child " << childName);

        if (childName.empty()) {
            childName = "Spacer";
        }
        if (childName.size() > 9 && childName.substr(childName.size() - 9, 9) == "Sensitive") {
            childName += std::to_string(ic);
        }

        std::string cName = childName.compare(0, 3, "NSW") == 0 || childName.compare(0, 8, "NewSmall") == 0 
                          ? name : name + childName;
        ATH_MSG_VERBOSE("child number,name,position:" << ic << ":" << clv->getName() << ":"<< Amg::toString(transform * transf));

        if (!cv->getNChildVols()) {
            bool found = false;
            for (unsigned int is = 0; is < vols.size(); is++) {
                if (cName == volNames[is]) {
                    if (std::abs((transform * transf).translation().perp() -
                                 vols[is].second.front().translation().perp()) <
                        1.) {
                        found = true;
                        // order transforms to position prototype at phi=0/
                        // 0.125 pi
                        double phiTr = (transform * transf).translation().phi();
                        if (phiTr > -0.001 && phiTr < 0.4) {
                            vols[is].second.insert(vols[is].second.begin(),
                                                   transform * transf);
                        } else
                            vols[is].second.push_back(transform * transf);
                        ATH_MSG_VERBOSE(
                            "clone?"
                            << clv->getName() << ","
                            << (transform * transf).translation().perp() << ","
                            << (transform * transf).translation().z() << ","
                            << phiTr);

                        break;
                    }
                }
            }
            if (!found) {
                std::vector<Amg::Transform3D> volTr;
                volTr.push_back(transform * transf);
                // divide mother material ? seems strange - check !
                // double scale =  1.;     // 1./nc;
                double thick = 2 * m_muonStationTypeBuilder->get_x_size(pv);
                if (pv->getLogVol()->getMaterial()->getName() != "Ether" &&
                    (childName == "MM_Frame" || childName == "sTGC_Frame")) {
                    Trk::MaterialProperties matComb(0., 10.e10, 10.e10, 13.,
                                                    26., 0.);
                    Trk::Material newMat = m_materialConverter.convert(
                        pv->getLogVol()->getMaterial());
                    matComb.addMaterial(newMat, thick / newMat.x0());
                    ATH_MSG_VERBOSE(" use mother volume thickness, x0: "
                                    << matComb.thickness() << ", "
                                    << matComb.x0()
                                    << " mat: " << matComb.thicknessInX0());
                    Trk::MaterialProperties* nMat =
                        new Trk::MaterialProperties(matComb);
                    // store mother volume (and not child = Frame)
                    std::pair<const GeoLogVol*, Trk::MaterialProperties*> cpair(
                        pv->getLogVol(), nMat);
                    vols.emplace_back(cpair, volTr);
                    // store extensive name
                    volNames.push_back(cName);
                    ATH_MSG_VERBOSE("new NSW station volume added:"
                                    << cName << ", "
                                    << clv->getMaterial()->getName() << ", "
                                    << volTr.back().translation().z() << ", "
                                    << volTr.back().translation().phi()
                                    << " mat: " << matComb.thicknessInX0());
                }
                // printInfo(cv);
            }
        } else {
            getNSWStationsForTranslation(cv, cName, transform * transf, vols,
                                         volNames);
        }
    }
}

std::vector<std::pair<const GeoVPhysVol*, std::vector<Muon::GMInfo>>>
Muon::MuonStationBuilderImpl::retrieveGMsensitive(const MuonGM::MuonDetectorManager* muonMgr) const {

    // a single loop over GM tree to retrieve all necessary information

    std::vector<std::pair<const GeoVPhysVol*, 
                          std::vector<Muon::GMInfo>>> sensitive;

    const GeoVPhysVol* top = muonMgr->getTreeTop(0);
    

    // NSW stations first to avoid double-counting
    const GeoVPhysVol* sTGC_top = m_gmBrowser.findTopBranch(top, "sTGC_1");
    const GeoVPhysVol* MM_top = m_gmBrowser.findTopBranch(top, "MM_1");
    if (sTGC_top && sTGC_top != top) {
        ATH_MSG_DEBUG("sTGC GeoModel branch found:" << sTGC_top->getLogVol()->getName());
        GeoVolumeCursor vol(sTGC_top);
        while (!vol.atEnd()) {
            const GeoVPhysVol* cv = vol.getVolume();
            const std::string& vname = cv->getLogVol()->getName();
            if (vname.find("sTGC_1") == std::string::npos) {
                vol.next();
                continue;
            }
            std::vector<std::pair<const GeoVPhysVol*,
                                  std::vector<Muon::GMInfo>>>::iterator it =
                sensitive.begin();
            while (it < sensitive.end()) {
                if (vname == (*it).first->getLogVol()->getName() &&
                    m_gmBrowser.compareGeoVolumes(cv, (*it).first, 1.e-3) == 0)
                    break;
                it++;
            }

            if (it == sensitive.end()) {
                std::vector<std::pair<Amg::Transform3D, int>> cloneList;
                cloneList.push_back(
                    std::make_pair(vol.getTransform(), 0));
                sensitive.push_back(std::make_pair(cv, cloneList));
            } else {
                Amg::Transform3D transf = vol.getTransform();
                // order transforms to position prototype at phi=0/ 0.125 pi
                double phiTr = transf.translation().phi();
                if (phiTr > -0.001 && phiTr < 0.4)
                    (*it).second.insert((*it).second.begin(),
                                        std::make_pair(vol.getTransform(), 0));
                else
                    (*it).second.push_back(std::make_pair(vol.getTransform(), 0));
            }
            vol.next();
        }
    }

    if (MM_top && MM_top != top) {
        ATH_MSG_DEBUG("MM GeoModel branch found:" << MM_top->getLogVol()->getName());
        GeoVolumeCursor vol(MM_top);
        while (!vol.atEnd()) {
            const GeoVPhysVol* cv = vol.getVolume();
            const std::string& vname = cv->getLogVol()->getName();
            if (vname.find("MM_1") == std::string::npos) {
                vol.next();
                continue;
            }
            std::vector<std::pair<const GeoVPhysVol*,
                                  std::vector<Muon::GMInfo>>>::iterator it =
                sensitive.begin();
            while (it < sensitive.end()) {
                if (vname == (*it).first->getLogVol()->getName() &&
                    m_gmBrowser.compareGeoVolumes(cv, (*it).first, 1.e-3) == 0)
                    break;
                it++;
            }

            if (it == sensitive.end()) {
                std::vector<std::pair<Amg::Transform3D, int>> cloneList;
                cloneList.push_back(std::make_pair(vol.getTransform(), 0));
                sensitive.push_back(std::make_pair(cv, cloneList));
            } else {
                Amg::Transform3D transf = vol.getTransform();
                // order transforms to position prototype at phi=0/ 0.125 pi
                double phiTr = transf.translation().phi();
                if (phiTr > -0.001 && phiTr < 0.4)
                    (*it).second.insert((*it).second.begin(), std::make_pair(vol.getTransform(), 0));
                else
                    (*it).second.push_back(std::make_pair(vol.getTransform(), 0));
            }
            vol.next();
        }
    }

    GeoVolumeCursor vol(top);
    while (!vol.atEnd()) {
        const GeoVPhysVol* cv = &(*(vol.getVolume()));
        const std::string& vname = cv->getLogVol()->getName();
        if (vname.find("Station") == std::string::npos &&
            vname.find("MM_1") == std::string::npos &&
            vname.find("sTGC_1") == std::string::npos) {
            vol.next();
            continue;
        }

        // TGC stations retrieved at level-1
        if (vname.substr(0, 1) == "T") {

            for (const auto&[tv, tvTrf] : geoGetVolumes(cv)) {
                const GeoLogVol* tlv = tv->getLogVol();
                const Amg::Transform3D transform = vol.getTransform() * tvTrf;
                const std::string& tgc_name = tlv->getName();

                std::vector<std::pair<const GeoVPhysVol*,
                                      std::vector<Muon::GMInfo>>>::iterator it =
                    sensitive.begin();
                while (it < sensitive.end()) {
                    if (tgc_name == (*it).first->getLogVol()->getName() &&
                        m_gmBrowser.compareGeoVolumes(tv, (*it).first, 1.e-3) ==
                            0)
                        break;
                    it++;
                }

                if (it == sensitive.end()) {
                    std::vector<std::pair<Amg::Transform3D, int>> cloneList;
                    cloneList.push_back(std::make_pair(transform, vol.getId()));
                    sensitive.push_back(std::make_pair(tv, cloneList));
                } else {
                    Amg::Transform3D transf = transform;
                    // order transforms to position prototype at phi=0/ 0.125 pi
                    double phiTr = transf.translation().phi();
                    if (phiTr > -0.001 && phiTr < 0.4)
                        (*it).second.insert((*it).second.begin(),
                                            std::make_pair(transform, vol.getId()));
                    else
                        (*it).second.push_back(std::make_pair(transform, vol.getId()));
                }

            }  // end loop over TGC

        } else {

            std::vector<std::pair<const GeoVPhysVol*,
                                  std::vector<Muon::GMInfo>>>::iterator it =
                sensitive.begin();
            while (it < sensitive.end()) {
                if (vname == (*it).first->getLogVol()->getName() &&
                    m_gmBrowser.compareGeoVolumes(cv, (*it).first, 1.e-3) == 0)
                    break;
                it++;
            }

            if (it == sensitive.end()) {
                std::vector<std::pair<Amg::Transform3D, int>> cloneList;
                cloneList.push_back(std::make_pair(vol.getTransform(), vol.getId()));
                sensitive.push_back(std::make_pair(cv, cloneList));
            } else {
                Amg::Transform3D transf = vol.getTransform();
                // order transforms to position prototype at phi=0/ 0.125 pi
                double phiTr = transf.translation().phi();
                if (phiTr > -0.001 && phiTr < 0.4)
                    (*it).second.insert((*it).second.begin(),
                                        std::make_pair(vol.getTransform(), vol.getId()));
                else
                    (*it).second.push_back(std::make_pair(vol.getTransform(), vol.getId()));
            }
        }  // end non-TGC
        vol.next();
    }

    ATH_MSG_DEBUG("Number of muon station types in GeoModel tree:" << sensitive.size());

    return sensitive;
}

std::unique_ptr<Trk::DetachedTrackingVolume>
Muon::MuonStationBuilderImpl::buildDetachedTrackingVolumeType(const MuonGM::MuonDetectorManager* muonMgr, 
                                                              const GeoVPhysVol* cv,
                                                              Muon::GMInfo gmInfo) const {
    const GeoLogVol* clv = cv->getLogVol();
    const std::string& vname = clv->getName();
    ATH_MSG_DEBUG(name() << " building station prototype for " << cv->getLogVol()->getName());
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    MuonStationTypeBuilder::Cache cache{};

    std::unique_ptr<Trk::DetachedTrackingVolume> typeStat{};

    if (vname.substr(0, 4) == "sTGC" || vname.substr(0, 2) == "MM") {
        std::string sName = vname.substr(vname.find('-') + 1);
        Identifier nswId =  m_muonStationTypeBuilder->identifyNSW(sName, gmInfo.first);

        if (vname.substr(0, 4) == "sTGC" && m_idHelperSvc->issTgc(nswId)) {
            return m_muonStationTypeBuilder->process_sTGC(nswId, cv, gmInfo.first);
        } else if (vname.substr(0, 2) == "MM" && m_idHelperSvc->isMM(nswId)) {
            return m_muonStationTypeBuilder->process_MM(nswId, cv, gmInfo.first);
        }
    }

    if (!m_buildBarrel && vname.compare(0, 1, "B") == 0)
        return typeStat;
    if (!m_buildEndcap && vname.compare(0, 1, "E") == 0)
        return typeStat;
    if (!m_buildCsc && vname.compare(0, 1, "C") == 0)
        return typeStat;
    if (!m_buildTgc && vname.compare(0, 1, "T") == 0)
        return typeStat;

    int etaphi = gmInfo.second;  // retrieve eta/phi indexes
    int sign = (etaphi < 0) ? -1 : 1;
    etaphi = sign * etaphi;
    int is_mirr = etaphi / 1000;
    etaphi = etaphi - is_mirr * 1000;
    int eta = etaphi / 100;
    int phi = etaphi - eta * 100;
    eta = eta * sign;
    const MuonGM::MuonStation* gmStation = muonMgr->getMuonStation(vname.substr(0, 3), eta, phi);
    if (!gmStation) {
        gmStation = muonMgr->getMuonStation(vname.substr(0, 4), eta, phi);
    }
    // assembly ?
    if (!gmStation) {
        int etaphi = gmInfo.second;  // retrieve eta/phi indexes
        int a_etaphi = static_cast<int>(etaphi / 100000);
        int sideC = static_cast<int>(a_etaphi / 10000);
        a_etaphi -= sideC * 10000;
        is_mirr = static_cast<int>(a_etaphi / 1000);
        a_etaphi -= is_mirr * 1000;
        eta = static_cast<int>(a_etaphi / 100);
        phi = a_etaphi - eta * 100;
        if (sideC)
            eta *= -1;
        gmStation = muonMgr->getMuonStation(vname.substr(0, 3), eta, phi);
    }
    //
    std::string stname = (vname.compare(0, 1, "T") == 0) ? vname : (clv->getName()).substr(0, vname.size() - 8);
    //
    if (stname.compare(0, 1, "B") == 0 && eta < 0) {
        stname = (clv->getName()).substr(0, vname.size() - 8) + "-";
    }
    ATH_MSG_VERBOSE(" new station type " << stname << "," << clv->getShape()->type());
    ATH_MSG_VERBOSE(" prototype built from eta, phi:" << eta << "," << phi);

    if (stname.compare(0, 2, "CS") == 0 || stname.compare(0, 1, "T") == 0) {

        if (stname.compare(0, 2, "CS") == 0) {
            auto csc_station = m_muonStationTypeBuilder->processCscStation(cv, stname, cache);
            // create layer representation
            auto layerRepr = m_muonStationTypeBuilder->createLayerRepresentation(*csc_station);
            // create prototype as detached tracking volume
            auto layerVec = std ::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
            typeStat = std::make_unique<Trk::DetachedTrackingVolume>(stname, csc_station.release(), 
                                                                     layerRepr.first.release(), layerVec.release());
        } else {
            std::unique_ptr<Trk::TrackingVolume> tgc_station{m_muonStationTypeBuilder->processTgcStation(cv, cache)};
            // create layer representation
            auto layerRepr =  m_muonStationTypeBuilder->createLayerRepresentation(*tgc_station);
            // create prototype as detached tracking volume
            auto layerVec = std ::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
            typeStat =  std::make_unique<Trk::DetachedTrackingVolume>(stname, tgc_station.release(), 
                                                                      layerRepr.first.release(), layerVec.release());
        }
        
    } else {
        const GeoShape* shapeS = clv->getShape();
        while (shapeS->type() != "Trd") {
            if (shapeS->type() == "Shift") {
                const GeoShapeShift* shift = dynamic_cast<const GeoShapeShift*>(shapeS);
                shapeS = shift->getOp();
            } else if (shapeS->type() == "Subtraction") {
                const GeoShapeSubtraction* sub = dynamic_cast<const GeoShapeSubtraction*>(shapeS);
                shapeS = sub->getOpA();
            } else if (shapeS->type() == "Union") {
                const GeoShapeUnion* uni = dynamic_cast<const GeoShapeUnion*>(shapeS);
                shapeS = uni->getOpA();
            } else {
                ATH_MSG_WARNING("unexpected station shape ? "<< shapeS->type() << ", station not built");
                break;
            }
        }
        const GeoTrd* trd = dynamic_cast<const GeoTrd*>(shapeS);

        double halfX1{0.}, halfX2{0.}, halfY1{0.}, halfY2{0.}, halfZ{0.};
        if (trd) {
            //
            halfX1 = trd->getXHalfLength1();
            halfX2 = trd->getXHalfLength2();
            halfY1 = trd->getYHalfLength1();
            halfY2 = trd->getYHalfLength2();
            halfZ = trd->getZHalfLength();

            // define enveloping volume
            std::unique_ptr<Trk::TrackingVolumeArray> confinedVolumes{};
            std::vector<std::unique_ptr<Trk::Layer>> confinedLayers{};
            std::unique_ptr<Trk::Volume> envelope;
            std::string shape = "Trd";
            if (halfX1 == halfX2 && halfY1 == halfY2)
                shape = "Box";
            if (shape == "Box") {
                auto envBounds = std::make_unique<Trk::CuboidVolumeBounds>(halfX1, halfY1, halfZ);
                // station components
                confinedVolumes = m_muonStationTypeBuilder->processBoxStationComponents(cv, *envBounds, cache);
                if (!confinedVolumes) {
                    confinedLayers = m_muonStationTypeBuilder->processBoxComponentsArbitrary(cv, *envBounds, cache);
                }
                // enveloping volume
                envelope = std::make_unique<Trk::Volume>(nullptr, envBounds.release());
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
                    ATH_MSG_WARNING("station envelope arbitrary trapezoid?"<< stname);
                }
                if (envBounds) {
                    // station components                   
                    confinedVolumes = m_muonStationTypeBuilder->processTrdStationComponents(cv, *envBounds, cache);
                    // enveloping volume
                    envelope = std::make_unique<Trk::Volume>(makeTransform(transf), envBounds.release());
                }
            }

            if (envelope) {
                // ready to build the station prototype
                std::unique_ptr<Trk::TrackingVolume> newType{};
                if (!confinedLayers.empty()) {
                    auto confinedLayerPtr = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(confinedLayers));
                    newType = std::make_unique<Trk::TrackingVolume>(*envelope, m_muonMaterial, confinedLayerPtr.release(), stname);
                } else {
                    newType = std::make_unique<Trk::TrackingVolume>(*envelope, m_muonMaterial, nullptr, confinedVolumes.release(), stname);  
                }
               

                // identify prototype
                if ((stname.compare(0, 1, "B") == 0 || stname.compare(0, 1, "E") == 0))
                    identifyPrototype(*newType, eta, phi, gmStation->getTransform(), muonMgr);

                // create layer representation
                auto layerRepr = m_muonStationTypeBuilder->createLayerRepresentation(*newType);

                // create prototype as detached tracking volume
                auto layerVec = std::make_unique<std::vector<Trk::Layer*>>(Muon::release(layerRepr.second));
                typeStat = std::make_unique<Trk::DetachedTrackingVolume>(stname, newType.release(), 
                                                                         layerRepr.first.release(), layerVec.release());
            }
        }
    }  // end new station type

    ATH_MSG_DEBUG(" station prototype built for " << vname);

    ///////////////////////////////////////////////////////////////////////////////////////
    return typeStat;
}

Identifier Muon::MuonStationBuilderImpl::resolveId(std::string vname, Muon::GMInfo gm_info, int& eta, int& phi,
                                                   const MuonGM::MuonDetectorManager* muonMgr) const {

    Identifier stId(0);

    int etaphi = gm_info.second;  // retrieve eta/phi indexes
    int sign = (etaphi < 0) ? -1 : 1;
    etaphi = sign * etaphi;
    int is_mirr = etaphi / 1000;
    etaphi = etaphi - is_mirr * 1000;
    eta = etaphi / 100;
    phi = etaphi - eta * 100;
    eta = eta * sign;
    const MuonGM::MuonStation* gmStation =
        muonMgr->getMuonStation(vname.substr(0, 3), eta, phi);
    // try to retrieve
    if (!gmStation) {
        gmStation = muonMgr->getMuonStation(vname.substr(0, 4), eta, phi);
    }
    // assembly ?
    if (!gmStation) {
        int etaphi = gm_info.second;  // retrieve eta/phi indexes
        int a_etaphi = static_cast<int>(etaphi / 100000);
        int sideC = static_cast<int>(a_etaphi / 10000);
        a_etaphi -= sideC * 10000;
        is_mirr = static_cast<int>(a_etaphi / 1000);
        a_etaphi -= is_mirr * 1000;
        eta = static_cast<int>(a_etaphi / 100);
        phi = a_etaphi - eta * 100;
        if (sideC)
            eta *= -1;
        gmStation = muonMgr->getMuonStation(vname.substr(0, 3), eta, phi);
    }
    //
    if (!gmStation)
        ATH_MSG_WARNING("Muon station not found! " << vname << "," << eta << ","
                                                   << phi);

    std::string stName;
    if (vname.compare(0, 1, "T") == 0 || vname.compare(0, 1, "C") == 0) {
        stName = vname.substr(0, 4);
    } else {
        stName = (vname.substr(0, vname.size() - 8));
        if (stName.compare(0, 1, "B") == 0 && eta < 0) {
            stName = vname.substr(0, vname.size() - 8) + "-";
        }
    }
    if (!gmStation)
        return stId;
    Amg::Transform3D transf = gmStation->getTransform();
    bool is_valid{false};
    if (m_idHelperSvc->hasCSC() && stName.compare(0, 1, "C") == 0) {
        stId = m_idHelperSvc->cscIdHelper().elementID(vname.substr(0, 3), eta,
                                                      phi, is_valid);
    }
    // adjust eta,phi
    if (vname.compare(0, 1, "C") == 0) {
        eta = 1;
        if (transf.translation().z() < 0)
            eta = 0;
        double phic = transf.translation().phi() + 0.1;
        phi =
            static_cast<int>(phic < 0 ? 4 * phic / M_PI + 8 : 4 * phic / M_PI);
    }
    if (vname.compare(0, 1, "T") == 0) {
        bool az = true;
        std::string sub = vname.substr(7, 2);
        if (transf.translation().z() < 0)
            az = false;
        if (sub == "01")
            eta = az ? 5 : 4;
        else if (sub == "02")
            eta = az ? 5 : 4;
        else if (sub == "03")
            eta = az ? 6 : 3;
        else if (sub == "04")
            eta = az ? 7 : 2;
        else if (sub == "05")
            eta = az ? 8 : 1;
        else if (sub == "06")
            eta = az ? 5 : 4;
        else if (sub == "07")
            eta = az ? 5 : 4;
        else if (sub == "08")
            eta = az ? 6 : 3;
        else if (sub == "09")
            eta = az ? 7 : 2;
        else if (sub == "10")
            eta = az ? 8 : 1;
        else if (sub == "11")
            eta = az ? 9 : 0;
        else if (sub == "12")
            eta = az ? 5 : 4;
        else if (sub == "13")
            eta = az ? 5 : 4;
        else if (sub == "14")
            eta = az ? 6 : 3;
        else if (sub == "15")
            eta = az ? 7 : 2;
        else if (sub == "16")
            eta = az ? 8 : 1;
        else if (sub == "17")
            eta = az ? 9 : 0;
        else if (sub == "18")
            eta = az ? 5 : 4;
        else if (sub == "19")
            eta = az ? 5 : 4;
        else if (sub == "20")
            eta = az ? 5 : 4;
        else if (sub == "21")
            eta = az ? 5 : 4;
        else if (sub == "22")
            eta = az ? 5 : 4;
    }
    if (m_idHelperSvc->hasTGC() && stName[0] == 'T') {
        int etaSt = eta - 4;
        if (eta < 5)
            etaSt = eta - 5;
        double phic = transf.translation().phi();
        if (vname.compare(2, 1, "E") == 0 && vname.compare(0, 3, "T4E") != 0)
            phi = static_cast<int>(phic < 0 ? 24 * phic / M_PI + 48
                                            : 24 * phic / M_PI);
        else
            phi = static_cast<int>(phic < 0 ? 12 * phic / M_PI + 24
                                            : 12 * phic / M_PI);
        phi++;
        stId = m_idHelperSvc->tgcIdHelper().elementID(vname.substr(0, 3), etaSt,
                                                      phi, is_valid);
    } else if (m_idHelperSvc->hasRPC() && stName.compare(0, 3, "BML") == 0) {
        stId = m_idHelperSvc->rpcIdHelper().elementID(vname.substr(0, 3), eta,
                                                      phi, 1, is_valid);
    } else if (m_idHelperSvc->hasMDT() && stName.compare(0, 1, "C") != 0) {
        stId = m_idHelperSvc->mdtIdHelper().elementID(vname.substr(0, 3), eta,
                                                      phi, is_valid);
    }
    if (!is_valid || !stId.get_compact()) {
        ATH_MSG_DEBUG("identifier of the station not found:"
                      << vname << "," << eta << "," << phi);
        if (!stId.get_compact())
            return Identifier(0);
    }
    return stId;
}

void Muon::MuonStationBuilderImpl::checkLayerId(
    std::string comment, const MuonGM::MuonDetectorManager* muonMgr,
    Identifier id, const Trk::Layer* lay) const {

    // RE
    if (m_idHelperSvc->isMdt(id)) {
        const MuonGM::MdtReadoutElement* mdtRE = muonMgr->getMdtReadoutElement(
            id);  //!< access via extended identifier (requires unpacking)

        if (mdtRE) {

            Amg::Transform3D trid = mdtRE->transform(id);

            if (!lay->surfaceRepresentation().isOnSurface(trid.translation()))
                ATH_MSG_DEBUG(comment << ":tube(id) not on surface:");
        }

    } else if (m_idHelperSvc->isRpc(id)) {

        const MuonGM::RpcReadoutElement* rpcRE = muonMgr->getRpcReadoutElement(
            id);  //!< access via extended identifier (requires unpacking)
        Amg::Transform3D trid = rpcRE->transform(id);

        Amg::Transform3D check_layer_identity = lay->surfaceRepresentation().transform().inverse() * trid;
        if (!Amg::doesNotDeform(check_layer_identity)) {
            ATH_MSG_DEBUG(__FILE__<<":"<<__LINE__<<" "<<comment<<" "<<Amg::toString(check_layer_identity));
        }

    } else if (m_idHelperSvc->isTgc(id)) {

        const MuonGM::TgcReadoutElement* tgcRE = muonMgr->getTgcReadoutElement(
            id);  //!< access via extended identifier (requires unpacking)

        Amg::Transform3D trid = tgcRE->transform(id);

        Amg::Transform3D check_layer_identity = lay->surfaceRepresentation().transform().inverse() * trid;
        if (!Amg::doesNotDeform(check_layer_identity)) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<comment <<" "<<Amg::toString(check_layer_identity));
        }
    }
}
