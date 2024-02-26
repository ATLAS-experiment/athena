/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonStationGeoHelpers/MuonChamber.h>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>


namespace {
    inline bool isNsw(const MuonGMR4::MuonChamber& chamber) {
        return chamber.detectorType() == ActsTrk::DetectorType::Mm ||
               chamber.detectorType() == ActsTrk::DetectorType::sTgc;
    }
    constexpr int sign(const int num){
        return num >= 0 ? 1 : -1;
    }
}

namespace MuonGMR4 {

using ReadoutSet = MuonChamber::ReadoutSet;
using SubDetAlignments = ActsGeometryContext::SubDetAlignments;
using AlignStorePtr = GeoModel::TransientConstSharedPtr<ActsTrk::AlignmentStore>;

MuonChamber::MuonChamber(defineArgs&& args):
    m_args{std::move(args)} {}
MuonChamber::MuonChamber(const MuonChamber& other):
    m_args{other.m_args} {}
MuonChamber::MuonChamber(MuonChamber&& other):
    m_args{std::move(other.m_args)} {}
MuonChamber& MuonChamber::operator=(const MuonChamber& other){
    if (this != &other) {
        m_args = other.m_args;
    }
    return *this;
}
MuonChamber& MuonChamber::operator=(MuonChamber&& other) {
    if (this != &other) {
        m_args = std::move(other.m_args);
    }
    return *this;
}
PVConstLink MuonChamber::physVol() const { return m_args.chamberVol; }
const MuonChamber::defineArgs& MuonChamber::parameters() const { return m_args; }
const Muon::IMuonIdHelperSvc* MuonChamber::idHelperSvc() const { return m_args.readoutEles[0]->idHelperSvc();}
Muon::MuonStationIndex::ChIndex MuonChamber::chamberIndex() const { return m_args.readoutEles[0]->chamberIndex(); }
int MuonChamber::stationName() const { return m_args.readoutEles[0]->stationName(); }
int MuonChamber::stationPhi() const { return m_args.readoutEles[0]->stationPhi(); }
int MuonChamber::stationEta() const { return m_args.readoutEles[0]->stationEta(); }
ActsTrk::DetectorType MuonChamber::detectorType() const { return m_args.readoutEles[0]->detectorType(); }
const ReadoutSet& MuonChamber::readOutElements() const{ return m_args.readoutEles; }
const Amg::Transform3D& MuonChamber::localToGlobalTrans(const ActsGeometryContext& gctx) const {
    SubDetAlignments::const_iterator itr = gctx.alignmentStores.find(detectorType());
    return m_localToGlobal.getTransform(itr != gctx.alignmentStores.end() ? itr->second.get() : nullptr);
}            
const Amg::Transform3D& MuonChamber::globalToLocalTrans(const ActsGeometryContext& gctx) const {
    SubDetAlignments::const_iterator itr = gctx.alignmentStores.find(detectorType());
    return m_globalToLocal.getTransform(itr != gctx.alignmentStores.end() ? itr->second.get() : nullptr); 
}
Amg::Transform3D MuonChamber::fromLayerToGlobal(ActsTrk::RawGeomAlignStore* store) const {
    ActsGeometryContext gctx{};
    /// If the store is given, assume that the tracking alignment already caches the transformations
    /// of the needed detector surfaces --> We can build a geo context on the fly.
    if (store) {
        gctx.alignmentStores[detectorType()] = store->trackingAlignment;
    }        
    return m_args.readoutEles[0]->localToGlobalTrans(gctx) * m_args.centerTrans;
}
bool MuonChamber::storeAlignment(ActsTrk::RawGeomAlignStore& store) const {
    if (store.detType != detectorType()) return false;
    m_localToGlobal.storeAlignment(store);
    m_globalToLocal.storeAlignment(store);
    return true;
}
double MuonChamber::halfXLong() const { return m_args.halfXLong; }
double MuonChamber::halfXShort() const { return m_args.halfXShort; }
double MuonChamber::halfY() const { return m_args.halfY; }
double MuonChamber::halfZ() const { return m_args.halfZ; }
int MuonChamber::sector() const {return idHelperSvc()->sector(m_args.readoutEles[0]->identify()); }

std::shared_ptr<Acts::Volume> MuonChamber::boundingVolume(const ActsGeometryContext& gctx) const {
    return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds());
}
std::shared_ptr<Acts::TrapezoidVolumeBounds> MuonChamber::bounds() const {
    return std::make_shared<Acts::TrapezoidVolumeBounds>(halfXLong(), halfXShort(), halfY(), halfZ());
}

