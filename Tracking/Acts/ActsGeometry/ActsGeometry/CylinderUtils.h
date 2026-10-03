/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSEVENT_CYLINDERUTILS_H
#define ACTSEVENT_CYLINDERUTILS_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Acts/Definitions/Direction.hpp"

#include <span>

namespace Acts{
    class BoundTrackParameters;
    class GeometryContext;
} 


namespace ActsTrk {
    /** @brief Returns the two path lengths for a track state with position
     *         and direction to intersect the cylinder with radius R. 
     *         The cylinder is assumed to be around the beam line
     *         The first path length is always the closest forward solution
     * @param pos: The position of the track state
     * @param dir: The direction of the tra state
     * @param cylinderR: The radius of the cylinder to intersect */
    std::array<double, 2> cylinderIntersectPaths(const Amg::Vector3D& pos, 
                                                 const Amg::Vector3D& dir,
                                                 const double cylinderR);
    /** @brief Propagates the track parameters to a cylinder with radius R and expresses
      *        them as bound track parameters
      *  @param tgContext: The geometry context to align the bound track incoming bound track paremters
      *  @param boundPars: The bound parameters that shall be extrapolated to the cylinder 
      *  @param cylinderR: The radius of the final cylinder on which the bound track paramters are expressed
      *  @param dir: The direction of the propagation (forward or backward) */
    std::optional<Acts::BoundTrackParameters> 
        expressOnCylinder(const Acts::GeometryContext& tgContext,
                           const Acts::BoundTrackParameters& boundPars,
                           const double cylinderR,
                           const Acts::Direction dir = Acts::Direction::Forward());
    /** @brief Propgate the track parameters to a set of cylinders. Only valid propagations are retained
      *        and sorted by ascending radius
      *  @param tgContext: The geometry context to align the bound track incoming bound track paremters
      *  @param boundPars: The bound parameters that shall be extrapolated to the cylinder 
      *  @param cylinderR: List of radii at which the parameters shall be expressed
      *  @param dir: The direction of the propagation (forward or backward) */
    std::vector<Acts::BoundTrackParameters> 
        expressOnCylinders(const Acts::GeometryContext& tgContext,
                            const Acts::BoundTrackParameters& boundPars,
                            std::span<const double> cylinderRadii,
                            const Acts::Direction dir = Acts::Direction::Forward());
    
}


#endif