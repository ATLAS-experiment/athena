/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilderCond.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Muon
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilderCond.h"

#include "AthenaKernel/IOVInfiniteRange.h"

Muon::MuonTrackingGeometryBuilderCond::MuonTrackingGeometryBuilderCond(
    const std::string& t, const std::string& n, const IInterface* p)
    : MuonTrackingGeometryBuilderImpl(t, n, p) {
    declareInterface<Trk::IGeometryBuilderCond>(this);
}

StatusCode Muon::MuonTrackingGeometryBuilderCond::initialize() {
    // Retrieve the station builder (if configured)
    // -------------------------------------------
    ATH_CHECK(m_stationBuilder.retrieve(EnableTool{m_muonActive}));

    // Retrieve the inert material builder builder (if configured)
    // -------------------------------------------

    ATH_CHECK(m_inertBuilder.retrieve(EnableTool{m_muonInert || m_blendInertMaterial}));
    return MuonTrackingGeometryBuilderImpl::initialize();
}

std::unique_ptr<Trk::TrackingGeometry>
Muon::MuonTrackingGeometryBuilderCond::trackingGeometry(const EventContext& ctx, 
                                                        Trk::TrackingVolume* tvol,
                                                        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const {
    ATH_MSG_DEBUG(" building tracking geometry");

    // process muon material objects
    
    DetachedVolVec stations{};
    if (m_muonActive && m_stationBuilder) {
        stations = m_stationBuilder->buildDetachedTrackingVolumes(ctx, whandle);
    }

    DetachedVolVec inertObjs{};
    if (m_muonInert && m_inertBuilder) {
        inertObjs = m_inertBuilder->buildDetachedTrackingVolumes(ctx, whandle, m_blendInertMaterial);
    }

    return MuonTrackingGeometryBuilderImpl::trackingGeometryImpl(std::move(stations), 
                                                                 std::move(inertObjs), tvol);
}
