/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackingGeometry/MuonInertMaterialBuilder.h"
// constructor
Muon::MuonInertMaterialBuilder::MuonInertMaterialBuilder(const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p)
    : Muon::MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilder::initialize() {
    StatusCode sc = detStore()->retrieve(m_muonMgr);
    sc = Muon::MuonInertMaterialBuilderImpl::initialize();
    return sc;
}

std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
Muon::MuonInertMaterialBuilder::buildDetachedTrackingVolumes(bool blend) const {

    return Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumesImpl(
        m_muonMgr, blend);
}
