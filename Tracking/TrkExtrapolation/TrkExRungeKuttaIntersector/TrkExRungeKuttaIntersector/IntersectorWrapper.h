/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

//////////////////////////////////////////////////////////////////////
// wrapper to IIntersector tool to provide IPropagator functionality
//   default configuration wraps the RungeKutta intersector
// (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#ifndef TRKEXRUNGEKUTTAINTERSECTOR_INTERSECTORWRAPPER_H
#define TRKEXRUNGEKUTTAINTERSECTOR_INTERSECTORWRAPPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkExInterfaces/IIntersector.h"

namespace Trk {
class TrackSurfaceIntersection;

class IntersectorWrapper final
  : public AthAlgTool
  , virtual public IPropagator
{

public:
  IntersectorWrapper(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);
  ~IntersectorWrapper(void); // destructor

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  /** N 0) <b>Neutral parameters method </b>
    - returns a ParametersBase object as well, 0 if the extrapolation did not
    succeed
    */
  virtual std::unique_ptr<NeutralParameters> propagate(
    const NeutralParameters&,
    const Surface&,
    PropDirection,
    const BoundaryCheck&,
    bool) const override final;
  /** [TrackParameters]
   * --------------------------------------------------------- */
  /** Propagation interface:
    The propagation method called by the TrkExtrapolator. The extrapolator
    is responsible for the underlying logic of which surface to go to.
    */
  virtual std::unique_ptr<TrackParameters> propagate(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    const MagneticFieldProperties& mprop,
    ParticleHypothesis particle,
    bool returnCurv,
    const TrackingVolume*) const override final;

  /** Propagation interface:
    The propagation method called by the TrkExtrapolator. The propagator
    finds the closest surface.
    */
  virtual std::unique_ptr<TrackParameters> propagate(
    const EventContext&,
    const TrackParameters&,
    std::vector<DestSurf>&,
    PropDirection,
    const MagneticFieldProperties&,
    ParticleHypothesis,
    std::vector<unsigned int>&,
    double&,
    bool,
    bool,
    const TrackingVolume*) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented propagate");
    return nullptr;
  }

  // unimplemented propagateT
  virtual std::unique_ptr<TrackParameters> propagateT(
    const EventContext&,
    const TrackParameters&,
    std::vector<DestSurf>&,
    PropDirection,
    const MagneticFieldProperties&,
    ParticleHypothesis,
    std::vector<unsigned int>&,
    PathLimit&,
    TimeLimit&,
    bool,
    const TrackingVolume*,
    std::vector<Trk::HitInfo>*&) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented propagate");
    return nullptr;
  }

  /** Propagation interface:
    The propagation method including the return of the TransportJacobian matrix.
    */
  virtual std::unique_ptr<TrackParameters> propagate(
    const EventContext& ctx,
    const TrackParameters&,
    const Surface&,
    PropDirection,
    const BoundaryCheck&,
    const MagneticFieldProperties&,
    std::optional<TransportJacobian>&,
    double&,
    ParticleHypothesis,
    bool,
    const TrackingVolume*) const override final;

  /** Propagation interface without Covariance matrix propagation
    the pathlength has to be returned for eventual following propagateCovariance
    */
  virtual std::unique_ptr<TrackParameters> propagateParameters(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    const MagneticFieldProperties& mprop,
    ParticleHypothesis particle = pion,
    bool returnCurv = false,
    const TrackingVolume* tVol = nullptr) const override final;

  /// implemented
  virtual std::unique_ptr<TrackParameters> propagateParameters(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    PropDirection dir,
    const BoundaryCheck& bcheck,
    const MagneticFieldProperties& mprop,
    std::optional<TransportJacobian>&,
    ParticleHypothesis particle = pion,
    bool returnCurv = false,
    const TrackingVolume* tVol = nullptr) const override final;

  /** Intersection interface:
    The intersection interface might be used by the material service as well to
    estimate the surfaces (sensitive and nonesensitive) while propagation
    */
  virtual std::optional<Trk::TrackSurfaceIntersection> intersect(
    const EventContext& ctx,
    const TrackParameters& parm,
    const Surface& sf,
    const MagneticFieldProperties& mprop,
    ParticleHypothesis particle = pion,
    const TrackingVolume* tVol = nullptr) const override final;

  // unimplemented globalPositions
  virtual void globalPositions(const EventContext&,
                               std::deque<Amg::Vector3D>&,
                               const TrackParameters&,
                               const MagneticFieldProperties&,
                               const CylinderBounds&,
                               double,
                               ParticleHypothesis,
                               const TrackingVolume*) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented globalPositions");
    return;
  }

  // unimplemented intersectSurface
  virtual std::optional<TrackSurfaceIntersection> intersectSurface(
    const EventContext&,
    const Surface&,
    const TrackSurfaceIntersection&,
    const double,
    const MagneticFieldProperties&,
    ParticleHypothesis) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented intersectSurface");
    return std::nullopt;
  }
  /** unimplemented propagateM*/
  virtual std::unique_ptr<Trk::TrackParameters> propagateM(
    const EventContext&,
    const TrackParameters&,
    std::vector<DestSurf>&,
    PropDirection,
    const MagneticFieldProperties&,
    ParticleHypothesis,
    std::vector<unsigned int>&,
    std::vector<const Trk::TrackStateOnSurface*>*&,
    std::vector<std::pair<std::unique_ptr<Trk::TrackParameters>, int>>*,
    double&,
    bool,
    bool,
    const Trk::TrackingVolume*,
    Trk::ExtrapolationCache*) const override final
  {
    ATH_MSG_ERROR("Call to non-implemented propagateM");
    return nullptr;
  }
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

private:
  struct Cache
  {
    double m_charge;
    double m_qOverP;
    std::optional<TrackSurfaceIntersection> m_intersection;
    std::unique_ptr<TrackParameters> m_parameters;
    Amg::Vector3D m_position;
    Amg::Vector3D m_momentum;

    Cache()
      : m_charge{}
      , m_qOverP{}
      , m_intersection{ std::nullopt }
      , m_parameters{ nullptr }
      , m_position{}
      , m_momentum{}
    {
    }
  };
  // private methods
  void createParameters(Cache& cache,
                        const Surface& surface,
                        const BoundaryCheck& boundsCheck,
                        bool curvilinear) const;
  void findIntersection(Cache& cache,
                        const TrackParameters& parameters,
                        const Surface& surface,
                        PropDirection dir = Trk::anyDirection) const;

  // helpers, managers, tools
  ToolHandle<IIntersector> m_intersector{this, "Intersector",
    "Trk::RungeKuttaIntersector/RungeKuttaIntersector"};
  ToolHandle<IPropagator> m_linePropagator{this, "LinePropagator", ""};
};

} // end of namespace

#endif // TRKEXRUNGEKUTTAINTERSECTOR_INTERSECTORWRAPPER_H
