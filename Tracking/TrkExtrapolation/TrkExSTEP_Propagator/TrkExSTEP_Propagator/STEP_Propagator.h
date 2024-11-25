/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

/////////////////////////////////////////////////////////////////////////////////
//  Header file for class  STEP_Propagator
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for track status propagation through magnetic field
/////////////////////////////////////////////////////////////////////////////////
// Based on RungeKuttaPropagator Version 1.0 21/05/2004 I.Gavrilenko
// Version 0.1 16/2/2005 Esben Lund
// Extended by S.Todorova for propagation to multiple surfaces
/////////////////////////////////////////////////////////////////////////////////

#ifndef STEP_Propagator_H
#define STEP_Propagator_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkEventPrimitives/SurfaceTypes.h"
#include "TrkExInterfaces/IPropagator.h"
//
#include "TrkExUtils/MaterialInteraction.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"
#include "TrkGeometry/BinnedMaterial.h" //Identified material typedef
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkParameters/TrackParameters.h" //TrackParameters typedef
//#include "TrkExUtils/ExtrapolationCache.h"
// Amg
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
//
#include "CxxUtils/restrict.h"
#include <list>
#include <vector>

namespace CLHEP{
  class HepRandomEngine;
}

#include "TrkExInterfaces/ITimedMatEffUpdator.h"
namespace Trk {
class Surface;
class TrackingVolume;
class ITimedMatEffUpdator;
class ScatteringAngles;
class AlignableTrackingVolume;
class ExtrapolationCache;
/**
   @class STEP_Propagator

   STEP (Simultaneous Track and Error Propagation )
   is an algorithm for track parameters propagation through
   magnetic field.

   The algorithm can produce the Jacobian that transports the covariance matrix
   from one set of track parameters at the initial surface to another set of
   track parameters at the destination surface. This is useful for Chi2
   fitting.

   One can choose to perform the transport of the parameters only and omit the
   transport of the associated covariances (propagateParameters).

   The implementation performs the propagation in global coordinates and uses
   Jacobian matrices (see RungeKuttaUtils) for the transformations between the
   global frame and local surface-attached coordinate systems.

   The STEP_Propagator includes material effects in the
   equation of motion and applies corrections to the covariance matrices
   continuously during the parameter transport.  It is designed for the
   propagation  of a particle going through a dense block of material
   (e.g a muon transversing the calorimeter).

   1.The first step of the algorithm is track parameters transformation from
   local presentation for given start surface to global Runge Kutta coordinates.

   2.The second step is propagation through magnetic field with or without
   jacobian.

   3.Third step is transformation from global Runge Kutta presentation to local
   presentation of given output surface.


   AtaPlane    AtaStraightLine      AtaDisc       AtaCylinder      Perigee
   |               |               |               |              |
   |               |               |               |              |
   V               V               V               V              V
   -----------------------------------------------------------------
   |              Local->Global transformation
   V
   Global position (Runge Kutta presentation)
   |
   |
   Propagation to next surface with or without jacobian
   |
   V              Global->Local transformation
   ----------------------------------------------------------------
   |               |               |               |              |
   |               |               |               |              |
   V               V               V               V              V
   PlaneSurface StraightLineSurface DiscSurface CylinderSurface PerigeeSurface

   For propagation using Runge Kutta method we use global coordinate, direction,
   inverse momentum and Jacobian of transformation. All this parameters we save
   in array P[42].
   /dL0    /dL1    /dPhi   /dThe   /dCM
   X  ->P[0]  dX /   P[ 7]   P[14]   P[21]   P[28]   P[35]
   Y  ->P[1]  dY /   P[ 8]   P[15]   P[22]   P[29]   P[36]
   Z  ->P[2]  dZ /   P[ 9]   P[16]   P[23]   P[30]   P[37]
   Ax ->P[3]  dAx/   P[10]   P[17]   P[24]   P[31]   P[38]
   Ay ->P[4]  dAy/   P[11]   P[18]   P[25]   P[32]   P[39]
   Az ->P[5]  dAz/   P[12]   P[19]   P[26]   P[33]   P[40]
   CM ->P[6]  dCM/   P[13]   P[20]   P[27]   P[34]   P[41]

   where
   in case local presentation

   L0  - first  local coordinate  (surface dependent)
   L1  - second local coordinate  (surface dependent)
   Phi - Azimuthal angle
   The - Polar     angle
   CM  - charge/momentum

   in case global presentation

   X   - global x-coordinate        = surface dependent
   Y   - global y-coordinate        = surface dependent
   Z   - global z-coordinate        = sutface dependent
   Ax  - direction cosine to x-axis = Sin(The)*Cos(Phi)
   Ay  - direction cosine to y-axis = Sin(The)*Sin(Phi)
   Az  - direction cosine to z-axis = Cos(The)
   CM  - charge/momentum            = local CM

   The Runge-Kutta method:

   The equations of motion are solved using an embedded pair of Runge-Kutta
   formulas. This method, Runge-Kutta-Fehlberg, calculates a number of points
   along the step and adds them up in different ways to get two different
   solutions, of different order, for the integration. The higher order solution
   is used for the propagation and the lower order solution for error control.
The difference between these solutions is used to estimate the quality of the
   integration (propagation), and to calculate the step size for the next step.
   If the quality is below a given tolerance then the step is rejected and
repeated with a shorter step length. This propagator uses the TP43
   (Tsitouras-Papakostas 4th and 3rd order) Runge-Kutta pair.

   The step size algoritm by L.P.Endresen and J.Myrheim was choosen for its low
   step rejection and effective step size calculation. The low step rejection is
   achieved by letting the step size oscillate around the optimal value instead
   of repeating steps every time they fall below the tolerance level.

   Units are mm, MeV and kiloGauss.

   @author esben.lund@fys.uio.no
**/

class STEP_Propagator final
  : public AthAlgTool
  , virtual public IPropagator
{
  /////////////////////////////////////////////////////////////////////////////////
  // Public methods:
  /////////////////////////////////////////////////////////////////////////////////
public:
  using IPropagator::propagate;

  STEP_Propagator(const std::string&, const std::string&, const IInterface*);

  virtual ~STEP_Propagator();

  /** AlgTool initialize method.*/
  virtual StatusCode initialize() override final;

  /** AlgTool finalize method */
  virtual StatusCode finalize() override final;

  /** Main propagation method NeutralParameters.
   * It is not implemented for STEP propagator.*/
  virtual std::unique_ptr<Trk::NeutralParameters> propagate(
    const Trk::NeutralParameters&,
    const Trk::Surface&,
    Trk::PropDirection,
    const Trk::BoundaryCheck&,
    bool rC = false) const override final;

  /** Propagate parameters and covariance without returning the Jacobian */
  virtual std::unique_ptr<Trk::TrackParameters> propagate(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    const Trk::Surface& targetSurface,
    Trk::PropDirection propagationDirection,
    const Trk::BoundaryCheck& boundaryCheck,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Propagate parameters and covariance with search of closest surface */
  virtual std::unique_ptr<Trk::TrackParameters> propagate(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    std::vector<Trk::DestSurf>& targetSurfaces,
    Trk::PropDirection propagationDirection,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    std::vector<unsigned int>& solutions,
    double& path,
    bool usePathLimit = false,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Propagate parameters and covariance with search of closest surface
   * time included*/
  virtual std::unique_ptr<Trk::TrackParameters> propagateT(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    std::vector<Trk::DestSurf>& targetSurfaces,
    Trk::PropDirection propagationDirection,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    std::vector<unsigned int>& solutions,
    Trk::PathLimit& path,
    Trk::TimeLimit& time,
    bool returnCurv,
    const Trk::TrackingVolume* tVol,
    std::vector<Trk::HitInfo>*& hitVector) const override final;

  /** Propagate parameters and covariance with search of closest surface and
   * material collection */
  virtual std::unique_ptr<Trk::TrackParameters> propagateM(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    std::vector<Trk::DestSurf>& targetSurfaces,
    Trk::PropDirection propagationDirection,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    std::vector<unsigned int>& solutions,
    std::vector<const Trk::TrackStateOnSurface*>*& matstates,
    std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>>*
      intersections,
    double& path,
    bool usePathLimit = false,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr,
    Trk::ExtrapolationCache* = nullptr) const override final;

  /** Propagate parameters and covariance, and return the Jacobian. WARNING:
   * Multiple Scattering is not included in the Jacobian! */
  virtual std::unique_ptr<Trk::TrackParameters> propagate(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    const Trk::Surface& targetSurface,
    Trk::PropDirection propagationDirection,
    const Trk::BoundaryCheck& boundaryCheck,
    const MagneticFieldProperties& magneticFieldProperties,
    std::optional<Trk::TransportJacobian>& jacobian,
    double& pathLimit,
    ParticleHypothesis particle,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Propagate parameters only */
  virtual std::unique_ptr<Trk::TrackParameters> propagateParameters(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    const Trk::Surface& targetSurface,
    Trk::PropDirection propagationDirection,
    const Trk::BoundaryCheck& boundaryCheck,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Propagate parameters and return Jacobian. WARNING: Multiple Scattering is
   * not included in the Jacobian! */
  virtual std::unique_ptr<Trk::TrackParameters> propagateParameters(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    const Trk::Surface& targetSurface,
    Trk::PropDirection propagationDirection,
    const Trk::BoundaryCheck& boundaryCheck,
    const MagneticFieldProperties& magneticFieldProperties,
    std::optional<Trk::TransportJacobian>& jacobian,
    ParticleHypothesis particle,
    bool returnCurv = false,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Propagate parameters and return path (Similar to propagateParameters */
  virtual std::optional<TrackSurfaceIntersection> intersect(
    const EventContext& ctx,
    const Trk::TrackParameters& trackParameters,
    const Trk::Surface& targetSurface,
    const Trk::MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    const Trk::TrackingVolume* tVol = nullptr) const override final;

  /** Intersection and propagation:
   */
  virtual std::optional<TrackSurfaceIntersection> intersectSurface(
    const EventContext& ctx,
    const Surface& surface,
    const TrackSurfaceIntersection& trackIntersection,
    const double qOverP,
    const MagneticFieldProperties& mft,
    ParticleHypothesis particle) const override final;

  /** Return a list of positions along the track */
  virtual void globalPositions(
    const EventContext& ctx,
    std::deque<Amg::Vector3D>& positionsList,
    const TrackParameters& trackParameters,
    const MagneticFieldProperties& magneticFieldProperties,
    const CylinderBounds& cylinderBounds,
    double maxStepSize,
    ParticleHypothesis particle,
    const Trk::TrackingVolume* tVol = 0) const override final;

  /** unimplemented  multiStatePropagate*/
  virtual Trk::MultiComponentState multiStatePropagate(
    const EventContext&,
    const MultiComponentState&,
    const Surface&,
    const MagneticFieldProperties&,
    const PropDirection,
    const BoundaryCheck&,
    const ParticleHypothesis) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented multiStatePropagate");
    return {};
  }
  virtual Trk::ExtrapolationCode propagate(
    const EventContext&,
    Trk::ExCellCharged&,
    Trk::TargetSurfaces&,
    Trk::TargetSurfaceVector&) const override final
  {
    return Trk::ExtrapolationCode::FailureConfiguration;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Private methods:
  /////////////////////////////////////////////////////////////////////////////////

  /** stuct to pass information to the heavy lifting
   * calculation internal methods
   */
  struct Cache
  {
    bool m_energyLoss{ true };
    bool m_detailedElossFlag{ true };
    bool m_straggling{ true };
    bool m_solenoid{ false };
    //!< Switch for turning off material effects temporarily
    bool m_matPropOK{ true };
    bool m_brem{ false };
    bool m_includeBgradients{ true };
    bool m_includeGgradient{ false };
    bool m_MPV{ false };
    bool m_multipleScattering{ true };
    int m_propagateWithPathLimit{};
    size_t m_currentLayerBin{};
    double m_matupd_lastmom{};
    double m_matupd_lastpath{};
    double m_matdump_lastpath{};
    double m_delRad{ 0 };    // deRad/dl;
    double m_delIoni{ 0 };   // deIoni/dl;
    double m_sigmaIoni{ 0 }; // dsigma(ioni)/dl;
    double m_kazL{ 0 };      // kazL constant;
    double m_sigmaRad{ 0 };  // dsigma(rad)/dl;
    // cache for input variance
    double m_inputThetaVariance{};
    double m_stragglingVariance{};
    double m_pathLimit{};
    double m_timeOfFlight{};
    double m_timeStep{};
    double m_particleMass{ 0 }; //!< cache
    double m_charge{};
    double m_combinedThickness{};
    double m_tolerance{ 1e-05 };
    double m_momentumCutOff{ 50. };
    double m_scatteringScale{ 1. };
    double m_maxPath{ 100000. };
    double m_maxSteps{ 10000 };
    double m_layXmax{ 1. };
    // secondary interactions
    double m_timeIn{};
    double m_bremMom{ 0. };
    double m_bremEmitThreshold{ 0. };
    double m_bremSampleThreshold{ 0. };
    double m_P[45]{};

    const Trk::BinnedMaterial* m_binMat{ nullptr };
    //!< cache of TrackStateOnSurfaces
    std::vector<const Trk::TrackStateOnSurface*>* m_matstates{ nullptr };
    //!< cache of intersections
    std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>>*
      m_identifiedParameters{ nullptr };

    //!< cache of intersections/hit info
    std::vector<Trk::HitInfo>* m_hitVector{ nullptr };

    ParticleHypothesis m_particle{};
    const TrackingVolume* m_trackingVolume{ nullptr };
    const Material* m_material{ nullptr };
    //!< cache for collecting the total X0 ans Elos
    Trk::ExtrapolationCache* m_extrapolationCache{ nullptr };
    // cache for combined covariance matrix contribution
    AmgSymMatrix(5) m_combinedCovariance;
    // cache for differential covariance matrix contribution ( partial material
    // dump )
    AmgSymMatrix(5) m_covariance;
    Trk::EnergyLoss m_combinedEloss;
    std::vector<std::pair<int, std::pair<double, double>>> m_currentDist;
    MagField::AtlasFieldCache m_fieldCache;
    CLHEP::HepRandomEngine* m_randomEngine { nullptr };
    const EventContext& m_ctx;

    Cache (const EventContext& ctx) : m_ctx (ctx) {}
  };

private:
  /// initialize cache with the variables we need to take from
  // the configured properties.
  void setCacheFromProperties(Cache& cache) const
  {
    cache.m_includeBgradients = m_includeBgradients;
    cache.m_includeGgradient = m_includeGgradient;
    cache.m_energyLoss = m_energyLoss;
    cache.m_detailedElossFlag = m_detailedEloss;
    cache.m_MPV = m_MPV;
    cache.m_multipleScattering = m_multipleScattering;
    cache.m_straggling = m_straggling;
    cache.m_tolerance = m_tolerance;
    cache.m_momentumCutOff = m_momentumCutOff;
    cache.m_scatteringScale = m_scatteringScale;
    cache.m_maxPath = m_maxPath;
    cache.m_maxSteps = m_maxSteps;
    cache.m_layXmax = m_layXmax;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Main function for propagation
  // with search of closest surface (ST)
  /////////////////////////////////////////////////////////////////////////////////
  std::unique_ptr<Trk::TrackParameters> propagateRungeKutta(
    Cache& cache,
    bool errorPropagation,
    const Trk::TrackParameters& trackParameters,
    std::vector<DestSurf>& targetSurfaces,
    Trk::PropDirection propagationDirection,
    const MagneticFieldProperties& magneticFieldProperties,
    ParticleHypothesis particle,
    std::vector<unsigned int>& solutions,
    double& path,
    bool returnCurv = false) const;

  ////////////////////////////////////////////////////////////////////////////////
  // Method for propagation with search of closest surface (ST)
  ////////////////////////////////////////////////////////////////////////////////
  bool propagateWithJacobian(Cache& cache,
                             bool errorPropagation,
                             std::vector<DestSurf>& sfs,
                             double* P,
                             Trk::PropDirection propDir,
                             std::vector<unsigned int>& solutions,
                             double& path,
                             double sumPath) const;

  //////////////////////////////////////////////////////////////////////////////
  // dump material effects
  /////////////////////////////////////////////////////////////////////////////
  void dumpMaterialEffects(Cache& cache,
                           const Trk::CurvilinearParameters* trackParameters,
                           double path) const;

  ////////////////////////////////////////////////////////////////////////////
  // Momentum smearing (simulation mode)
  ////////////////////////////////////////////////////////////////////////////
  void smear(Cache& cache,
             double& phi,
             double& theta,
             const Trk::TrackParameters* parms,
             double radDist) const;
  //////////////////////////////////////////////////////////////////////////
  // Bremstrahlung (simulation mode)
  //////////////////////////////////////////////////////////////////////////
  void sampleBrem(Cache& cache, double mom) const;

  double m_tolerance; //!< Error tolerance. Low tolerance gives high accuracy
  bool m_materialEffects;    //!< Switch material effects on or off
  bool m_includeBgradients;  //!< Include B-gradients in the error propagation
  bool m_includeGgradient;   //!< Include g-gradient in the error propagation
  double m_momentumCutOff;   //!< Stop propagation below this momentum
  bool m_multipleScattering; //!< Switch multiple scattering on or off
  bool m_energyLoss;
  bool m_detailedEloss;
  bool m_straggling;
  bool m_MPV;
  double m_stragglingScale;
  double m_scatteringScale;
  double m_maxPath;
  double m_maxSteps;
  double m_layXmax;

  // The following are needed for simulation
  // enable simulation mode
  bool m_simulation;
  /** secondary interactions (brem photon emission)*/
  ToolHandle<ITimedMatEffUpdator> m_simMatUpdator;
  /** Random Generator service */
  ServiceHandle<IAthRNGSvc> m_rndGenSvc;
  /** Random engine */
  ATHRNG::RNGWrapper*                   m_rngWrapper = nullptr;
  std::string m_randomEngineName;

  // Read handle for conditions object to get the field cache
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey{
    this,
    "AtlasFieldCacheCondObj",
    "fieldCondObj",
    "Name of the Magnetic Field conditions object key"
  };
  void getFieldCacheObject(Cache& cache, const EventContext& ctx) const;

  CLHEP::HepRandomEngine* getRandomEngine (const EventContext& ctx) const;
};

} // end of namespace Trk

#endif // STEP_Propagator_H
