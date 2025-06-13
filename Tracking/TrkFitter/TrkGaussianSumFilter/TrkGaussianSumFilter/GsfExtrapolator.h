/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @file   GsfExtrapolator.h
 * @date   Tuesday 25th January 2005
 * @author Tom Athkinson, Anthony Morley, Christos Anastopoulos
 * Extrapolation of a Multi Component State
 *
 * - The Runge Kutta Propagator is used as IPropagator
 *   for the transport of parameters (no material effects)
 * - The actual material effects, following the GSF formalism,
 *   are added via an IMaterialMixtureConvolution instance
 * - We also need the Navigator for Tracking
 *   Geometry navigation.
 */

#ifndef TrkGsfExtrapolator_H
#define TrkGsfExtrapolator_H

#include "TrkGaussianSumFilter/IMultiStateExtrapolator.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Accumulators.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrkExInterfaces/INavigator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkGaussianSumFilter/IMaterialMixtureConvolution.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkParameters/TrackParameters.h"

#include <memory>
#include <string>
#include <vector>

namespace Trk {

class Layer;
class Surface;
class TrackingVolume;
class TrackingGeometry;
class TrackStateOnSurface;
class MaterialProperties;
/** @class GsfExtrapolator */
class GsfExtrapolator final
  : public AthAlgTool
  , virtual public IMultiStateExtrapolator
{

public:
  /** Constructor with AlgTool parameters */
  GsfExtrapolator(const std::string&, const std::string&, const IInterface*);
  virtual ~GsfExtrapolator() override final;
  virtual StatusCode initialize() override final;

  /** Extrapolation method applying material affects*/
  virtual MultiComponentState extrapolate(
    const EventContext& ctx,
    Cache&,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const override final;

  /** Extrapolation method without material effects */
  virtual MultiComponentState extrapolateDirectly(
    const EventContext& ctx,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const override final;

  //!< The particle hypothesis used.
  virtual Trk::ParticleHypothesis particleHypothesis() const override final{
    return m_materialUpdator->particleHypothesis();
  }


private:
  /** Implementation of main extrapolation method*/
  MultiComponentState extrapolateImpl(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const;

  /** Implementation of extrapolation without material effects*/
  MultiComponentState extrapolateDirectlyImpl(
    const EventContext& ctx,
    const MultiComponentState&,
    const Surface&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const;

  /** Two primary private extrapolation methods
    - extrapolateToVolumeBoundary : extrapolates to the
    exit of the destination tracking volume
    The exit layer surface will be hit in this method.

    - extrapolateInsideVolume : extrapolates to the destination surface in
    the final tracking volume
    */
  void extrapolateToVolumeBoundary(const EventContext& ctx,
                                   Cache& cache,
                                   const MultiComponentState&,
                                   const Layer*,
                                   const TrackingVolume&,
                                   PropDirection direction) const;

  MultiComponentState extrapolateInsideVolume(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState&,
    const Surface&,
    const Layer*,
    const TrackingVolume&,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const;

  /** Layer stepping, stopping at the last layer before destination */
  MultiComponentState extrapolateFromLayerToLayer(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState&,
    const TrackingVolume&,
    const Layer* startLayer,
    const Layer* destinationLayer,
    PropDirection direction) const;

  /** Single extrapolation step to an intermediate layer */
  MultiComponentState extrapolateToIntermediateLayer(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState&,
    const Layer&,
    const TrackingVolume&,
    PropDirection direction) const;

  /** Final extrapolation step to a destination layer */
  MultiComponentState extrapolateToDestinationLayer(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState&,
    const Surface&,
    const Layer&,
    const Layer*,
    PropDirection direction,
    const BoundaryCheck& boundaryCheck) const;

  /** Method to initialise navigation parameters including starting state, layer
   * and volume, and destination volume */
  std::unique_ptr<Trk::TrackParameters> initialiseNavigation(
    const EventContext& ctx,
    Cache& cache,
    const MultiComponentState& initialState,
    const Surface& surface,
    const Layer*& associatedLayer,
    const TrackingVolume*& currentVolume,
    const TrackingVolume*& destinationVolume,
    PropDirection& direction) const;

  ToolHandle<IPropagator> m_propagator{this, "Propagator", "", ""};
  ToolHandle<INavigator> m_navigator{this, "Navigator",
                                     "Trk::Navigator/Navigator", ""};
  ToolHandle<IMaterialMixtureConvolution> m_materialUpdator{
      this, "GsfMaterialConvolution", "", "Gsf Material effects"};
  BooleanProperty m_fastField{this, "UseFastField", false};

  Trk::MagneticFieldProperties m_fieldProperties = Trk::FullField;
};

} // end namespace Trk

#endif
