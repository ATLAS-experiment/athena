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
    std::unique_ptr<Amg::Transform3D> htrans,
    std::shared_ptr<VolumeBounds> volbounds,
    const Trk::BinnedMaterial& matprop,
    int sampleID,
    const std::string& volumeName)
    : Trk::TrackingVolume(std::move(htrans), std::move(volbounds), matprop, nullptr, nullptr, volumeName),
      m_binnedMaterial(matprop),
      m_sampleID(sampleID){}
