/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AlignableTrackingVolume.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_ALIGNABLETRACKINGVOLUME_H
#define TRKGEOMETRY_ALIGNABLETRACKINGVOLUME_H

class MsgStream;

#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/BinnedMaterial.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkSurfaces/Surface.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"

namespace Trk {

class Surface;
class MaterialProperties;

/**
 @class AlignableTrackingVolume

 Base Class for a navigation object (active) in the Calo realm.
 Takes BinnedMaterial as an argument ( can be dummy )

 @author Sarka.Todorova@cern.ch

 */

class AlignableTrackingVolume : public TrackingVolume {

 public:
  /**Default Constructor*/
  AlignableTrackingVolume() = default;
  virtual ~AlignableTrackingVolume() override = default;
  /**Constructor*/
  AlignableTrackingVolume(std::unique_ptr<Amg::Transform3D> htrans,
                          std::shared_ptr<VolumeBounds> volbounds,
                          const BinnedMaterial& matprop,
                          int sampleID,
                          const std::string& volumeName = "undefined");

  /** returns the alignedTrackingVolume */
  const TrackingVolume* alignedTrackingVolume() const;
  /** returns the id */
  int identify() const;
  /** access to binned material */
  const BinnedMaterial* binnedMaterial() const;

  virtual bool isAlignable() const override final;

 private:
  std::unique_ptr<Amg::Transform3D> m_alignment = nullptr;
  const BinnedMaterial m_binnedMaterial{};
  int m_sampleID{};
};

inline int AlignableTrackingVolume::identify() const {
  return m_sampleID;
}

inline const BinnedMaterial* AlignableTrackingVolume::binnedMaterial() const {
  return &m_binnedMaterial;
}

inline bool AlignableTrackingVolume::isAlignable() const {
  return true;
}
}  // namespace Trk

#endif  // TRKGEOMETRY_ALIGNABLETRACKINGVOLUME_H

