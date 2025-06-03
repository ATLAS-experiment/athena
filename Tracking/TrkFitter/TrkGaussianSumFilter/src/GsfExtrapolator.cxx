/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @file   GsfExtrapolator.cxx
 * @date   Tuesday 25th January 2005
 * @author Anthony Morley, Christos Anastopoulos
 * @brief  Implementation code for GsfExtrapolator class
 */

#include "TrkGaussianSumFilter/GsfExtrapolator.h"
//
#include "TrkGaussianSumFilter/IMaterialMixtureConvolution.h"
#include "TrkGaussianSumFilterUtils/GsfConstants.h"
//
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/MaterialLayer.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkSurfaces/Surface.h"
//
#include "TrkExUtils/MaterialUpdateMode.h"
//
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackStateOnSurface.h"
//
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <utility>

namespace {
constexpr bool useBoundaryMaterialUpdate(true);

// We have cases where the MultiComponentState is not owned
// so we need to set just the cache ptr pointing to that.
// In other cases we need to add it to the cache
// and make it to point to the last element we push to the m_mcsRecycleBin
inline void
addMultiComponentToCache(Trk::IMultiStateExtrapolator::Cache& cache,
                         Trk::MultiComponentState&& input) {
  cache.m_mcsRecycleBin.emplace_back(std::move(input));
  cache.m_stateAtBoundary = &(cache.m_mcsRecycleBin.back());
}

inline void
setRecallInformation(Trk::IMultiStateExtrapolator::Cache& cache,
                     const Trk::Surface& recallSurface,
                     const Trk::Layer& recallLayer,
                     const Trk::TrackingVolume& recallTrackingVolume)
{
  cache.m_recallSurface = &recallSurface;
  cache.m_recallLayer = &recallLayer;
  cache.m_recallTrackingVolume = &recallTrackingVolume;
}

inline void
resetRecallInformation(Trk::IMultiStateExtrapolator::Cache& cache)
{
  cache.m_recallSurface = nullptr;
  cache.m_recallLayer = nullptr;
  cache.m_recallTrackingVolume = nullptr;
}

inline void
emptyRecycleBins(Trk::IMultiStateExtrapolator::Cache& cache)
{
  // Reset the boundary information
  cache.m_stateAtBoundary = nullptr;
  cache.m_navigationParameters = nullptr;
  cache.m_trackingVolume = nullptr;
  cache.m_mcsRecycleBin.clear();
}

int
radialDirection(const Trk::MultiComponentState& pars, Trk::PropDirection dir)
{
  // safe inbound/outbound estimation
  const double prePositionR = pars.begin()->params->position().perp();
  return (prePositionR > (pars.begin()->params->position() +
                          static_cast<int>(dir) * 0.5 * prePositionR *
                              pars.begin()->params->momentum().unit())
                             .perp())
             ? -1
             : 1;
}
/**
Check for radial (perpendicular) direction change,
returns true if the radial direction change is allowed
*/
inline bool
radialDirectionCheck(const EventContext& ctx,
                     const Trk::IPropagator& prop,
                     const Trk::MultiComponentState& startParm,
                     const Trk::MultiComponentState& parsOnLayer,
                     const Trk::TrackingVolume& tvol,
                     const Trk::MagneticFieldProperties& fieldProperties,
                     const Trk::PropDirection dir,
                     const Trk::ParticleHypothesis particle)
{
  const Amg::Vector3D& startPosition = startParm.begin()->params->position();
  const Amg::Vector3D& onLayerPosition = parsOnLayer.begin()->params->position();

  // the 3D distance to the layer intersection
  const double distToLayer = (startPosition - onLayerPosition).mag();
  // get the innermost contained surface for crosscheck
  const auto& boundarySurfaces = tvol.boundarySurfaces();
  // only for tubes the crossing makes sense to check for validity
  if (boundarySurfaces.size() == 4) {
    // propagate to the inside surface and compare the distance:
    // it can be either the next layer from the initial point, or the inner tube
    // boundary surface
    const Trk::Surface& insideSurface =
      (boundarySurfaces[Trk::tubeInnerCover])->surfaceRepresentation();
    auto parsOnInsideSurface =
      prop.propagateParameters(ctx,
                               *(startParm.begin()->params),
                               insideSurface,
                               dir,
                               true,
                               fieldProperties,
                               particle);
    const double distToInsideSurface =
      parsOnInsideSurface
        ? (startPosition - (parsOnInsideSurface->position())).mag()
        : 10e10;
    // the intersection with the original layer is valid if it is before the
    // inside surface
    return distToLayer < distToInsideSurface;
  }
  return true;
}

} // end of anonymous namespace

