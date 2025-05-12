/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BoundarySurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKVOLUMES_BOUNDARYSURFACE_H
#define TRKVOLUMES_BOUNDARYSURFACE_H

// Trk
#include "TrkDetDescrUtils/BinnedArray.h"
#include <memory>
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkVolumes/Volume.h"
// Gaudi
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoPrimitives/GeoPrimitives.h"

namespace Trk {

// class TrackParameters;
class Surface;

/**
 @class BoundarySurface

 Description of a BoundarySurface inside the tracking realm,
 it extends the Surface description to make a surface being a boundary of a
 Trk::Volume, Trk::TrackingVolume or a Trk::MagneticFieldVolume.

 A Trk::BoundarySurface can have an inside Volume and an outside Volume, resp.
 a Trk::BinnedArray for inside or outside direction.

 @author Andreas.Salzburger@cern.ch
*/


template <class Tvol>
class BoundarySurface {
  /** typedef the BinnedArray */
  typedef BinnedArray<Tvol> VolumeArray;
 public:
  /** Default Constructor - needed for pool and inherited classes */
  BoundarySurface() = default;

  /** Constructor for a Boundary with exact two Volumes attached to it*/
  BoundarySurface(const Tvol* inside, const Tvol* outside)
      : m_insideVolume(inside),
        m_outsideVolume(outside),
        m_insideVolumeArray(),
        m_outsideVolumeArray() {}

  /** Constructor for a Boundary with exact two Volumes attached to it*/
  BoundarySurface(std::shared_ptr<const VolumeArray> insideArray,
                  std::shared_ptr<const VolumeArray> outsideArray)
      : m_insideVolume(),
        m_outsideVolume(),
        m_insideVolumeArray(std::move(insideArray)),
        m_outsideVolumeArray(std::move(outsideArray)) {}

  /** Get the next Volume depending on the TrackParameters and the requested
   * direction */
  virtual const Tvol* attachedVolume(const TrackParameters& parms,
                                     PropDirection dir) const = 0;

  /** Get the next Volume depending on
   GlobalPosition, GlobalMomentum, dir
   on the TrackParameters and the requested direction */
  virtual const Tvol* attachedVolume(const Amg::Vector3D& pos,
                                     const Amg::Vector3D& mom,
                                     PropDirection dir) const = 0;

  /** templated onBoundary method */
  template <class T>
  bool onBoundary(const T& pars) const {
    return surfaceRepresentation().onSurface(pars);
  }

  /** The Surface Representation of this */
  virtual const Surface& surfaceRepresentation() const = 0;
  virtual Surface& surfaceRepresentation() = 0;

  /**Virtual Destructor*/
  virtual ~BoundarySurface() {}

  /** output debug information */
  void debugInfo(MsgStream& msg) const;

  /** getters/setters for inside/outside Volume*/
  Tvol const* insideVolume() const{
    return m_insideVolume;
  }
  void setInsideVolume(const Tvol* vol){
    m_insideVolume = vol;
  }

  Tvol const* outsideVolume() const{
    return m_outsideVolume;
  }
  void setOutsideVolume(const Tvol* vol){
    m_outsideVolume = vol;
  }

  /** getters/setters for inside/outside Volume arrays */
  const VolumeArray* insideVolumeArray() const{
    return m_insideVolumeArray.get();
  }
  void setInsideVolumeArray(std::shared_ptr<const VolumeArray> volArray){
    m_insideVolumeArray = std::move(volArray);
  }
  const VolumeArray* outsideVolumeArray() const{
   return m_outsideVolumeArray.get();
  }
  void setOutsideVolumeArray(std::shared_ptr<const VolumeArray> volArray){
    m_outsideVolumeArray = std::move(volArray);
  }

 protected:
  //Not owning ptr to the volumes
  const Tvol* m_insideVolume{};
  const Tvol* m_outsideVolume{};
  //The volume arrays can be shared when we glue volumes
  std::shared_ptr<const VolumeArray> m_insideVolumeArray{};
  std::shared_ptr<const VolumeArray> m_outsideVolumeArray{};
};


template <class Tvol>
inline void BoundarySurface<Tvol>::debugInfo(MsgStream& msg) const {
  msg << "BoundarySurface debug information: " << std::endl;
  msg << "     -> pointer to insideVolume         = " << m_insideVolume
      << std::endl;
  msg << "     -> pointer to insideVolumeArray    = "
      << m_insideVolumeArray.get() << std::endl;
  msg << "     -> pointer to outsideVolume        = " << m_outsideVolume
      << std::endl;
  msg << "     -> pointer to outsideVolumeArray   = "
      << m_outsideVolumeArray.get() << endmsg;
}

}  // end of namespace Trk

#endif  // TRKVOLUMES_BOUNDARYSURFACE_H