bool operator<(const MuonChamber& a, const MuonChamber& b) { 
    if (isNsw(a) || isNsw(b)) {
        if (isNsw(a) != isNsw(b)) return isNsw(a);
        const int secA = a.sector();
        const int secB = b.sector();
        if (secA != secB) return secA < secB;
        return sign(a.stationEta()) < sign(b.stationEta());
    }  
    if (a.stationName() != b.stationName()) {
        return a.stationName() < b.stationName();
    }
    if (a.stationEta() != b.stationEta()) {
        return a.stationEta() < b.stationEta();
    }
    return a.stationPhi() < b.stationPhi();
}
bool operator<(const Identifier& a, const MuonChamber& b) {
    const bool isNswA = b.idHelperSvc()->isMM(a) || b.idHelperSvc()->issTgc(a);
    if (isNsw(b) || isNswA) {
        if (isNsw(b) != isNswA) return isNswA;
        const int secB = b.sector();
        const int secA = b.idHelperSvc()->sector(a);
        if (secA != secB) return secA < secB;
        return sign(b.idHelperSvc()->stationEta(a)) < sign(b.stationEta());
    } 
    const int stName = b.idHelperSvc()->stationName(a);
    if (stName != b.stationName()) { return stName < b.stationName(); }
    const int stEta = b.idHelperSvc()->stationEta(a);
    if (stEta != b.stationEta()) { return  stEta < b.stationEta(); }
    return  b.idHelperSvc()->stationPhi(a) < b.stationPhi();
}
bool operator<(const MuonChamber& a, const Identifier& b) {
    const bool isNswB = a.idHelperSvc()->isMM(b) || a.idHelperSvc()->issTgc(b);
    if (isNsw(a) || isNswB) {
        if (isNsw(a) != isNswB) return isNsw(a);
        const int secA = a.sector();
        const int secB = a.idHelperSvc()->sector(b);
        if (secA != secB) return secA < secB;
        return sign(a.stationEta()) < sign(a.idHelperSvc()->stationEta(b));        
    } 
    const int stName = a.idHelperSvc()->stationName(b);
    if (stName != a.stationName()) { return a.stationName() < stName; }
    
    const int stEta = a.idHelperSvc()->stationEta(b);
    if (stEta != a.stationEta()) { return a.stationEta() < stEta; }
    return a.stationPhi() < a.idHelperSvc()->stationPhi(b);
}

bool operator<(const MuonReadoutElement& a, const MuonChamber& b) { return a.identify() < b; }
bool operator<(const MuonChamber& a, const MuonReadoutElement& b) { return a < b.identify(); }

std::ostream& operator<<(std::ostream& ostr, 
                         const MuonChamber::defineArgs& args) {
    ostr<<"halfX (S/L): "<<args.halfXShort<<"/"<<args.halfXLong<<" [mm], ";
    ostr<<"halfY: "<<args.halfY<<" [mm], ";
    ostr<<"halfZ: "<<args.halfZ<<" [mm],";
    ostr<<" center w.r.t chamber: "<<Amg::toString(args.centerTrans, 2);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr,
                         const MuonChamber& chamber) {
    ostr<<chamber.parameters();
    return ostr;
}

}
