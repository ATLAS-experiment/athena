/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#include "MuonReadoutGeometryR4/Chamber.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <format>

namespace MuonGMR4{
   
    std::ostream& operator<<(std::ostream& ostr,
                             const Chamber::defineArgs& args) {
        ostr<<"halfX (S/L): "<<halfXlowY(*args.bounds)<<"/"<<halfXhighY(*args.bounds)<<" [mm], ";
        ostr<<"halfY: "<<halfY(*args.bounds)<<" [mm], ";
        ostr<<"halfZ: "<<halfZ(*args.bounds)<<" [mm], ";
        return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, const Chamber& chamber) {
        ostr<<chamber.identString()<<" "<<chamber.parameters();
        return ostr;
    }

    Chamber::Chamber(defineArgs&& args):
        m_args{std::move(args)} {}
    
    //Comparison operator for the MuonChambers: 
    // Legacy & NSW: sorted by StationName
    // NSW (MM) & NSW (STG) : sorted by Station Name
    // NSW (MM/STGC) & NSW(MM/STGC) : if they have same eta and phi -> sorted by multilayer (case for large sectors)
    // small NSW chambers are sorted by phi - all elements in a sector are grouped in the same chamber
    bool Chamber::operator<(const Chamber& other) const {
        if(detectorType() != other.detectorType()) {
            return detectorType() < other.detectorType();
        }
        if (stationName() != other.stationName()) {
            return stationName() < other.stationName();
        }
        if (stationPhi() != other.stationPhi()) {
            return stationPhi() < other.stationPhi();
        }
        if (stationEta() != other.stationEta()) {
            return stationEta() < other.stationEta();
        }
        return readoutEles().front()->identify() <
         other.readoutEles().front()->identify();
    }

    std::string Chamber::identString() const {
        if (readoutEles().size() == 1 || detectorType() == ActsTrk::DetectorType::Mm || detectorType() == ActsTrk::DetectorType::sTgc) {
            return std::format("MS chamber {:}",
                               idHelperSvc()->toStringDetEl(readoutEles().front()->identify()));
        }
        return std::format("MS chamber {:} station {:} eta {:02} phi {:02}",
                          ActsTrk::to_string(detectorType()),
                          idHelperSvc()->stationNameString(readoutEles().front()->identify()),
                          stationEta(), stationPhi());        
    }
    bool Chamber::barrel() const {
        return !idHelperSvc()->isEndcap(readoutEles().front()->identify());
    }
    const Muon::IMuonIdHelperSvc* Chamber::idHelperSvc() const {
        return readoutEles().front()->idHelperSvc();
    }
    Muon::MuonStationIndex::ChIndex Chamber::chamberIndex() const {
        return readoutEles().front()->chamberIndex();
    }
      
    ActsTrk::DetectorType Chamber::detectorType() const {
       return readoutEles().front()->detectorType();
    }
    double Chamber::halfXLong() const { return MuonGMR4::halfXhighY(*m_args.bounds); }
    double Chamber::halfXShort() const { return MuonGMR4::halfXlowY(*m_args.bounds);  }
    double Chamber::halfY() const { return MuonGMR4::halfY(* m_args.bounds); }
    double Chamber::halfZ() const { return MuonGMR4::halfZ(*m_args.bounds);}

    std::shared_ptr<Acts::Volume> Chamber::boundingVolume(const ActsTrk::GeometryContext& gctx) const {
        return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds());
    }
    std::shared_ptr<Acts::VolumeBounds> Chamber::bounds() const { return m_args.bounds; }
    int Chamber::stationPhi() const{ return readoutEles().front()->stationPhi(); }
    int Chamber::stationEta() const{ return readoutEles().front()->stationEta(); }
    int Chamber::stationName() const{ return readoutEles().front()->stationName(); }
    int8_t Chamber::side() const{ return Acts::copySign(1, readoutEles().front()->stationEta()); }
    int Chamber::sector() const{ return idHelperSvc()->sector(readoutEles().front()->identify()); }
    const Chamber::defineArgs& Chamber::parameters() const { return m_args; }
    const Chamber::ReadoutSet& Chamber::readoutEles() const {
        return m_args.detEles;
    }
    const Acts::PlaneSurface& Chamber::surface() const {
        return *m_args.surface;
    }
    const Amg::Transform3D& Chamber::localToGlobalTrans(const ActsTrk::GeometryContext& gctx) const {
        return surface().transform(gctx.context());
    }
    Amg::Transform3D Chamber::globalToLocalTrans(const ActsTrk::GeometryContext& gctx) const {
        return localToGlobalTrans(gctx).inverse();
    }
    const SpectrometerSector* Chamber::parent() const { return m_parent; }
    void Chamber::setParent(const SpectrometerSector* parent) { m_parent = parent; }
}
#endif