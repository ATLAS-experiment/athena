/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITrackingVolumeArrayCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ITRACKINGVOLUMEARRAYCREATOR_H
#define TRKDETDESCRINTERFACES_ITRACKINGVOLUMEARRAYCREATOR_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// TrkDetDescrUtils - templated classes & enums
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinningType.h"
// STL
#include <memory>
#include <vector>
namespace Trk {

/** forward declarations*/
class TrackingVolume;

/** @typedef TrackingVolumeArray
    simply for the eye */
using TrackingVolumeArray = BinnedArray<TrackingVolume>;

/** @class ITrackingVolumeArrayCreator

  Interface class ITrackingVolumeArrayCreators
  It inherits from IAlgTool. The actual implementation of the AlgTool
  can be found in TrkDetDescrTools as the LayerArrayCreator.

  It is designed to centralize the code to create
  Arrays of Tracking Volumes for both:
    - confinedment in another TrackingVolume
    - navigation and glueing

  @author Andreas.Salzburger@cern.ch
  */
class ITrackingVolumeArrayCreator : virtual public IAlgTool {

 public:
  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(ITrackingVolumeArrayCreator, 1, 0);
  using VolumePtr = std::shared_ptr<TrackingVolume>;
  /**Virtual destructor*/
  virtual ~ITrackingVolumeArrayCreator() = default;

  /** Extra interface methods for compatibility*/
  virtual TrackingVolumeArray* cylinderVolumesArrayInR(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const = 0;
  virtual TrackingVolumeArray* cylinderVolumesArrayInZ(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const = 0;
  virtual TrackingVolumeArray* cylinderVolumesArrayInPhiR(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const = 0;
  virtual TrackingVolumeArray* cylinderVolumesArrayInPhiZ(
      const std::vector<TrackingVolume*>& vols,
      bool navigationtype = false) const = 0;

  /** TrackingVolumeArrayCreator interface method -
      create a R-binned cylindrical volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInR(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const = 0;
  /** TrackingVolumeArrayCreator interface method -
      create a R-binned cylindrical volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInZ(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const = 0;
  /** TrackingVolumeArrayCreator interface method -
      create a Phi-binned cylindrical volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhi(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const = 0;
  /** TrackingVolumeArrayCreator interface method -
      create a 2dim cylindrical volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhiR(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const = 0;
  /** TrackingVolumeArrayCreator interface method -
      create a 2dim cylindrical volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cylinderVolumesArrayInPhiZ(
      const std::vector<VolumePtr>& vols,
      bool navigationtype = false) const = 0;
  /** TrackingVolumeArrayCreator interface method -
       create a cuboid volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> cuboidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const = 0;
  /** TrackingVolumeArrayCreator interface method -
       create a trapezoid volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> trapezoidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const = 0;
  /** TrackingVolumeArrayCreator interface method -
       create a doubleTrapezoid volume array*/
  virtual std::unique_ptr<TrackingVolumeArray> doubleTrapezoidVolumesArrayNav(
      const std::vector<VolumePtr>& vols,
      const Trk::BinUtility& binUtil) const = 0;
};

}  // namespace Trk

#endif

