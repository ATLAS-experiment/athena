/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackingGeometry/MuonStationBuilderCond.h"

#include "StoreGate/ReadCondHandle.h"

StatusCode Muon::MuonStationBuilderCond::initialize() {
    ATH_CHECK(m_muonMgrReadKey.initialize());
    return Muon::MuonStationBuilderImpl::initialize();
}

Muon::MuonStationBuilderCond::DetachedVolVec
Muon::MuonStationBuilderCond::buildDetachedTrackingVolumes(const EventContext& ctx,
    SG::WriteCondHandle<Trk::TrackingGeometry>& whandle, bool blend) const {

    SG::ReadCondHandle readHandle{m_muonMgrReadKey, ctx};
    if (!readHandle.isValid() ) {
        ATH_MSG_FATAL(m_muonMgrReadKey.fullKey() << " is not available.");
        throw std::runtime_error("No detector manager available");
    }
    whandle.addDependency(readHandle);
 return Muon::MuonStationBuilderImpl::buildDetachedTrackingVolumesImpl(readHandle.cptr(), blend);
}