/*
 * AlgTool implementations
 */
Trk::GsfExtrapolator::GsfExtrapolator(const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_fastField(false)
{
  declareInterface<IMultiStateExtrapolator>(this);
  declareProperty("MagneticFieldProperties", m_fastField);
}

Trk::GsfExtrapolator::~GsfExtrapolator() = default;

StatusCode
Trk::GsfExtrapolator::initialize()
{

  ATH_CHECK(m_propagator.retrieve());
  ATH_CHECK(m_navigator.retrieve());
  ATH_CHECK(m_materialUpdator.retrieve());

  m_fieldProperties = m_fastField
                        ? Trk::MagneticFieldProperties(Trk::FastField)
                        : Trk::MagneticFieldProperties(Trk::FullField);

  return StatusCode::SUCCESS;
}

/************************************************************/
/*
 * Implement the public extrapolate methods
 */
/************************************************************/
/*
 * Extrapolate Interface
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolate(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{
  if (multiComponentState.empty()) {
    return {};
  }
  return extrapolateImpl(ctx,
                         cache,
                         multiComponentState,
                         surface,
                         direction,
                         boundaryCheck,
                         particleHypothesis);
}

/*
 * Extrapolate Directly method. Does not use a cache
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateDirectly(
  const EventContext& ctx,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{
  if (multiComponentState.empty()) {
    return {};
  }

  const Trk::TrackingVolume* currentVolume = m_navigator->highestVolume(ctx);
  if (!currentVolume) {
    ATH_MSG_WARNING(
      "Current tracking volume could not be determined... returning {}");
    return {};
  }
  return extrapolateDirectlyImpl(ctx,
                                 multiComponentState,
                                 surface,
                                 direction,
                                 boundaryCheck,
                                 particleHypothesis);
}

/************************************************************/
/*
 * Now the implementation of all the internal methods
 * that perform the actual calculations
 */
/************************************************************/

/*
 * This is the actual extrapolation method implementation
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateImpl(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{

  // If the extrapolation is to be without material effects simply revert to the
  // extrapolateDirectly method
  if (particleHypothesis == Trk::nonInteracting) {
    return extrapolateDirectlyImpl(ctx,
                                   multiComponentState,
                                   surface,
                                   direction,
                                   boundaryCheck,
                                   particleHypothesis);
  }

  const Trk::Layer* associatedLayer = nullptr;
  const Trk::TrackingVolume* startVolume = nullptr;
  const Trk::TrackingVolume* destinationVolume = nullptr;
  std::unique_ptr<Trk::TrackParameters> referenceParameters = nullptr;

  initialiseNavigation(ctx,
                       cache,
                       multiComponentState,
                       surface,
                       associatedLayer,
                       startVolume,
                       destinationVolume,
                       referenceParameters,
                       direction);

  // Bail to direct extrapolation if the direction cannot be determined
  if (direction == Trk::anyDirection) {
    return extrapolateDirectlyImpl(ctx,
                                   multiComponentState,
                                   surface,
                                   direction,
                                   boundaryCheck,
                                   particleHypothesis);
  }

  const Trk::TrackParameters* combinedState =
    multiComponentState.begin()->params.get();

  const Trk::MultiComponentState* currentState = &multiComponentState;

  /* Define the initial distance between destination and current position.
     Destination should be determined from either
     - reference parameters (prefered if they exist) or
     - destination surface
     */

  const Amg::Vector3D globalSeparation =
    referenceParameters
      ? referenceParameters->position() - combinedState->position()
      : surface.globalReferencePoint() - combinedState->position();
  const double initialDistance = globalSeparation.mag();
  // Clean up memory from combiner. It is no longer needed
  combinedState = nullptr;

  /* There are two parts to the extrapolation:
     - Extrapolate from start point to volume boundary
     - Extrapolate from volume boundary to destination surface
     */
  /*
   * Extrapolation to destination volume boundary
   */
  bool foundFinalBoundary(true);
  int fallbackOscillationCounter(0);
  const Trk::TrackingVolume* currentVolume = startVolume;
  const Trk::TrackingVolume* previousVolume = nullptr;

  while (currentVolume && currentVolume != destinationVolume) {
    // Extrapolate to volume boundary
    extrapolateToVolumeBoundary(ctx,
                                cache,
                                *currentState,
                                associatedLayer,
                                *currentVolume,
                                direction,
                                particleHypothesis);

    // New current state is the state extrapolated to the tracking volume
    // boundary.
    currentState = cache.m_stateAtBoundary;
    // The volume that the extrapolation is about to enter into is called the
    // nextVolume
    const Trk::TrackingVolume* nextVolume = cache.m_trackingVolume;
    // Break the loop if the next tracking volume is the same as the current one
    if (!nextVolume || nextVolume == currentVolume) {
      foundFinalBoundary = false;
      break;
    }
    // Break the loop if an oscillation is detected
    if (previousVolume == nextVolume) {
      ++fallbackOscillationCounter;
    }
    if (fallbackOscillationCounter > 10) {
      foundFinalBoundary = false;
      break;
    }
    // Break the loop if the distance between the surface and the track
    // parameters has increased
    combinedState = currentState->begin()->params.get();

    auto parametersAtDestination =
      m_propagator->propagateParameters(ctx,
                                        *combinedState,
                                        surface,
                                        direction,
                                        false,
                                        m_fieldProperties,
                                        particleHypothesis);
    Amg::Vector3D newDestination;
    if (parametersAtDestination) {
      newDestination = parametersAtDestination->position();
      // delete parametersAtDestination;
    } else {
      newDestination = surface.center();
    }

    const double revisedDistance =
        (cache.m_navigationParameters->position() - newDestination).mag();

    const double distanceChange = std::abs(revisedDistance - initialDistance);

    if (revisedDistance > initialDistance && distanceChange > 0.01) {
      foundFinalBoundary = false;
      break;
    }

    combinedState = nullptr;
    // Initialise the oscillation checker
    previousVolume = currentVolume;
    // As the extrapolation is moving into the next volume, the next volume ->
    // current volume
    currentVolume = nextVolume;
    // Associated layer now needs to be reset
    // if(!entryLayerFound)
    associatedLayer = nullptr;
  } // end while loop

  // catch failures now
  if (!currentState || (currentVolume != destinationVolume)) {
    currentState = &multiComponentState;
    foundFinalBoundary = false;
  }

  if (!foundFinalBoundary) {
    Trk::MultiComponentState bailOutState =
      m_propagator->multiStatePropagate(ctx,
                                        *currentState,
                                        surface,
                                        m_fieldProperties,
                                        Trk::anyDirection,
                                        boundaryCheck,
                                        particleHypothesis);

    emptyRecycleBins(cache);
    return bailOutState;
  }

  /*
   * Extrapolation from volume boundary to surface
   */

  // extrapolate inside destination volume
  Trk::MultiComponentState destinationState =
    extrapolateInsideVolume(ctx,
                            cache,
                            *currentState,
                            surface,
                            associatedLayer,
                            *currentVolume,
                            direction,
                            boundaryCheck,
                            particleHypothesis);

  // FALLBACK POINT: Crisis if extrapolation fails here... As per extrapolation
  // to volume boundary, in emergency revert to extrapolateDirectly

  // or we failed to reach the target
  if (!destinationState.empty() &&
      &((*(destinationState.begin())).params->associatedSurface()) != &surface) {
    destinationState.clear();
  }

  if (destinationState.empty()) {
    destinationState = m_propagator->multiStatePropagate(ctx,
                                                         *currentState,
                                                         surface,
                                                         m_fieldProperties,
                                                         Trk::anyDirection,
                                                         boundaryCheck,
                                                         particleHypothesis);
  }
  emptyRecycleBins(cache);
  return destinationState;
}

