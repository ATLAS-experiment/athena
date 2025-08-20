/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TimedExtrapolator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXTOOLS_TIMEDEXTRAPOLATOR_H
#define TRKEXTOOLS_TIMEDEXTRAPOLATOR_H

#include "TrkExInterfaces/ITimedExtrapolator.h"

#include "TrkExInterfaces/INavigator.h"
#include "TrkExInterfaces/ITimedMatEffUpdator.h"

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// Trk
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/BoundaryCheck.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/BoundarySurfaceFace.h"
// STL
#include <cstring>
#include <map>
#include <vector>

// For marking not thread safety
#include "CxxUtils/checker_macros.h"
class MsgStream;

namespace Trk {

class Track;
class Surface;
class Layer;
class Volume;
class DetachedTrackingVolume;
class AlignableTrackingVolume;
class TrackingGeometry;
class TrackParticleBase;
class IPropagator;
class IDynamicLayerCreator;
class INavigator;
class IMultipleScatteringUpdator;
class IEnergyLossUpdator;

typedef std::vector<const Trk::TrackParameters*> TrackParametersVector;

typedef std::pair<const Surface*, BoundaryCheck> DestSurf;

struct DestBound
{
  const Surface* surface; // surface
  double distance;        // distance to surface
  unsigned int bIndex;    // boundary index for fast navigation to the next volume
  //
  DestBound(const Surface* surf, double dist, unsigned int index)
    : surface(surf)
    , distance(dist)
    , bIndex(index)
  {
  }
};

struct BoundaryTrackParameters
{
  const TrackParameters* trPar;
  const TrackingVolume* exitVol;
  const TrackingVolume* entryVol;
  //
  BoundaryTrackParameters(const TrackParameters* parms,
                          const TrackingVolume* exitTV,
                          const TrackingVolume* entryTV)
    : trPar(parms)
    , exitVol(exitTV)
    , entryVol(entryTV)
  {
  }
};

struct IdentifiedIntersection
{
  float distance;
  int identifier;
  const Trk::Material* material;
  //
  IdentifiedIntersection(float dist, int id, const Trk::Material* mat)
    : distance(dist)
    , identifier(id)
    , material(mat)
  {
  }
};

/** @struct ParametersAtBoundarySurface
    has only three member
    - BoundarySurface
    - TrackParameters
    - bool that indicated the deletion of the TrackParameters
 */

struct ParamsNextVolume
{
  //!< the members
  const TrackingVolume* nextVolume;
  const TrackParameters* nextParameters;
  const TrackParameters* navParameters;
  BoundarySurfaceFace exitFace;

  ParamsNextVolume()
    : nextVolume(nullptr)
    , nextParameters(nullptr)
    , navParameters(nullptr)
    , exitFace(undefinedFace)
  {
  }

  //!< update the boundaryInformation
  void boundaryInformation(const TrackingVolume* tvol,
                           const TrackParameters* nextPars,
                           const TrackParameters* navPars,
                           BoundarySurfaceFace face = undefinedFace)
  {
    nextVolume = tvol;
    nextParameters = nextPars;
    navParameters = navPars;
    exitFace = face;
  }

