/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Muon
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilder.h"

// constructor
Muon::MuonTrackingGeometryBuilder::MuonTrackingGeometryBuilder(const std::string& t, const std::string& n,
                                                               const IInterface* p)
    : MuonTrackingGeometryBuilderImpl(t, n, p) {
  declareInterface<Trk::IGeometryBuilder>(this);
}

// Athena standard methods
// initialize

StatusCode Muon::MuonTrackingGeometryBuilder::initialize() {
  // Retrieve the station builder (if configured) -------------------------------------------
  if (m_muonActive) {
    if (m_stationBuilder.retrieve().isFailure()) {
      ATH_MSG_ERROR("Failed to retrieve tool " << m_stationBuilder << " Creation of stations might fail.");
    } else
      ATH_MSG_INFO("Retrieved tool " << m_trackingVolumeArrayCreator);
  } else {
    m_activeAdjustLevel = 0;  // no active material to consider
  }

  // Retrieve the inert material builder builder (if configured) -------------------------------------------
  if (m_muonInert || m_blendInertMaterial) {
    if (m_inertBuilder.retrieve().isFailure()) {
      ATH_MSG_ERROR("Failed to retrieve tool " << m_inertBuilder << "Creation of inert material objects might fail.");
    } else
      ATH_MSG_INFO("Retrieved tool " << m_trackingVolumeArrayCreator);
  }

  if (!m_muonInert) m_inertAdjustLevel = 0;

  return MuonTrackingGeometryBuilderImpl::initialize();
}

std::unique_ptr<Trk::TrackingGeometry> Muon::MuonTrackingGeometryBuilder::trackingGeometry(Trk::TrackingVolume* tvol) const {
  // process muon material objects
  std::unique_ptr<const std::vector<std::unique_ptr<Trk::DetachedTrackingVolume> > > stations;
  if (m_muonActive && m_stationBuilder) {
    stations = m_stationBuilder->buildDetachedTrackingVolumes();
  }

  std::unique_ptr<const std::vector<std::unique_ptr<Trk::DetachedTrackingVolume> > > inertObjs;

  if (m_muonInert && m_inertBuilder) {
    inertObjs = m_inertBuilder->buildDetachedTrackingVolumes(m_blendInertMaterial);
  }

  return MuonTrackingGeometryBuilderImpl::trackingGeometryImpl(std::move(stations), std::move(inertObjs), tvol);
}
