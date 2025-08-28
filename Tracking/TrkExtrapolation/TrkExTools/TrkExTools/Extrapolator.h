/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

///////////////////////////////////////////////////////////////////
// Extrapolator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_EXTRAPOLATOR_H
#define TRKEXTOOLS_EXTRAPOLATOR_H

#include "LocalExtrapolatorCache.h" //for Trk::Cache
#include "ExtrUniquePtrHolder.h"

#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExInterfaces/IMaterialEffectsUpdator.h" // in tool handle array
#include "TrkExInterfaces/INavigator.h"              //in tool handle
#include "TrkExInterfaces/IPropagator.h"             //in tool handle
// Trk
#include "TrkEventPrimitives/ParticleHypothesis.h"  //enum
#include "TrkEventPrimitives/PropDirection.h"       //enum
#include "TrkGeometry/MagneticFieldProperties.h"    //member
#include "TrkNeutralParameters/NeutralParameters.h" //typedef
#include "TrkParameters/TrackParameters.h" //template parameter in typedef
#include "TrkSurfaces/BoundaryCheck.h"     //template parameter in typedef
// Amg
#include "EventPrimitives/EventPrimitives.h" //Amg::Vector etc
// xAOD
#include "xAODTracking/NeutralParticle.h" //typedef
#include "xAODTracking/TrackParticle.h"   //typedef
// Gaudi/StoreGate
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthCheckedComponent.h"
#include "GaudiKernel/ToolHandle.h"
#include <Gaudi/Accumulators.h>
// STL
#include <cstring>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class MsgStream;
class EventContext;

