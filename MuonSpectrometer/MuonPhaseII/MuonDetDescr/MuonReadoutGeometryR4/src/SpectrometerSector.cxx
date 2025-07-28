/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Geometry/Volume.hpp>
#include <ActsGeoUtils/NoDeletePtr.h>

#include <format>

namespace MuonGMR4 {

using ChamberSet = SpectrometerSector::ChamberSet;
using BoundEnums = Acts::TrapezoidVolumeBounds::BoundValues;

SpectrometerSector::SpectrometerSector(defineArgs&& args):
    m_args{std::move(args)} {
        for (auto & chamber : m_args.chambers) {
            chamber->setParent(this);
        }
    }

bool SpectrometerSector::operator<(const SpectrometerSector& other) const {
    if (side() != other.side()) {
        return side() < other.side();
    }
    if (sector() != other.sector()) {
        return sector() < other.sector();
    }
    if (other.chamberIndex() != chamberIndex()) {
        return chamberIndex() < other.chamberIndex();
    }
    return (*m_args.chambers.front()) < (*other.m_args.chambers.front()); 
}
int8_t SpectrometerSector::side() const {
    return m_args.chambers.front()->stationEta() > 0 ? 1 : -1;
}
const SpectrometerSector::defineArgs& SpectrometerSector::parameters() const { return m_args; }
const Muon::IMuonIdHelperSvc* SpectrometerSector::idHelperSvc() const { return m_args.chambers.front()->idHelperSvc();}
Muon::MuonStationIndex::ChIndex SpectrometerSector::chamberIndex() const { return m_args.chambers.front()->chamberIndex(); }
int SpectrometerSector::stationPhi() const { return m_args.chambers.front()->stationPhi(); }
int SpectrometerSector::sector() const {return m_args.chambers.front()->sector(); }
bool SpectrometerSector::barrel() const { return m_args.chambers.front()->barrel(); }
std::string SpectrometerSector::identString() const {
    return std::format("{:} {:}-side sector: {:2}",  
                       Muon::MuonStationIndex::chName(chamberIndex()), 
                       side() == 1 ? 'A' : 'C' , sector());
}
const ChamberSet& SpectrometerSector::chambers() const{ return m_args.chambers; }
const Acts::PlaneSurface& SpectrometerSector::surface() const {
    return *m_args.surface;
}
const Amg::Transform3D& SpectrometerSector::localToGlobalTrans(const ActsGeometryContext& gctx) const {
    return surface().transform(gctx.context());
}            
Amg::Transform3D SpectrometerSector::globalToLocalTrans(const ActsGeometryContext& gctx) const {
    return localToGlobalTrans(gctx).inverse(); 
}
double SpectrometerSector::halfXLong() const { return m_args.bounds->get(BoundEnums::eHalfLengthXposY); }
double SpectrometerSector::halfXShort() const { return m_args.bounds->get(BoundEnums::eHalfLengthXnegY); }
double SpectrometerSector::halfY() const { return  m_args.bounds->get(BoundEnums::eHalfLengthY); }
double SpectrometerSector::halfZ() const { return m_args.bounds->get(BoundEnums::eHalfLengthZ); }


std::shared_ptr<Acts::Volume> SpectrometerSector::boundingVolume(const ActsGeometryContext& gctx) const {
    return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds());
}
std::shared_ptr<Acts::TrapezoidVolumeBounds> SpectrometerSector::bounds() const {
    return m_args.bounds;
}
Chamber::ReadoutSet SpectrometerSector::readoutEles() const {
    Chamber::ReadoutSet toReturn{};
    for (const ChamberPtr& ch : chambers()) {
        toReturn.insert(toReturn.end(), ch->readoutEles().begin(), ch->readoutEles().end());
    }
    return toReturn;
}
const std::vector<SpectrometerSector::chamberLocation> & SpectrometerSector::chamberLocations() const{
    return m_args.detectorLocs;

}
std::ostream& operator<<(std::ostream& ostr, 
                         const SpectrometerSector::defineArgs& args) {
    ostr<<std::endl
        <<"halfX (S/L): "<<args.bounds->get(BoundEnums::eHalfLengthXnegY)
        <<"/"<<args.bounds->get(BoundEnums::eHalfLengthXposY)<<" [mm], ";
    ostr<<"halfY: "<<args.bounds->get(BoundEnums::eHalfLengthY)<<" [mm], ";
    ostr<<"halfZ: "<<args.bounds->get(BoundEnums::eHalfLengthZ)<<" [mm], ";
    ostr<<"************************************************************************"<<std::endl;
    for (const SpectrometerSector::ChamberPtr& ch : args.chambers) {
        ostr<<" --- "<<(*ch)<<std::endl;
    }
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const SpectrometerSector& chamber) {
    ostr<<"MS sector "<<chamber.identString()<<" "<<chamber.parameters();
    return ostr;
}

}
#endif
