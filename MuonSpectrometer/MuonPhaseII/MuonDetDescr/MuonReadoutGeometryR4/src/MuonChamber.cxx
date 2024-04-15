/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE
#include <MuonReadoutGeometryR4/MuonChamber.h>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <ActsGeoUtils/NoDeletePtr.h>


namespace ActsTrk {
template<> 
    TransformCacheDetEle<MuonGMR4::MuonChamber>::TransformCacheDetEle(const IdentifierHash& hash, 
                                                                     const MuonGMR4::MuonChamber* chamber):
        TransformCache{hash, chamber->detectorType()},
        m_parent{chamber} {}
template<> const IDetectorElement*
        TransformCacheDetEle<MuonGMR4::MuonChamber>::parent() const { return nullptr; }
template <> Amg::Transform3D 
    TransformCacheDetEle<MuonGMR4::MuonChamber>::fetchTransform(const DetectorAlignStore* store) const{
        ActsGeometryContext gctx{};
        /// If the store is given, assume that the tracking alignment already caches the transformations
        /// of the needed detector surfaces --> We can build a geo context on the fly.
        if (store) {
            auto copyStore = std::make_unique<DetectorAlignStore>(detectorType());
            copyStore->geoModelAlignment = store->geoModelAlignment;
            copyStore->trackingAlignment = store->trackingAlignment;
            copyStore->internalAlignment = store->internalAlignment;
            gctx.setStore(std::move(copyStore)); 
        }        
        const auto& pars{m_parent->parameters()};
        return pars.readoutEles[0]->localToGlobalTrans(gctx) * pars.centerTrans;

    }

}


namespace MuonGMR4 {

using ReadoutSet = MuonChamber::ReadoutSet;

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
const MuonChamber::defineArgs& MuonChamber::parameters() const { return m_args; }
const Muon::IMuonIdHelperSvc* MuonChamber::idHelperSvc() const { return m_args.readoutEles[0]->idHelperSvc();}
Muon::MuonStationIndex::ChIndex MuonChamber::chamberIndex() const { return m_args.readoutEles[0]->chamberIndex(); }
int MuonChamber::stationName() const { return m_args.readoutEles[0]->stationName(); }
int MuonChamber::stationPhi() const { return m_args.readoutEles[0]->stationPhi(); }
int MuonChamber::stationEta() const { return m_args.readoutEles[0]->stationEta(); }
ActsTrk::DetectorType MuonChamber::detectorType() const { return m_args.readoutEles[0]->detectorType(); }
const ReadoutSet& MuonChamber::readOutElements() const{ return m_args.readoutEles; }
const Amg::Transform3D& MuonChamber::localToGlobalTrans(const ActsGeometryContext& gctx) const {
    return m_localToGlobal.getTransform(gctx.getStore(detectorType()).get());
}            
Amg::Transform3D MuonChamber::globalToLocalTrans(const ActsGeometryContext& gctx) const {
    return localToGlobalTrans(gctx).inverse(); 
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
#endif