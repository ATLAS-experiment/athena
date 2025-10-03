/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   IMultiStateExtrapolator.h
 * @date   Tuesday 25th January 2005
 * @author Anthony Morley, Christos Anastopoulos
 * Abstract base class for extrapolation of a MultiComponentState
 */

#ifndef TrkIMultiStateExtrapolator_H
#define TrkIMultiStateExtrapolator_H

#include "TrkGaussianSumFilterUtils/GsfMaterial.h"
#include "TrkParameters/ComponentParameters.h"
//
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/BoundaryCheck.h"
//
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
//
#include <memory>
#include <vector>

namespace Trk {

class Surface;
class Track;
class TrackingVolume;
class TrackStateOnSurface;

static const InterfaceID IID_IMultiStateExtrapolator("IMultiStateExtrapolator",
                                                     1, 0);

class IMultiStateExtrapolator : virtual public IAlgTool
{
public:
  /** @brief MultiStateExtrapolator cache class.
   *
   * This object holds information regarding the internal state of the
   * extrpolation process as well as a large store for  material effects
   * properties.
   *
   * It to be passed as argument to extrapolation calls
   */
  struct Cache
  {
    //!< Surface for recall   (not owning)
    const Surface* m_recallSurface = nullptr;
    //!< Layer for recall   (not owning)
    const Layer* m_recallLayer = nullptr;
    //!< Tracking volume for recall (not owning)
    const TrackingVolume* m_recallTrackingVolume = nullptr;
    // Current State at boundary (not owning)
    const MultiComponentState* m_stateAtBoundary = nullptr;
    // Current Navigation parameters
    std::unique_ptr<TrackParameters> m_navigationParameters = nullptr;
    // Ptr to current volume (not owning)
    const TrackingVolume* m_trackingVolume = nullptr;
    //!< keep track of the MultiComponentStates
    std::vector<MultiComponentState> m_mcsRecycleBin;
    // Vector of combined material effects
    std::vector<GsfMaterial::Combined> m_materialEffectsCaches;
    Cache() { m_materialEffectsCaches.reserve(12); }
  };

  /** Virtual destructor */
  virtual ~IMultiStateExtrapolator() = default;

  /** AlgTool interface method */
  static const InterfaceID& interfaceID()
  {
    return IID_IMultiStateExtrapolator;
  };

  /** Configured AlgTool extrapolation method (1) */
  virtual MultiComponentState extrapolate(
    const EventContext& ctx,
    Cache&,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const = 0;

  /** Configured AlgTool extrapolation without material effects method (2) */
  virtual MultiComponentState extrapolateDirectly(
    const EventContext& ctx,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const = 0;

  //!< The particle hypothesis used.
  virtual Trk::ParticleHypothesis particleHypothesis() const = 0;


};

} // end trk namespace

#endif
