/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AlignableTrackingVolume.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkGeometry/AlignableTrackingVolume.h"

#include "TrkGeometry/TrackingVolume.h"
#include "TrkVolumes/VolumeBounds.h"

Trk::AlignableTrackingVolume::AlignableTrackingVolume(
    Amg::Transform3D* htrans,
    Amg::Transform3D* align,
    VolumeBounds* volbounds,
    const Trk::BinnedMaterial* matprop,
    int sampleID,
    const std::string& volumeName)
    : Trk::TrackingVolume(htrans, volbounds, *matprop, nullptr, nullptr, volumeName),
      m_alignment(align),
      m_alignedTV(nullptr),
      m_binnedMaterial(matprop),
      m_sampleID(sampleID){
  if (m_alignment) {
    m_alignedTV = std::unique_ptr<TrackingVolume>(this->cloneTV(*m_alignment));
  }
}

const Trk::TrackingVolume* Trk::AlignableTrackingVolume::alignedTrackingVolume() const {
  if (m_alignedTV) {
    return m_alignedTV.get();
  }
  return this;
}