/*
 * Extrapolate Directly method. Does not use a cache
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateDirectlyImpl(
  const EventContext& ctx,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{
  return m_propagator->multiStatePropagate(ctx,
                                           multiComponentState,
                                           surface,
                                           m_fieldProperties,
                                           direction,
                                           boundaryCheck,
                                           particleHypothesis);
}

/*
 *   Extrapolate to Volume Boundary!
 */
void
Trk::GsfExtrapolator::extrapolateToVolumeBoundary(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Layer* layer,
  const Trk::TrackingVolume& trackingVolume,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis) const
{

  //We 1st point to the input.
  //As we move on we might change to point to the
  //last element held by
  //cache.m_mcsRecycleBin
  cache.m_stateAtBoundary = &multiComponentState;

  const Trk::TrackParameters* combinedState =
      cache.m_stateAtBoundary->begin()->params.get();
  const Trk::Layer* associatedLayer = layer;

  if (!associatedLayer) {
    // Get entry layer but do not use it as  it should have already be hit if it
    // was desired
    associatedLayer = trackingVolume.associatedLayer(combinedState->position());
    associatedLayer = associatedLayer
                          ? associatedLayer
                          : trackingVolume.nextLayer(
                                combinedState->position(),
                                direction * combinedState->momentum().unit(),
                                associatedLayer);
  }
  // Only loop over layers if they can be found within the tracking volume
  else if (trackingVolume.confinedLayers() &&
           associatedLayer->layerMaterialProperties()) {
    Trk::MultiComponentState updatedState = m_materialUpdator->postUpdate(
        cache.m_materialEffectsCaches,
        *(cache.m_stateAtBoundary),
        *layer,
        direction,
        particleHypothesis);

    if (!updatedState.empty()) {
      addMultiComponentToCache(cache,std::move(updatedState));
    }
  }

  // If an associated surface can be found, extrapolation within the tracking
  // volume is mandatory This will take extrapolate to the last layer in the
  // volume
  if (associatedLayer) {
    Trk::MultiComponentState nextState = extrapolateFromLayerToLayer(
        ctx,
        cache,
        *(cache.m_stateAtBoundary),
        trackingVolume,
        associatedLayer,
        nullptr,
        direction,
        particleHypothesis);
    // if we have a next State update the currentState
    if (!nextState.empty()) {
      addMultiComponentToCache(cache,std::move(nextState));
    }
  }

  /* =============================================
     Find the boundary surface using the navigator
     ============================================= */

  Trk::NavigationCell nextNavigationCell(nullptr, nullptr);

  combinedState = cache.m_stateAtBoundary->begin()->params.get();

  const Trk::TrackingVolume* nextVolume = nullptr;
  const Trk::TrackParameters* navigationParameters =
    cache.m_navigationParameters
      ? cache.m_navigationParameters.get()
      : combinedState;

  nextNavigationCell = m_navigator->nextTrackingVolume(
    ctx, *m_propagator, *navigationParameters, direction, trackingVolume);

  nextVolume = nextNavigationCell.nextVolume;

  std::unique_ptr<Trk::TrackParameters> nextNavigationParameters =
      std::move(nextNavigationCell.parametersOnBoundary);

  if (!nextVolume) {
    // Reset the layer recall
    resetRecallInformation(cache);
  }

  if (useBoundaryMaterialUpdate) {
    // Check for two things:
    // 1. If the next volume was found
    // 2. If there is material associated with the boundary layer.
    // If so, apply material effects update.

    // Get layer associated with boundary surface.
    const Trk::MaterialLayer * layerAtBoundary =
      (nextNavigationCell.parametersOnBoundary)
        ? (nextNavigationCell.parametersOnBoundary->associatedSurface())
            .materialLayer()
        : nullptr;
    Trk::MultiComponentState matUpdatedState{};
    if (nextVolume && layerAtBoundary) {
      if (layerAtBoundary->layerMaterialProperties()) {
        matUpdatedState = m_materialUpdator->postUpdate(
            cache.m_materialEffectsCaches,
            *(cache.m_stateAtBoundary),
            *layerAtBoundary,
            direction,
            particleHypothesis);
      }
    }

    // If state has changed due to boundary material, modify state, parameters
    // accordingly.
    if (!matUpdatedState.empty()) {
      addMultiComponentToCache(cache, std::move(matUpdatedState));
      nextNavigationParameters =
          cache.m_stateAtBoundary->begin()->params->uniqueClone();
    }
  }
  // Update the rest of the boundary information in the cache
  cache.m_navigationParameters = std::move(nextNavigationParameters);
  cache.m_trackingVolume = nextVolume;
}

