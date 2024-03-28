/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackingGeometry/MuonInertMaterialBuilder.h"
// constructor
Muon::MuonInertMaterialBuilder::MuonInertMaterialBuilder(const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p)
    : Muon::MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilder::initialize() {
    ATH_CHECK(detStore()->retrieve(m_muonMgr));
    return Muon::MuonInertMaterialBuilderImpl::initialize();
}

std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>
    Muon::MuonInertMaterialBuilder::buildDetachedTrackingVolumes(bool blend) const {

    return Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumesImpl(m_muonMgr->getTreeTop(0), blend);
}