namespace Trk {
class Track;
class Surface;
class Layer;
class Volume;
class DetachedTrackingVolume;
class TrackingGeometry;
class TrackParticleBase;
class IDynamicLayerCreator;
class IMultipleScatteringUpdator;
class IEnergyLossUpdator;
class AlignableTrackingVolume;
class ExtrapolationCache;
class TrackingVolume;


using TrackParametersUVector = std::vector<std::unique_ptr<Trk::TrackParameters>>;
using DestSurf = std::pair<const Surface*, BoundaryCheck>;

/**
@class Extrapolator

@brief  Extrapolation of track parameters and their
associated covariances to destination surfaces

It combines
- The mathematical propagation of track
parameters to a destination surface via the configured propagators
RungeKuttaPropagator or STEP_propagator.

- The possible navigation procedure determining the starting
and destination volume for the extrapolation that provides
the boundary surfaces to be be intersected.

- The application of material effects either in certain
points/surfaces via the relevant MaterialEffectsUpdators,
EnergyLoss and MultipleScattering tools,
or continuously via the STEP_propagator for dense volumes.

There are always one  Navigator
and one STEP_Propagator (forced for muon like workload inside dense volumes)

There are also arrays of Propagators, MaterialEffects updators
and possible of EnergyLoss and MultipleScattering Updators.
These must have at least one entry [Global] .
The default / Global propagator is a RungeKuttaPropator
for most typical cases.

Multiple methods are provided for tasks
of varying complexity.

As an example:
In one side there
is ExtrapolateDirectly which is a thinwrapper
over the [Global] Propagator call (can be replaced completely by a call
to it in client code).
In the other side there is an extrapolate overload
that allows to find and collect all intersections
for a particle (typically muon) traversing the
ATLAS calorimeters.

@author Andreas.Salzburger@cern.ch
@authors (Athena MT) G. Gaycken, S. Roe , C. Anastopoulos
*/

class Extrapolator final
  : public AthCheckedComponent<AthAlgTool>
  , virtual public IExtrapolator
{
public:

  /**Constructor */
  Extrapolator(const std::string&, const std::string&, const IInterface*);
  /**Destructor*/
  ~Extrapolator();

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;
  /** AlgTool finalize method */
  virtual StatusCode finalize() override;

  /** Extrapolate directly: Forwards directly the call to the
   * configured "Global" propagator. No navigation and no
   * material effecs. Useful when we need fast propagation
   * without these. */
  virtual std::unique_ptr<TrackParameters> extrapolateDirectly(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion) const override final;

  /** Main extrapolation Interface starting from neutral parameters
   * and aiming at surface.*/
  virtual std::unique_ptr<NeutralParameters> extrapolate(
    const NeutralParameters& parameters,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true) const override final;

  /** Main extrapolation interface starting from charged parameters and aiming
   * at Surface*/
  virtual std::unique_ptr<TrackParameters> extrapolate(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise,
    Trk::ExtrapolationCache* cache = nullptr) const override final;

  /** Extrapolation method where a step-wise navigation to the
    destination surface is performed. Returns a vector
    parameters on all detector elements. Used mainly by the
    hole-search*/
  virtual TrackParametersUVector extrapolateStepwise(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion) const override final;

  /** Main extrapolation interface starting from a Trk::Track and aiming
   * at Surface. It uses the navigator to find the closest parameters
   * of the track to the surface. */
  virtual std::unique_ptr<TrackParameters> extrapolateTrack(
    const EventContext& ctx,
    const Track& trk,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise,
    Trk::ExtrapolationCache* cache = nullptr) const override final;

  /** extrapolateBlindly like step-wise extrapolation, but without
   * a destination surface.  Blind inside the given tracking Volume (boundaryVol).
   * If none is give stops at last boundary surface of the known
   * TrackingGeometry
   */
  virtual TrackParametersUVector extrapolateBlindly(
    const EventContext& ctx,
    const TrackParameters& parm,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    const Volume* boundaryVol = nullptr) const override final;

  /** Extrapolate to a destination surface,
   * while collecting all the material
   * layers in between. Useful for chi2 based tracking.
   */
  virtual std::vector<const TrackStateOnSurface*>* extrapolateM(
    const EventContext& ctx,
    const TrackParameters& parameters,
    const Surface& sf,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    ParticleHypothesis particle = pion,
    Trk::ExtrapolationCache* cache = nullptr) const override final;

  /** Extrapolation method collecting intersections with subdetector
   * boundaries and active volumes/layers. Destination
   * (subdetector boundary) : geoID (+ entry, -exit)
   * ( default Calo = 3 exit see GeometrySignature.h)
   * Employs the STEP_propagator, used to create ParticleCaloExtensions
   * mainly for muons and Particle Flow.
   */
  virtual std::unique_ptr<std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>>>
  collectIntersections(
    const EventContext& ctx,
    const Trk::TrackParameters& parm,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    int destination = 3) const override final;

  /** Extrapolation to the next active layer with material collection*/
  virtual std::pair<std::unique_ptr<TrackParameters>, const Layer*> extrapolateToNextActiveLayerM(
    const EventContext& ctx,
    const TrackParameters& parm,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    std::vector<const Trk::TrackStateOnSurface*>& material,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const override final;

  /** Extrapolation to volume :*/
  virtual std::unique_ptr<TrackParameters> extrapolateToVolume(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Trk::TrackingVolume& vol,
    PropDirection dir = anyDirection,
    ParticleHypothesis particle = pion) const override final;

  /** Return the TrackingGeometry used by the Extrapolator (forward information from Navigator)*/
  virtual const TrackingGeometry* trackingGeometry() const override final;

private:

  typedef std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>> identifiedParameters_t;
  /**
   * Actual heavy lifting implementation for  extrapolate
   */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateImpl(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** * Actual heavy lifting implementation for extrapolateStepwise
  - returns a vector of TrackParameters representing the tracking detector
  elements hit in between and the TrackParameters at the destination Surface (if
  final extrapolation suceeds), empty if the extrapolation to the destination
  surface does not suceed*/
  TrackParametersUVector extrapolateStepwiseImpl(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion) const;

  /**
   * Actual heavy lifting implementation for extrapolate
   * - Extrapolation using specific intermediate surfaces and energy loss
   * effects to be accounted for at each surface as specified by the
   * corresponding MaterialEffectsOnTrack -Final boolean only relevant if
   * LandauMode = true for the configured MaterialEffectsUpdator
   */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateImpl(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const std::vector<MaterialEffectsOnTrack>& sfMeff,
    const TrackingVolume& tvol,
    PropDirection dir,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Actual heavy lifting implementation for extrapolate
  */
  virtual Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateImpl(
    const EventContext& ctx,
    Cache& cache,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise,
    Trk::ExtrapolationCache* extrapolationCache = nullptr) const;

  /** Actual heavy lifting implementation for extrapolateDirectly
   */
  std::unique_ptr<TrackParameters> extrapolateDirectlyImpl(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion) const;

  /** Actual heavy lifting implementation for extrapolateBlindly
   */
  Trk::TrackParametersUVector extrapolateBlindlyImpl(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    const Volume* boundaryVol = nullptr) const;

  /** Actual heavy lifting implementation for extrapolateToNextActiveLayer
   */
  std::pair<std::unique_ptr<TrackParameters>, const Layer*>
  extrapolateToNextActiveLayerImpl(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parm,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Actual heavy lifting implementation for extrapolateToNextActiveLayerM
   */
  std::pair<std::unique_ptr<TrackParameters>, const Layer*>
  extrapolateToNextActiveLayerMImpl(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parm,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    std::vector<const Trk::TrackStateOnSurface*>& material,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Actual heavy lifting implementation for extrapolateToVolume
   */
  std::unique_ptr<TrackParameters> extrapolateToVolumeImpl(
    const EventContext& ctx,
    const IPropagator& prop,
    const TrackParameters& parm,
    const Trk::TrackingVolume& vol,
    PropDirection dir = anyDirection,
    ParticleHypothesis particle = pion) const;

  /** Private method for extrapolation in final volume to destination surface
    - Parameters are: IPropagator& prop            ... propagator to be used
    TrackParameters& parm        ... starting parameters
    Surface& sf                  ... destination surface
    TrackingVolume&              ... the initial volume
    Layer* associatedLayer       ... layer associatiated with starting parameters (steers
    postupdate) PropDirection dir            ... propagation direction const BoundaryCheck&  bcheck
    ... boolean for bounday check ParticleHypothesis  particle ... the particle hypothesis
    std::vector<const TrackParameters*>* dethits ... for blind extrapolation

    it will call:
    - A) insideVolumeStaticLayers() for a TrackingVolume with static layers
    - C) insideVolumeDetachedVolumes() for a TrackingVolume with detached inner Volumes
    */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateInsideVolume(const EventContext& ctx,
                                              Cache& cache,
                                              const IPropagator& prop,
                                              Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
                                              const Surface& sf,
                                              const Layer* associatedLayer,
                                              const TrackingVolume& tvol,
                                              PropDirection dir = anyDirection,
                                              const BoundaryCheck& bcheck = true,
                                              ParticleHypothesis particle = pion,
                                              MaterialUpdateMode matupmode = addNoise) const;

  /** A) call from extrapolateInsideVolume or toBoundary,
    if it is to boundary, the return parameters are the parameters at the boundary
    */
  Trk::CacheOwnedPtr<Trk::TrackParameters> insideVolumeStaticLayers(const EventContext& ctx,
                                               Cache& cache,
                                               bool toBoundary,
                                               const IPropagator& prop,
                                               Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
                                               const Layer* associatedLayer,
                                               const TrackingVolume& tvol,
                                               PropDirection dir = anyDirection,
                                               const BoundaryCheck& bcheck = true,
                                               ParticleHypothesis particle = pion,
                                               MaterialUpdateMode matupmode = addNoise) const;

  /** C) call from extrapolateInsideVolume */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateWithinDetachedVolumes(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Surface& sf,
    const TrackingVolume& tvol,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateToNextMaterialLayer(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Trk::Surface* destSurf,
    const Trk::TrackingVolume* vol,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateInAlignableTV(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Trk::Surface* destSurf,
    const Trk::AlignableTrackingVolume* vol,
    PropDirection dir,
    ParticleHypothesis particle = pion) const;

  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateToVolumeWithPathLimit(
    const EventContext& ctx,
    Cache& cache,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    double pathLim,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    const Trk::TrackingVolume* destVol,
    MaterialUpdateMode matupmod = addNoise) const;

  /** Private method for extrapolation in intermediate volume to boundary surface
    - Parameters are: IPropagator& prop            ... propagator to be used
    TrackParameters& parm        ... starting parameters
    Surface& sf                  ... destination surface
    TrackingVolume&              ... the initial volume
    Layer* associatedLayer       ... layer associatiated with starting parameters (steers
    postupdate) PropDirection dir            ... propagation direction const BoundaryCheck&  bcheck
    ... boolean for bounday check ParticleHypothesis  particle ... the particle hypothesis
    std::vector<const TrackParameters*>* dethits ... for blind extrapolation
    it will call:
    - A) toVolumeBoundaryStaticLayers() for a TrackingVolume with static layers
    - C) toVolumeBoundaryDetachedVolumes() for a TrackingVolume with detached inner Volumes
    */
  void extrapolateToVolumeBoundary(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Layer* associatedLayer,
    const TrackingVolume& tvol,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Private method to step from one to the last
    layer and stop at last layer (before 0) or before destination layer */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateFromLayerToLayer(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const TrackingVolume& tvol,
    const Layer* nextLayer,
    const Layer* destinationLayer,
    Trk::CacheOwnedPtr<Trk::TrackParameters> navParameters,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Private to extrapolate to the destination layer + surface
   */
  Trk::CacheOwnedPtr<Trk::TrackParameters> extrapolateToDestinationLayer(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Surface& sf,
    const Layer& lay,
    const TrackingVolume& tvol,
    const Layer* startLayer,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise) const;

  /** Private to extrapolate to the destination layer + surface, special treatment for exit layer
   * @return valid track parameters or nullptr, as first element and in case of nullptr as second
   * element true to indicate to kill the loop from material update(?)
   */
  std::pair<Trk::CacheOwnedPtr<Trk::TrackParameters>, bool> extrapolateToIntermediateLayer(
    const EventContext& ctx,
    Cache& cache,
    const IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
    const Layer& lay,
    const TrackingVolume& tvol,
    PropDirection dir = anyDirection,
    const BoundaryCheck& bcheck = true,
    ParticleHypothesis particle = pion,
    MaterialUpdateMode matupmode = addNoise,
    bool perpendicularCheck = true) const;

  /** Private to search for overlap surfaces */
  void overlapSearch(const EventContext& ctx,
                     Cache& cache,
                     const IPropagator& prop,
                     Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
                     Trk::CacheOwnedPtr<Trk::TrackParameters> parsOnLayer,
                     const Layer& lay,
                     const TrackingVolume& tvol,
                     PropDirection dir = anyDirection,
                     const BoundaryCheck& bcheck = true,
                     ParticleHypothesis particle = pion,
                     bool startingLayer = false) const;

  /** Private method for Initial Extrapolation setup
    -> overwrites the given pointers for the start and destination parameters
    -> returns a direction for the Navigation :
    - usually dir if dir @f$ \in  @f$ [ Trk::alongMomentum, Trk::oppositeMomentum ]
    - a chosen one if dir == Trk::anyDirection
    */
  PropDirection initializeNavigation(
    const EventContext& ctx,
    Cache& cache,
    const Trk::IPropagator& prop,
    Trk::CacheOwnedPtr<Trk::TrackParameters> startPars,
    const Trk::Surface& destSurface,
    Trk::PropDirection dir,
    ParticleHypothesis particle,
    Trk::CacheOwnedPtr<Trk::TrackParameters>& referenceParameters,
    const Trk::Layer*& associatedLayer,
    const Trk::TrackingVolume*& associatedVolume,
    const Trk::TrackingVolume*& destinationVolume) const;

  /** Check for punchThrough in case of radial (perpendicular) direction change,
    returns true if the radial direction change is actually ok (i.e.
    punch-through allowed)
    */
  bool radialDirectionCheck(const EventContext& ctx,
                            const IPropagator& prop,
                            const TrackParameters& startParm,
                            const TrackParameters& parsOnLayer,
                            const TrackingVolume& tvol,
                            PropDirection dir = anyDirection,
                            ParticleHypothesis particle = pion) const;

  /** Access the subPropagator to the given volume*/
  const IPropagator* subPropagator(const TrackingVolume& tvol) const;

  /** Access the subPropagator to the given volume*/
  const IMaterialEffectsUpdator* subMaterialEffectsUpdator(const TrackingVolume& tvol) const;

  /** For the output - global position */
  std::string positionOutput(const Amg::Vector3D& pos) const;

  /** helper method for MaterialEffectsOnTrack to be added */
  void addMaterialEffectsOnTrack(const EventContext& ctx,
                                 Cache& cache,
                                 const Trk::IPropagator& prop,
                                 Trk::CacheOwnedPtr<Trk::TrackParameters> parm,
                                 const Trk::Layer& lay,
                                 const Trk::TrackingVolume& vol,
                                 Trk::PropDirection propDir,
                                 Trk::ParticleHypothesis) const;



  /** Private method for conversion of the synchronized geometry signature to
   * the natural subdetector ordering */
  // unsigned int geoIDToDetOrder(Trk::GeometrySignature geoid) const;

  // --------------- Used Tools ----------------------------- //
  //!<  Array of Propagators
  ToolHandleArray<IPropagator> m_propagators{ this, "Propagators", {} };
  //!<  Steo Propagator
  ToolHandle<IPropagator> m_stepPropagator{
    this,
    "STEP_Propagator",
    "Trk::STEP_Propagator/AtlasSTEP_Propagator"
  };
  //!<  Navigator for TrackingGeometry and magnetic fiels acces
  ToolHandle<INavigator> m_navigator{ this,
                                      "Navigator",
                                      "Trk::Navigator/AtlasNavigator" };
  //!<  Array of Material updatersc
  ToolHandleArray<IMaterialEffectsUpdator> m_updaters{
    this,
    "MaterialEffectsUpdators",
    {}
  };
  //!<  Array of MultipleScattering updaters
  ToolHandle<IMultipleScatteringUpdator> m_msupdater{
    this,
    "MultipleScatteringUpdater",
    "Trk::MultipleScatteringUpdator/AtlasMultipleScatteringUpdator"
  };
  //!<  Array of EnergyLoss updaters
  ToolHandle<IEnergyLossUpdator> m_elossupdater{ this,
                                                "EnergyLossUpdater",
                                                "Trk::EnergyLossUpdator/AtlasEnergyLossUpdator" };

  // ---------------- For Extrapolation handling ------------ //

   //!< Propagators to chose from (steered by signature)
  std::vector<const IPropagator*> m_subPropagators;
  //!< updaters to chose from (steered by signature)
  std::vector<const IMaterialEffectsUpdator*> m_subupdaters;

  // ---------------- For Extrapolator configuration ------------ //

  StringArrayProperty m_propNames{this, "SubPropagators", {},
    "configuration of subPropagators"};
  StringArrayProperty m_updatNames{this, "SubMEUpdators", {},
    "configuration of subupdaters"};

  // --------------- General steering & Navigation -------------- //

  BooleanProperty m_includeMaterialEffects{this, "ApplyMaterialEffects", true,
    "boolean to switch on/off material effects"};
  BooleanProperty m_stopWithNavigationBreak
    {this, "StopWithNavigationBreak", false,
     "return 0 if navigation breaks - for validation reasons"};
  BooleanProperty m_stopWithUpdateZero{this, "StopWithUpdateKill", false,
    "return 0 if update kills the trajectory"};
  BooleanProperty m_skipInitialLayerUpdate{this, "SkipInitialPostUpdate", false,
    "skip the initial post-Update at the layer [Fatras conversion mode]"};
  BooleanProperty m_resolveActive{this, "ResolveMuonStation", false};
  BooleanProperty m_resolveMultilayers{this, "ResolveMultilayers", true};
  //!< number of sub valid propagators in the m_subPropagators array
  //if we have no valid subpropagatos it will be set to an INVALID value
  unsigned int m_numOfValidPropagators;
  UnsignedIntegerProperty m_initialLayerAttempts
    {this, "InitialLayerAttempts", 3,
    "allowed layer intersection attempts at the start of a volume"};
  UnsignedIntegerProperty m_successiveLayerAttempts
    {this, "SuccessiveLayerAttempts", 1,
    "layer intersection attemps after one layer has been hit sucessfully"};
  UnsignedIntegerProperty m_maxMethodSequence
    {this, "MaximalMethodSequence", 2000};
  DoubleProperty m_tolerance{this, "Tolerance", 0.002,
    "surface & volume tolerance"};
  // ------------------------------------------------------- //
  BooleanProperty m_useMuonMatApprox{this, "UseMuonMatApproximation", false,
    "use approximative MS inert material"};
  BooleanProperty m_useDenseVolumeDescription
    {this, "UseDenseVolumeDescription", true,
     "use dense volume description when available in ID/Calo"};
  UnsignedIntegerProperty m_maxRecursion{this, "MaxRecursion", 1000};

  static const unsigned int m_maxNavigSurf = 1000;
  static const unsigned int m_maxNavigVol = 50;
  BooleanProperty m_dumpCache{this, "DumpCache", false};
  //------------ Magnetic field properties
  BooleanProperty m_fastField{this, "MagneticFieldProperties", false};
  Trk::MagneticFieldProperties m_fieldProperties;
  //------------Reference surface --------------
  std::unique_ptr<Surface> m_referenceSurface = nullptr;
  //-------------------------- SCREEN output steering -------------------------------------------//
  BooleanProperty m_printRzOutput{this, "positionOutput", true};
  //------------------------- VALIDATION  SECTION ------------------------------------------//
  // flags
  BooleanProperty m_navigationStatistics
    {this, "NavigationStatisticsOutput", false,
     "steer the output for the navigation statistics"};
  BooleanProperty m_navigationBreakDetails
    {this, "DetailedNavigationOutput", false,
     "steer the output for the navigation break details"};

  // extrapolation counters
  mutable Gaudi::Accumulators::Counter<> m_extrapolateCalls{};         //!< number of calls: extrapolate() method
  mutable Gaudi::Accumulators::Counter<> m_extrapolateBlindlyCalls{};  //!< number of calls: extrapolateBlindly() method
  mutable Gaudi::Accumulators::Counter<> m_extrapolateDirectlyCalls{}; //!< number of calls: extrapolateDirectly() method
  mutable Gaudi::Accumulators::Counter<> m_extrapolateStepwiseCalls{}; //!< number of calls: extrapolateStepwise() method

  mutable Gaudi::Accumulators::Counter<> m_startThroughAssociation{};        //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_startThroughRecall{};             //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_startThroughGlobalSearch{};       //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_destinationThroughAssociation{};  //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_destinationThroughRecall{};       //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_destinationThroughGlobalSearch{}; //!< navigation intialization
  mutable Gaudi::Accumulators::Counter<> m_layerSwitched{};                  //!< number of layers that have been switched

  // navigation counters
  mutable Gaudi::Accumulators::Counter<> m_navigationBreakLoop{}; //!< number of navigation breaks due to loop
  mutable Gaudi::Accumulators::Counter<> m_navigationBreakOscillation{}; //!< number of navigation breaks due to oscillation
  mutable Gaudi::Accumulators::Counter<> m_navigationBreakNoVolume{}; //!< number of navigation breaks due no Volume found
  mutable Gaudi::Accumulators::Counter<> m_navigationBreakDistIncrease{}; //!< number of navigation breaks due to distance increase
  mutable Gaudi::Accumulators::Counter<> m_navigationBreakVolumeSignature{}; //!< number of navigation breaks due to distance increase
  mutable Gaudi::Accumulators::Counter<> m_overlapSurfaceHit{}; //!< number of OverlapSurfaces found

  mutable Dbg::PropStat m_propStat ATLAS_THREAD_SAFE;
};

} // end of namespace
#include "TrkExTools/Extrapolator.icc"

#endif // TRKEXTOOLS_TRKEXTRAPOLATOR_H