/*
 * Extrapolate inside volume to destination surface!
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateInsideVolume(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  const Trk::Layer* layer,
  const Trk::TrackingVolume& trackingVolume,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{
  //curent state is a plainn ptr to keep track
  const Trk::MultiComponentState* currentState = &multiComponentState;

  // Retrieve the destination layer
  // 1. Association
  const Trk::Layer* destinationLayer = surface.associatedLayer();

  // 2. Recall and Global Search
  if (!destinationLayer) {
    destinationLayer =
      (&surface == cache.m_recallSurface)
        ? cache.m_recallLayer
        : trackingVolume.associatedLayer(surface.globalReferencePoint());
  }

  // Retrieve the current layer
  // Produce a combined state
  const Trk::TrackParameters* combinedState =
    currentState->begin()->params.get();

  const Trk::Layer* associatedLayer = layer;

  Trk::MultiComponentState updatedState{};
  if (!associatedLayer) {
    // Get entry layer but do not use it as  it should have already be hit if it
    // was desired
    associatedLayer = trackingVolume.associatedLayer(combinedState->position());
    associatedLayer =
      associatedLayer
        ? associatedLayer
        : trackingVolume.nextLayer(combinedState->position(),
                                   direction * combinedState->momentum().unit(),
                                   associatedLayer);
  }

  else if (associatedLayer != destinationLayer &&
           trackingVolume.confinedLayers() &&
           associatedLayer->layerMaterialProperties()) {

    updatedState = m_materialUpdator->postUpdate(cache.m_materialEffectsCaches,
                                                 *currentState,
                                                 *associatedLayer,
                                                 direction,
                                                 particleHypothesis);

    if (!updatedState.empty()) {
      // Refresh the current state pointer
      currentState = &updatedState;
    }
  }

  // Reset combined state target
  combinedState = nullptr;
  Trk::MultiComponentState nextState{};
  if (destinationLayer) {
    // If there are intermediate layers then additional extrapolations need to
    // be done
    if (associatedLayer && associatedLayer != destinationLayer) {
      nextState = extrapolateFromLayerToLayer(ctx,
                                              cache,
                                              *currentState,
                                              trackingVolume,
                                              associatedLayer,
                                              destinationLayer,
                                              direction,
                                              particleHypothesis);

      // currentState is now the next
      if (!nextState.empty()) {
        // Refresh the current state pointer
        currentState = &nextState;
      }
    }
    // Final extrapolation to destination surface
    Trk::MultiComponentState returnState =
      extrapolateToDestinationLayer(ctx,
                                    cache,
                                    *currentState,
                                    surface,
                                    *destinationLayer,
                                    associatedLayer,
                                    direction,
                                    boundaryCheck,
                                    particleHypothesis);
    // Set the information for the current layer, surface, tracking volume
    setRecallInformation(cache, surface, *destinationLayer, trackingVolume);
    return returnState;
  }

  // FALLBACK POINT: If no destination layer is found fall-back and extrapolate
  // directly
  Trk::MultiComponentState returnState =
    m_propagator->multiStatePropagate(ctx,
                                      *currentState,
                                      surface,
                                      m_fieldProperties,
                                      direction,
                                      boundaryCheck,
                                      particleHypothesis);

  // No destination layer exists so layer recall method cannot be used and
  // should be reset
  resetRecallInformation(cache);

  return returnState;
}

/*
 * Extrapolate from Layer to Layer
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateFromLayerToLayer(
  const EventContext& ctx,
  Cache& cache,
  const MultiComponentState& multiComponentState,
  const TrackingVolume& trackingVolume,
  const Layer* startLayer,
  const Layer* destinationLayer,
  PropDirection direction,
  ParticleHypothesis particleHypothesis) const
{

  const Trk::Layer* currentLayer = startLayer;
  Trk::MultiComponentState currentState{};

  const Trk::TrackParameters* combinedState =
    multiComponentState.begin()->params.get();
  Amg::Vector3D currentPosition = combinedState->position();
  Amg::Vector3D currentDirection = direction * combinedState->momentum().unit();

  // No need to extrapolate to start layer, find the next one
  const Trk::Layer* nextLayer =
    currentLayer->nextLayer(currentPosition, currentDirection);

  using LayerSet = boost::container::flat_set<
    const Trk::Layer*,
    std::less<const Trk::Layer*>,
    boost::container::small_vector<const Trk::Layer*, 8>>;
  LayerSet layersHit;

  layersHit.insert(currentLayer);

  // Begin while loop over all intermediate layers
  while (nextLayer && nextLayer != destinationLayer) {
    layersHit.insert(nextLayer);
    // Only extrapolate to an intermediate layer if it requires material
    // update. Otherwise step over it
    if (nextLayer->layerMaterialProperties()) {
      currentState = extrapolateToIntermediateLayer(
          ctx,
          cache,
          !currentState.empty() ? currentState : multiComponentState,
          *nextLayer,
          trackingVolume,
          direction,
          particleHypothesis);
    }

    if (!currentState.empty()) {
      combinedState = currentState.begin()->params.get();
      currentPosition = combinedState->position();
      currentDirection = direction * combinedState->momentum().unit();
    }

    // Find the next layer
    currentLayer = nextLayer;
    nextLayer = currentLayer->nextLayer(currentPosition, currentDirection);
    if (layersHit.find(nextLayer) != layersHit.end()) {
      break;
    }
  }

  if (destinationLayer && nextLayer != destinationLayer &&
      !currentState.empty()) {
    currentState.clear();
  }

  return currentState;
}

/*
 * Extrapolate to Intermediate Layer
 */
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateToIntermediateLayer(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Layer& layer,
  const Trk::TrackingVolume& trackingVolume,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis,
  bool doPerpCheck) const
{
  const Trk::MultiComponentState* initialState = &multiComponentState;

  // Propagate over all components
  Trk::MultiComponentState destinationState =
    m_propagator->multiStatePropagate(ctx,
                                      *initialState,
                                      layer.surfaceRepresentation(),
                                      m_fieldProperties,
                                      direction,
                                      true,
                                      particleHypothesis);

  if (destinationState.empty()) {
    return {};
  }

  // the layer has been intersected
  // ------------------------------------------------------------------------
  // check for radial direction change
  // ---------------------------------------------------------------------
  const int rDirection = radialDirection(multiComponentState, direction);
  const int newrDirection = radialDirection(destinationState, direction);
  if (newrDirection != rDirection && doPerpCheck) {
    // it is unfortunate that the cancelling could invalidate the material
    // collection
    // reset the nextParameters if the radial change is not allowed
    //  resetting is ok - since the parameters are in the garbage bin already
    if (!radialDirectionCheck(ctx,
                              *m_propagator,
                              multiComponentState,
                              destinationState,
                              trackingVolume,
                              m_fieldProperties,
                              direction,
                              particleHypothesis)) {
      return {};
    }
  }

  /* -------------------------------------
     Material effects
     ------------------------------------- */

  Trk::MultiComponentState updatedState =
    m_materialUpdator->update(cache.m_materialEffectsCaches,
                              destinationState,
                              layer,
                              direction,
                              particleHypothesis);

  if (updatedState.empty()) {
    return destinationState;
  }

  return updatedState;
}