  //!< reset the boundary information by invalidating it
  void resetBoundaryInformation()
  {
    nextVolume = nullptr;
    nextParameters = nullptr;
    navParameters = nullptr;
    exitFace = undefinedFace;
  }
};

/**
  @class TimedExtrapolator

  The TimedExtrapolator is to be used for the simulation purposes

  The output level is as follows:
     INFO    : initialize / finalize information
     DEBUG   : Method call sequence
     VERBOSE : Method call sequence with values

  @author sarka.todorova@cern.ch
 */

class TimedExtrapolator final
  : public AthAlgTool
  , virtual public ITimedExtrapolator
{
public:
  /**Constructor */
  TimedExtrapolator(const std::string&, const std::string&, const IInterface*);
  /**Destructor*/
  virtual ~TimedExtrapolator();

  /** AlgTool initailize method.
  In this method the extrapolator should retrieve the Propagator of highest
  order which is then passed through the extrapolate method. The Propagator
  itself should be specified whether to use propagators of a lower hirarchy
  level or not.
   */
  virtual StatusCode initialize() override;
  /** AlgTool finalize method */
  virtual StatusCode finalize() override;

  /** Extrapolation method for charged, possibly unstable particles.
      The extrapolation is interrupted at subdetector boundary for
     surviving/stable particles.
  */
  virtual std::unique_ptr<const Trk::TrackParameters> extrapolateWithPathLimit(
    const Trk::TrackParameters& parm,
    Trk::PathLimit& pathLim,
    Trk::TimeLimit& time,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    std::vector<Trk::HitInfo>*& hitVector,
    Trk::GeometrySignature& nextGeoID,
    const Trk::TrackingVolume* boundaryVol = nullptr) const override;

  /** Transport method for neutral, possibly unstable particles.
      The extrapolation is interrupted at subdetector boundary for
     surviving/stable particles.
  */
  virtual std::unique_ptr<const Trk::TrackParameters>
  transportNeutralsWithPathLimit(
    const Trk::TrackParameters& parm,
    Trk::PathLimit& pathLim,
    Trk::TimeLimit& time,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    std::vector<Trk::HitInfo>*& hitVector,
    Trk::GeometrySignature& nextGeoId,
    const Trk::TrackingVolume* boundaryVol = nullptr) const override;

  /** Return the TrackingGeometry used by the Extrapolator (forward information
   * from Navigator)*/
  virtual const TrackingGeometry* trackingGeometry() const override;

  /** Validation Action:
      Can be implemented optionally, outside access to internal validation steps
   */
  virtual void validationAction() const override;

private:
  struct Cache;
  std::unique_ptr<const Trk::TrackParameters> extrapolateToVolumeWithPathLimit(
    Trk::TimedExtrapolator::Cache& cache,
    const Trk::TrackParameters& parm,
    Trk::TimeLimit& time,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    Trk::GeometrySignature& nextGeoID,
    const Trk::TrackingVolume* destVol) const;

  BoundaryTrackParameters extrapolateInAlignableTV(Trk::TimedExtrapolator::Cache& cache,
                                                   const Trk::TrackParameters& parm,
                                                   Trk::TimeLimit& time,
                                                   Trk::PropDirection dir,
                                                   Trk::ParticleHypothesis particle,
                                                   Trk::GeometrySignature& nextGeoId,
                                                   const Trk::AlignableTrackingVolume* aliTV) const;

  std::unique_ptr<const Trk::TrackParameters> transportToVolumeWithPathLimit(
    Trk::TimedExtrapolator::Cache& cache,
    const Trk::TrackParameters& parm,
    Trk::TimeLimit& time,
    Trk::PropDirection dir,
    Trk::ParticleHypothesis particle,
    Trk::GeometrySignature& nextGeoId,
    const Trk::TrackingVolume* boundaryVol) const;

  BoundaryTrackParameters transportInAlignableTV(Trk::TimedExtrapolator::Cache& cache,
                                                 const Trk::TrackParameters& parm,
                                                 Trk::TimeLimit& time,
                                                 Trk::PropDirection dir,
                                                 Trk::ParticleHypothesis particle,
                                                 Trk::GeometrySignature& nextGeoId,
                                                 const Trk::AlignableTrackingVolume* aliTV) const;

  /** Access the subPropagator to the given volume*/
  const IPropagator* subPropagator(const TrackingVolume& tvol) const;

  /** Access the subPropagator to the given volume*/
  const ITimedMatEffUpdator* subMaterialEffectsUpdator(const TrackingVolume& tvol) const;

  /** Private method for throwing into the GarbageBin */
  void throwIntoGarbageBin(Trk::TimedExtrapolator::Cache& cache,
                           const Trk::TrackParameters* garbage) const;

  /** Private method for emptying the GarbageBin */
  void emptyGarbageBin(Trk::TimedExtrapolator::Cache& cache, const Trk::TrackParameters*) const;

  /** Private to search for overlap surfaces */
  void overlapSearch(Trk::TimedExtrapolator::Cache& cache,
                     const IPropagator& prop,
                     const TrackParameters& parm,
                     const TrackParameters& parsOnLayer,
                     const Layer& lay,
                     // const TrackingVolume& tvol,
                     float time,
                     PropDirection dir = anyDirection,
                     const BoundaryCheck& bcheck = true,
                     ParticleHypothesis particle = pion,
                     bool startingLayer = false) const;

  /** Private method for conversion of the synchronized geometry signature to
   * the natural subdetector ordering */
  // unsigned int geoIDToDetOrder(Trk::GeometrySignature geoid) const;

  /** For the output - global position */
  std::string positionOutput(const Amg::Vector3D& pos) const;

  /** For the output - global momentum */
  static std::string momentumOutput(const Amg::Vector3D& mom);

  // --------------- Used Tools ----------------------------- //

  ToolHandleArray<IPropagator> m_propagators{this,
    "Propagators", {}, "Array of Propagators"};
  ToolHandle<IPropagator> m_stepPropagator{this,
    "STEP_Propagator", "Trk::STEP_Propagator/AtlasSTEP_Propagator"};
  ToolHandle<INavigator> m_navigator{this,
    "Navigator", "Trk::Navigator/AtlasNavigator",
    "Navigator for TrackingGeometry and magnetic fiels access"};
  ToolHandleArray<ITimedMatEffUpdator> m_updators{this,
    "MaterialEffectsUpdators", {}, "Array of Material Updators"};
  ToolHandleArray<IMultipleScatteringUpdator> m_msupdators{this,
    "MultipleScatteringUpdators", {}, " Array of MultipleScattering Updators"};
  ToolHandle<IEnergyLossUpdator> m_elossupdater{this,
    "EnergyLossUpdater", "Trk::EnergyLossUpdator/AtlasEnergyLossUpdator"};

  // ---------------- For Extrapolation handling ------------ //

  std::vector<const IPropagator*>
    m_subPropagators; //!< Propagators to chose from (steered by signature)
  std::vector<const ITimedMatEffUpdator*>
    m_subUpdators; //!< Updators to chose from (steered by signature)

  // ---------------- For Extrapolator configuration ------------ //

  StringArrayProperty m_propNames{this, "SubPropagators", {},
    "configuration of subPropagators"};
  StringArrayProperty m_updatNames{this, "SubMEUpdators", {},
    "configuration of subupdaters"};

  // --------------- General steering & Navigation -------------- //

  UnsignedIntegerProperty m_meotpIndex{
    this, "MaterialEffectsOnTrackProviderIndex", 0,
    "if several meotps are available in a volume steer which one to use"};
  unsigned int m_configurationLevel = 10; //!< see the supported levels of
                                          //!< configuration above
  BooleanProperty m_includeMaterialEffects{this, "ApplyMaterialEffects", true,
    "boolean to switch on/off material effects"};
  BooleanProperty m_stopWithNavigationBreak{this, "StopWithNavigationBreak", false,
    "return 0 if navigation breaks - for validation reasons"};
  BooleanProperty m_stopWithUpdateZero{this, "StopWithUpdateKill", false,
    "return 0 if update kills the trajectory"};
  BooleanProperty m_skipInitialLayerUpdate{this, "SkipInitialPostUpdate", false,
    "skip the initial post-Update at the layer [Fatras conversion mode]"};
  BooleanProperty m_referenceMaterial{this, "ReferenceMaterial", false,
    "use the reference material for the update"};
  UnsignedIntegerProperty m_initialLayerAttempts{this, "InitialLayerAttempts", 3,
    "allowed layer intersection attempts at the start of a volume"};
  UnsignedIntegerProperty m_successiveLayerAttempts{
    this, "SuccessiveLayerAttempts", 1,
    "layer intersection attemps after one layer has been hit sucessfully"};

  DoubleProperty m_tolerance{this, "Tolerance", 0.002, "surface & volume tolerance"};

  BooleanProperty m_caloMsSecondary{this, "CaloMsSecondary", false,
    "handling of secondaries beyond ID"};

  // ------------------------------------------------------- //

  BooleanProperty m_robustSampling{this, "RobustSampling", true};
  BooleanProperty m_useDenseVolumeDescription{
    this, "UseDenseVolumeDescription", true,
    "use dense volume description when available in ID/Calo"};
  BooleanProperty m_useMuonMatApprox{this, "UseMuonMatApproximation", false,
    "use approximative MS inert material"};
  BooleanProperty m_resolveActive{this, "ResolveMuonStation", false};
  BooleanProperty m_resolveMultilayers{this, "ResolveMultilayers", true};

  //-------------------------- SCREEN output steering
  //-------------------------------------------//
  BooleanProperty m_printHelpOutputAtInitialize{this, "HelpOutput", false};
  BooleanProperty m_printRzOutput{this, "positionOutput", true};
  // ----------------------------- navigation validation section
  // -----------------------------------------------------------

  BooleanProperty m_navigationStatistics{this, "NavigationStatisticsOutput", false,
    "steer the output for the navigation statistics"};
  BooleanProperty m_navigationBreakDetails{this, "DetailedNavigationOutput", false,
    "steer the output for the navigation break details"};
  BooleanProperty m_materialEffectsOnTrackValidation{this,
    "MaterialEffectsOnTrackValidation", false, "mat effects on track validation"};
  unsigned int m_maxNavigSurf{};
  unsigned int m_maxNavigVol{};

  struct Cache
  {
    Cache(unsigned int max_navig_surf = 1000.)
      : m_path(0., 0)
    {
      m_navigSurfs.reserve(max_navig_surf);
    }
    ~Cache()
    {
      for (std::pair<const Trk::TrackParameters*, bool> param : m_garbageBin) {
        if (param.second) {
          delete param.first;
        }
      }
    }
    bool m_dense{}; //!<  internal switch for resolved configuration

    // ------------ Recall / Boundary Information ----------------------- //
    ParamsNextVolume m_parametersAtBoundary;  //!< return helper for parameters and boundary
    std::vector<Trk::HitInfo>* m_hitVector{}; //!< return helper for hit info

    std::map<const Trk::TrackParameters*, bool>
      m_garbageBin; //!< garbage collection during extrapolation

    const Trk::TrackingVolume* m_currentStatic{};
    const Trk::TrackingVolume* m_currentDense{};
    const Trk::TrackingVolume* m_highestVolume{};
    std::pair<unsigned int, unsigned int> m_denseResolved{};
    unsigned int m_layerResolved{};
    std::vector<std::pair<const Trk::DetachedTrackingVolume*, unsigned int>> m_detachedVols;
    std::vector<std::pair<const Trk::TrackingVolume*, unsigned int>> m_denseVols;
    std::vector<std::pair<const Trk::TrackingVolume*, const Trk::Layer*>> m_navigLays;
    std::vector<DestSurf> m_staticBoundaries;
    std::vector<DestSurf> m_detachedBoundaries;
    std::vector<DestSurf> m_denseBoundaries;
    std::vector<DestSurf> m_navigBoundaries;
    std::vector<DestSurf> m_layers;
    PathLimit m_path;
    double m_time{};

    //------------------------- NAVIGATION --------
    //----------------------------------------------//
    int m_methodSequence{};

    // ------------------------------- cache
    // --------------------------------------------------------------------

    const Layer* m_lastMaterialLayer{}; //!< cache layer with last material update

    std::vector<std::pair<const Trk::Surface*, Trk::BoundaryCheck>> m_navigSurfs;
    std::vector<std::pair<const Trk::Surface*, double>> m_trSurfs;
    std::vector<Trk::DestBound> m_trStaticBounds; // need to cache the boundary index, too
    std::vector<std::pair<const Trk::Surface*, double>> m_trDenseBounds;
    std::vector<std::pair<const Trk::Surface*, double>> m_trLays;

    double m_particleMass{};
  };

  //------------ Magnetic field properties
  BooleanProperty m_fastField{this, "MagneticFieldProperties", false};
  Trk::MagneticFieldProperties m_fieldProperties;


};

inline const TrackingGeometry*
TimedExtrapolator::trackingGeometry() const
{
  if (m_navigator) {
    return m_navigator->trackingGeometry(Gaudi::Hive::currentContext());
  }
  return nullptr;
}

inline const IPropagator*
TimedExtrapolator::subPropagator(const Trk::TrackingVolume& tvol) const
{
  const IPropagator* currentPropagator = (tvol.geometrySignature() < m_subPropagators.size())
                                           ? m_subPropagators[tvol.geometrySignature()]
                                           : nullptr;
  if (!currentPropagator) {
    msg(MSG::ERROR) << "[!] Configuration problem: no Propagator found for volumeSignature: "
                    << tvol.geometrySignature() << endmsg;
  }
  return currentPropagator;
}

inline const ITimedMatEffUpdator*
TimedExtrapolator::subMaterialEffectsUpdator(const Trk::TrackingVolume& tvol) const
{
  return (tvol.geometrySignature() < m_subUpdators.size()) ? m_subUpdators[tvol.geometrySignature()]
                                                           : nullptr;
}

inline void
TimedExtrapolator::throwIntoGarbageBin(Trk::TimedExtrapolator::Cache& cache,
                                       const Trk::TrackParameters* pars) const
{
  if (pars)
    cache.m_garbageBin[pars] = true;
}


} // end of namespace

#endif // TRKEXTOOLS_TIMEDEXTRAPOLATOR_H

