/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTrackingGeometry/MuonStationBuilderCond.h"

#include "StoreGate/ReadCondHandle.h"
// constructor
Muon::MuonStationBuilderCond::MuonStationBuilderCond(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
    : Muon::MuonStationBuilderImpl(t, n, p) {
    declareInterface<Trk::IDetachedTrackingVolumeBuilderCond>(this);
}

StatusCode Muon::MuonStationBuilderCond::initialize() {
    ATH_CHECK(m_muonMgrReadKey.initialize());
    return Muon::MuonStationBuilderImpl::initialize();
}

std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
Muon::MuonStationBuilderCond::buildDetachedTrackingVolumes(
    const EventContext& ctx,
    SG::WriteCondHandle<Trk::TrackingGeometry>& whandle, bool blend) const {

    SG::ReadCondHandle<MuonGM::MuonDetectorManager> readHandle{m_muonMgrReadKey,
                                                               ctx};
    if (!readHandle.isValid() || !(*readHandle)) {
        ATH_MSG_FATAL(m_muonMgrReadKey.fullKey() << " is not available.");
        return {};
    }
    whandle.addDependency(readHandle);

    const MuonGM::MuonDetectorManager* muonMgr = readHandle.cptr();

    return Muon::MuonStationBuilderImpl::buildDetachedTrackingVolumesImpl(
        muonMgr, blend);
}
