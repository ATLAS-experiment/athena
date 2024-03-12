/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Muon
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilder.h"

// constructor
Muon::MuonTrackingGeometryBuilder::MuonTrackingGeometryBuilder(
    const std::string& t, const std::string& n, const IInterface* p)
    : MuonTrackingGeometryBuilderImpl(t, n, p) {
    declareInterface<Trk::IGeometryBuilder>(this);
}

// Athena standard methods
// initialize

StatusCode Muon::MuonTrackingGeometryBuilder::initialize() {
    // Retrieve the station builder (if configured)
    // -------------------------------------------
    ATH_CHECK(m_stationBuilder.retrieve(EnableTool{m_muonActive}));
    if (!m_muonActive) m_activeAdjustLevel = 0;
    //
    // Retrieve the inert material builder builder (if configured)
    // -------------------------------------------
    ATH_CHECK(m_inertBuilder.retrieve(EnableTool{m_muonInert || m_blendInertMaterial}));
    
    if (!m_muonInert)
        m_inertAdjustLevel = 0;

    return MuonTrackingGeometryBuilderImpl::initialize();
}

std::unique_ptr<Trk::TrackingGeometry>
Muon::MuonTrackingGeometryBuilder::trackingGeometry(Trk::TrackingVolume* tvol) const {
    // process muon material objects
    std::vector<std::unique_ptr<Trk::DetachedTrackingVolume>> stations;
    if (m_muonActive && m_stationBuilder) {
        stations = m_stationBuilder->buildDetachedTrackingVolumes();
    }

  
   std::vector<std::unique_ptr<Trk::DetachedTrackingVolume> > inertObjs;
    if (m_muonInert && m_inertBuilder) {
        inertObjs = m_inertBuilder->buildDetachedTrackingVolumes(m_blendInertMaterial);
    }

    return MuonTrackingGeometryBuilderImpl::trackingGeometryImpl(std::move(stations), std::move(inertObjs), tvol);
}
