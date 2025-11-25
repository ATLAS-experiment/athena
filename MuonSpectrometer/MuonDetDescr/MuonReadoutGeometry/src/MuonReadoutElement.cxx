/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 A Muon GeoVDetectorElement
 -----------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/MuonReadoutElement.h"

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/StraightLineSurface.h"

namespace {
    std::string to_string(const Trk::DetectorElemType type ) {
        if (type == Trk::DetectorElemType::Mdt) return "Mdt";
        else if (type == Trk::DetectorElemType::Rpc) return "Rpc";
        else if (type == Trk::DetectorElemType::Tgc) return "Tgc";
        else if (type == Trk::DetectorElemType::sTgc) return "sTgc";
        else if (type == Trk::DetectorElemType::MM) return "Mm";
        else if (type == Trk::DetectorElemType::Csc) return "Csc";
        return "Unknown";
    }
}
namespace MuonGM {

    MuonReadoutElement::MuonReadoutElement(GeoVFullPhysVol* pv, MuonDetectorManager* mgr, Trk::DetectorElemType detType) :
        TrkDetElementBase(pv), 
        AthMessaging{to_string(detType)+"MuonReadoutElement"}, 
        m_type{detType}, 
        m_muon_mgr{mgr} {
            if (!m_idHelperSvc.retrieve().isSuccess()) {
                ATH_MSG_FATAL("Failed to retrieve the MuonIdHelperSvc");
                throw std::runtime_error("Invalid MuonIdHelperSvc");
            }
        }

    MuonReadoutElement::~MuonReadoutElement() = default;

    const Amg::Vector3D MuonReadoutElement::globalPosition() const { return absTransform().translation(); }

    bool MuonReadoutElement::largeSector() const {
        // this doesn't apply to TGC
        if(m_statname.size() >= 3){
            char c = m_statname[2];
            if (c == 'L')
                return true;
            else if (c == 'S')
                return false;
            else {
                if (c == 'E' || c == 'F' || c == 'G') return false;
                if (c == 'M' || c == 'R') return true;
            }
        }
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" largeSector() - is this station a larger sector answer is no for readout element "<<m_idHelperSvc->toStringDetEl(identify()));
        throw std::runtime_error("Unknown sector");
        return false;
    }

    bool MuonReadoutElement::smallSector() const {
        // this doesn't apply to TGC
        return (!largeSector());
    }
 
    Amg::Transform3D MuonReadoutElement::toParentStation() const {
        return getMaterialGeom()->getX();
    }

    void MuonReadoutElement::setParentMuonStation(const MuonStation* mstat) { 
        m_parentMuonStation = mstat; 
    }

    const MuonStation* MuonReadoutElement::parentMuonStation() const { return m_parentMuonStation; }

    Amg::Vector3D MuonReadoutElement::parentMuonStationPos() const {
        return parentMuonStation()->getTransform().translation();       
    }

    Amg::Vector3D MuonReadoutElement::AmdbLRSToGlobalCoords(const Amg::Vector3D& x) const {       
        return AmdbLRSToGlobalTransform() * x;
    }

    Amg::Transform3D MuonReadoutElement::AmdbLRSToGlobalTransform() const {
        return parentMuonStation()->getTransform() * parentMuonStation()->getNativeToAmdbLRS().inverse();
    }

    Amg::Vector3D MuonReadoutElement::GlobalToAmdbLRSCoords(const Amg::Vector3D& x) const {       
        return GlobalToAmdbLRSTransform() * x;
    }

    Amg::Transform3D MuonReadoutElement::GlobalToAmdbLRSTransform() const {
        return parentMuonStation()->getNativeToAmdbLRS() * parentMuonStation()->getTransform().inverse();
    }
    void MuonReadoutElement::setIdentifier(const Identifier& id) {
        m_id = id;
        if (!m_idHelperSvc->isMuon(id)) {
            ATH_MSG_FATAL("The Identifier "<<m_idHelperSvc->toString(id)<<" is not a muon one.");
            throw std::runtime_error("Invalid Identifier set");
        }
        m_stIdx = m_idHelperSvc->stationName(id);
        m_eta = m_idHelperSvc->stationEta(id);
        m_phi = m_idHelperSvc->stationPhi(id);
        m_idhash = m_idHelperSvc->moduleHash(id);
        m_detectorElIdhash = m_idHelperSvc->detElementHash(id);
    }
    void MuonReadoutElement::setTechnologyName(const std::string& str) { m_techname = str; }
    void MuonReadoutElement::setStationName(const std::string& str) { m_statname = str; }
    void MuonReadoutElement::setStationS(double v) { m_stationS = v; }
    void MuonReadoutElement::setLongSsize(double v) { m_LongSsize = v; }
    void MuonReadoutElement::setLongRsize(double v) { m_LongRsize = v; }
    void MuonReadoutElement::setLongZsize(double v) { m_LongZsize = v; }
    void MuonReadoutElement::setSsize(double v) { m_Ssize = v; }
    void MuonReadoutElement::setRsize(double v) { m_Rsize = v; }
    void MuonReadoutElement::setZsize(double v) { m_Zsize = v; }
    void MuonReadoutElement::refreshCache() {
        clearCache();
        fillCache();
    }
}  // namespace MuonGM
