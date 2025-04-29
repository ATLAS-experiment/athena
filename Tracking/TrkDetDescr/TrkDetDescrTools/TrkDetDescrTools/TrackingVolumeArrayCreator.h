/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeArrayCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_TRACKINGVOLUMEARRAYCREATOR_H
#define TRKDETDESCRTOOLS_TRACKINGVOLUMEARRAYCREATOR_H

// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkGeometry/TrackingVolume.h"
// STL
#include <algorithm>
#include <memory>

namespace Trk {

class Layer;
class PlaneLayer;

/** @class TrackingVolumeArrayCreator

  The TrackingVolumeArrayCreator is a simple Tool that helps to construct
  binned arrays of TrackingVolumes for both, confinement in another volume
  and navigation issues.

  @author Andreas.Salzburger@cern.ch
 */

class TrackingVolumeArrayCreator final
    : public AthAlgTool,
      virtual public ITrackingVolumeArrayCreator {

 public:
  using TrackingVolumeOrderPosition = std::pair<VolumePtr, Amg::Vector3D>;
  using TrackingVolumeNavOrder = std::pair<VolumePtr, const Amg::Transform3D*>;

  /** Constructor */
  TrackingVolumeArrayCreator(const std::string&, const std::string&,
                             const IInterface*);

  /** Destructor */
  virtual ~TrackingVolumeArrayCreator();

  /** Extra interface methods for compatibility*/
  TrackingVolumeArray* cylinderVolumesArrayInR(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const override;
  TrackingVolumeArray* cylinderVolumesArrayInZ(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const override;
  TrackingVolumeArray* cylinderVolumesArrayInPhiR(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const override;
  TrackingVolumeArray* cylinderVolumesArrayInPhiZ(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const override;

  /** TrackingVolumeArrayCreator interface method -
      create a R-binned cylindrical volume array*/
  std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInR(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const override;
  /** TrackingVolumeArrayCreator interface method -
      create a R-binned cylindrical volume array*/
  std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInZ(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const override;
  /** TrackingVolumeArrayCreator interface method -
      create a R-binned cylindrical volume array*/
  std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhi(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const override;
  /** TrackingVolumeArrayCreator interface method -
      create a 2dim cylindrical volume array*/
  std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhiR(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const override;
  /** TrackingVolumeArrayCreator interface method -
      create a 2dim cylindrical volume array*/
  std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhiZ(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const override;

  /** TrackingVolumeArrayCreator interface method -
      create a cuboid volume array - linked to detached tracking volumes */
  std::unique_ptr<TrackingVolumeArray> cuboidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const override;
  /** TrackingVolumeArrayCreator interface method -
      create a trapezoid volume array - linked to detached tracking volumes */
  std::unique_ptr<TrackingVolumeArray> trapezoidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const override;

  /** TrackingVolumeArrayCreator interface method -
      create a doubleTrapezoid volume array - linked to detached tracking
     volumes */
  std::unique_ptr<TrackingVolumeArray> doubleTrapezoidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const override;
};

}  // namespace Trk

#endif