/*
   Extrapolate to Destination Layer
*/
Trk::MultiComponentState
Trk::GsfExtrapolator::extrapolateToDestinationLayer(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  const Trk::Layer& layer,
  const Trk::Layer* startLayer,
  Trk::PropDirection direction,
  const Trk::BoundaryCheck& boundaryCheck,
  Trk::ParticleHypothesis particleHypothesis) const
{

  const Trk::MultiComponentState* initialState = &multiComponentState;
  const Trk::TrackParameters* combinedState = nullptr;

  // Propagate over all components
  Trk::MultiComponentState destinationState =
    m_propagator->multiStatePropagate(ctx,
                                      multiComponentState,
                                      surface,
                                      m_fieldProperties,
                                      direction,
                                      boundaryCheck,
                                      particleHypothesis);

  // Require a fall-back if the initial state is close to the destination
  // surface then a fall-back solution is required

  if (destinationState.empty()) {
    combinedState = initialState->begin()->params.get();
    if (surface.isOnSurface(
          combinedState->position(), true, 0.5 * layer.thickness())) {
      destinationState = m_propagator->multiStatePropagate(ctx,
                                                           *initialState,
                                                           surface,
                                                           m_fieldProperties,
                                                           Trk::anyDirection,
                                                           boundaryCheck,
                                                           particleHypothesis);
    }
    combinedState = nullptr;
    if (destinationState.empty()) {
      return {};
    }
  }

  /* ----------------------------------------
     Material effects
     ---------------------------------------- */

  Trk::MultiComponentState updatedState{};
  if (startLayer != &layer) {
    updatedState = m_materialUpdator->preUpdate(cache.m_materialEffectsCaches,
                                                destinationState,
                                                layer,
                                                direction,
                                                particleHypothesis);
  }

  if (updatedState.empty()) {
    return destinationState;
  }

  return updatedState;
}

