/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Muon
#include "MuonTrackingGeometry/MuonInertMaterialBuilderCond.h"

#include "StoreGate/ReadCondHandle.h"

// constructor
Muon::MuonInertMaterialBuilderCond::MuonInertMaterialBuilderCond(
    const std::string& t, const std::string& n, const IInterface* p)
    : MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilderCond::initialize() {
    ATH_CHECK(m_muonMgrReadKey.initialize());
    return Muon::MuonInertMaterialBuilderImpl::initialize();
}

Muon::MuonInertMaterialBuilderCond::DetachedVolVec
    Muon::MuonInertMaterialBuilderCond::buildDetachedTrackingVolumes(const EventContext& ctx,
                                                                     SG::WriteCondHandle<Trk::TrackingGeometry>& whandle, 
                                                                     bool blend) const {

    SG::ReadCondHandle<MuonGM::MuonDetectorManager> readHandle{m_muonMgrReadKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_ERROR(m_muonMgrReadKey.fullKey()
            << " is not available. Could not get MuonDetectorManager, no layers for muons will be built.");
        throw std::runtime_error("Failed to build tracking inert material");
    }
    whandle.addDependency(readHandle);
    return Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumesImpl(readHandle->getTreeTop(0), blend);
}
