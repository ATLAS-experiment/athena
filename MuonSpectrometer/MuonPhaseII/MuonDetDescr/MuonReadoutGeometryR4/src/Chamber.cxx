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
    using BoundEnums = Acts::TrapezoidVolumeBounds::BoundValues;

    std::ostream& operator<<(std::ostream& ostr,
                             const Chamber::defineArgs& args) {
        ostr<<"halfX (S/L): "<<args.bounds->get(BoundEnums::eHalfLengthXnegY)
            <<"/"<<args.bounds->get(BoundEnums::eHalfLengthXposY)<<" [mm], ";
        ostr<<"halfY: "<<args.bounds->get(BoundEnums::eHalfLengthY)<<" [mm], ";
        ostr<<"halfZ: "<<args.bounds->get(BoundEnums::eHalfLengthZ)<<" [mm], ";
        return ostr;
    }
    std::ostream& operator<<(std::ostream& ostr, const Chamber& chamber) {

        ostr<<chamber.identString()<<" "<<chamber.parameters();
        return ostr;
    }

    Chamber::Chamber(defineArgs&& args):
        m_args{std::move(args)} {}
    
    bool Chamber::operator<(const Chamber& other) const {
        if (stationName() != other.stationName()) {
                return stationName() < other.stationName();
        }
        if (stationEta() != other.stationEta()) {
            return stationEta() < other.stationEta();
        }
        return stationPhi() < other.stationPhi();
    }
    std::string Chamber::identString() const {
        return std::format("MS chamber {:} eta {:02} phi {:02}",
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
    double Chamber::halfXLong() const { return m_args.bounds->get(BoundEnums::eHalfLengthXposY); }
    double Chamber::halfXShort() const { return m_args.bounds->get(BoundEnums::eHalfLengthXnegY); }
    double Chamber::halfY() const { return  m_args.bounds->get(BoundEnums::eHalfLengthY); }
    double Chamber::halfZ() const { return m_args.bounds->get(BoundEnums::eHalfLengthZ); }

    std::shared_ptr<Acts::Volume> Chamber::boundingVolume(const ActsGeometryContext& gctx) const {
        return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds());
    }
    std::shared_ptr<Acts::TrapezoidVolumeBounds> Chamber::bounds() const {
        return m_args.bounds;
    }
    int Chamber::stationPhi() const{ return readoutEles().front()->stationPhi(); }
    int Chamber::stationEta() const{ return readoutEles().front()->stationEta(); }
    int Chamber::stationName() const{ return readoutEles().front()->stationName(); }
    int Chamber::sector() const{ return idHelperSvc()->sector(readoutEles().front()->identify()); }
    const Chamber::defineArgs& Chamber::parameters() const { return m_args; }
    const Chamber::ReadoutSet& Chamber::readoutEles() const {
        return m_args.detEles;
    }
    const Acts::PlaneSurface& Chamber::surface() const {
        return *m_args.surface;
    }
    const Amg::Transform3D& Chamber::localToGlobalTrans(const ActsGeometryContext& gctx) const {
        return surface().transform(gctx.context());
    }
    Amg::Transform3D Chamber::globalToLocalTrans(const ActsGeometryContext& gctx) const {
        return localToGlobalTrans(gctx).inverse();
    }
    const SpectrometerSector* Chamber::parent() const { return m_parent; }
    void Chamber::setParent(const SpectrometerSector* parent) { m_parent = parent; }
}
#endif