/*
 * Initialise Navigation
 */
void
Trk::GsfExtrapolator::initialiseNavigation(
  const EventContext& ctx,
  Cache& cache,
  const Trk::MultiComponentState& multiComponentState,
  const Trk::Surface& surface,
  const Trk::Layer*& currentLayer,
  const Trk::TrackingVolume*& currentVolume,
  const Trk::TrackingVolume*& destinationVolume,
  std::unique_ptr<Trk::TrackParameters>& referenceParameters,
  Trk::PropDirection direction) const
{

  // Empty the garbage bin
  emptyRecycleBins(cache);
  const Trk::TrackParameters* combinedState =
    multiComponentState.begin()->params.get();
  /* =============================================
     Look for current volume
     ============================================= */
  // 1. See if the current layer is associated with a tracking volume

  const Trk::Surface* associatedSurface = &(combinedState->associatedSurface());
  currentLayer =
    associatedSurface ? associatedSurface->associatedLayer() : currentLayer;
  currentVolume =
    currentLayer ? currentLayer->enclosingTrackingVolume() : currentVolume;

  // If the association method failed then try the recall method

  if (!currentVolume && associatedSurface == cache.m_recallSurface) {
    currentVolume = cache.m_recallTrackingVolume;
    currentLayer = cache.m_recallLayer;
  }
  // Global search method if this fails

  else if (!currentVolume) {
    // If the recall method fails then the cashed information needs to be reset
    resetRecallInformation(cache);
    currentVolume = m_navigator->volume(ctx, combinedState->position());
    currentLayer = (currentVolume)
                     ? currentVolume->associatedLayer(combinedState->position())
                     : nullptr;
  }
  /* =============================================
     Determine the resolved direction
     ============================================= */
  if (direction == Trk::anyDirection) {
    referenceParameters =
      currentVolume
        ? m_propagator->propagateParameters(
            ctx, *combinedState, surface, direction, false, m_fieldProperties)
        : nullptr;
    // These parameters will need to be deleted later. Add to list of garbage to
    // be collected
    if (referenceParameters) {
      const Amg::Vector3D surfaceDirection(referenceParameters->position() -
                                     combinedState->position());
      direction = (surfaceDirection.dot(combinedState->momentum()) > 0.)
                    ? Trk::alongMomentum
                    : Trk::oppositeMomentum;
    }
  }

  /* =============================================
     Look for destination volume
     ============================================= */

  // 1. See if the destination layer is associated with a tracking volume
  destinationVolume = surface.associatedLayer()
                        ? surface.associatedLayer()->enclosingTrackingVolume()
                        : nullptr;

  // 2. See if there is a cashed recall surface
  if (!destinationVolume && &surface == cache.m_recallSurface) {
    destinationVolume = cache.m_recallTrackingVolume;
    // If no reference parameters are defined, then determine them
    if (!referenceParameters) {
      referenceParameters =
        currentVolume
          ? m_propagator->propagateParameters(
              ctx, *combinedState, surface, direction, false, m_fieldProperties)
          : nullptr;
    }
    // 3. Global search
  } else {
    // If no reference parameters are defined try to determine them
    if (!referenceParameters) {
      referenceParameters =
        currentVolume
          ? m_propagator->propagateParameters(
              ctx, *combinedState, surface, direction, false, m_fieldProperties)
          : nullptr;
    }
    // Global search of tracking geometry to find the destination volume
    destinationVolume = m_navigator->volume(
        ctx, referenceParameters ? referenceParameters->position()
                                 : surface.globalReferencePoint());
  }
}

