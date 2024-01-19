/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Muon
#include "MuonTrackingGeometry/MuonInertMaterialBuilderCond.h"

#include "StoreGate/ReadCondHandle.h"

// constructor
Muon::MuonInertMaterialBuilderCond::MuonInertMaterialBuilderCond(const std::string& t, const std::string& n,
                                                                 const IInterface* p)
    : MuonInertMaterialBuilderImpl(t, n, p) {}

StatusCode Muon::MuonInertMaterialBuilderCond::initialize() {
  if (m_muonMgrReadKey.initialize().isFailure()) {
    ATH_MSG_ERROR("Could not get MuonDetectorManager, no layers for muons will be built. ");
  }
  return Muon::MuonInertMaterialBuilderImpl::initialize();
}


std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>>>
Muon::MuonInertMaterialBuilderCond::buildDetachedTrackingVolumes(const EventContext& ctx,
                                                                 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
                                                                 bool blend) const {

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> readHandle{m_muonMgrReadKey, ctx};
  if (!readHandle.isValid() || !(*readHandle)) {
    ATH_MSG_ERROR(m_muonMgrReadKey.fullKey()
                  << " is not available. Could not get MuonDetectorManager, no layers for muons will be built.");
    return {};
  }
  whandle.addDependency(readHandle);
  const MuonGM::MuonDetectorManager* muonMgr = *readHandle;
  if (!muonMgr) {
    ATH_MSG_FATAL("Somehow the Muon detector manager is missing ");
    return {};
  }
  return Muon::MuonInertMaterialBuilderImpl::buildDetachedTrackingVolumesImpl(muonMgr, blend);
}

