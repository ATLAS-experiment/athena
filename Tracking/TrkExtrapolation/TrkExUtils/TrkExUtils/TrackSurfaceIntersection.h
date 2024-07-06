/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSurfaceIntersection.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXUTILS_TRACKSURFACEINTERSECTION_H
#define TRKEXUTILS_TRACKSURFACEINTERSECTION_H

// Trk
#include <memory>

#include "GeoPrimitives/GeoPrimitives.h"
class MsgStream;

namespace Trk {

/** @class TrackSurfaceIntersection

   An intersection with a Surface is given by
   - a global position
   - a momentum direction (unit vector)
   - the pathlength to go there from the starting point

   We can optionally attach an additional block of data (`cache')
   for the private use of a particular intersection tool.

    @author Andreas.Salzburger@cern.ch
  */
class TrackSurfaceIntersection final {

 public:
  /**Base class for cache block.*/
  class IIntersectionCache {
   public:
    virtual ~IIntersectionCache() = default;
    virtual std::unique_ptr<IIntersectionCache> clone() const = 0;
  };

  /**Constructor*/
  TrackSurfaceIntersection(const Amg::Vector3D& pos, const Amg::Vector3D& dir,
                           double path);
  TrackSurfaceIntersection() = default;
  /**Destructor*/
  ~TrackSurfaceIntersection() = default;

  TrackSurfaceIntersection(const TrackSurfaceIntersection& other);
  TrackSurfaceIntersection(const TrackSurfaceIntersection& other,
                           std::unique_ptr<IIntersectionCache> cache);
  TrackSurfaceIntersection& operator=(const TrackSurfaceIntersection& other);

  TrackSurfaceIntersection(TrackSurfaceIntersection&& other) = default;
  TrackSurfaceIntersection& operator=(TrackSurfaceIntersection&& other) = default;

  /** Method to retrieve the position of the Intersection */
  const Amg::Vector3D& position() const;
  Amg::Vector3D& position();

  /** Method to retrieve the direction at the Intersection */
  const Amg::Vector3D& direction() const;
  Amg::Vector3D& direction();

  /** Method to retrieve the pathlength propagated till the Intersection */
  double pathlength() const;
  double& pathlength();

  /** Retrieve the associated cache block, if it exists. */
  const IIntersectionCache* cache() const;
  IIntersectionCache* cache();

 private:
  Amg::Vector3D m_position;
  Amg::Vector3D m_direction;
  double m_pathlength{0};
  std::unique_ptr<IIntersectionCache> m_cache;
};

inline const Amg::Vector3D& TrackSurfaceIntersection::position() const {
  return m_position;
}

inline Amg::Vector3D& TrackSurfaceIntersection::position() {
  return m_position;
}

inline const Amg::Vector3D& TrackSurfaceIntersection::direction() const {
  return m_direction;
}

inline Amg::Vector3D& TrackSurfaceIntersection::direction() {
  return m_direction;
}

inline double TrackSurfaceIntersection::pathlength() const {
  return m_pathlength;
}

inline double& TrackSurfaceIntersection::pathlength() {
  return m_pathlength;
}

inline const TrackSurfaceIntersection::IIntersectionCache*
TrackSurfaceIntersection::cache() const {
  return m_cache.get();
}

inline TrackSurfaceIntersection::IIntersectionCache*
TrackSurfaceIntersection::cache() {
  return m_cache.get();
}


/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream& operator<<(MsgStream& sl, const TrackSurfaceIntersection& tsfi);
std::ostream& operator<<(std::ostream& sl,
                         const TrackSurfaceIntersection& tsfi);

}  // namespace Trk

#endif  // TRKEXUTILS_TRACKSURFACEINTERSECTION_H
