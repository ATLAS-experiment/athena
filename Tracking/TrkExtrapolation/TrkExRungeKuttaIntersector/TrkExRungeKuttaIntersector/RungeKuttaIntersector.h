/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// Runge-Kutta method for tracking a particle through a magnetic field
// (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#ifndef TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H
#define TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkExInterfaces/IIntersector.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"

namespace Trk {

class RungeKuttaIntersector final : public extends<AthAlgTool, IIntersector> {

 public:
  RungeKuttaIntersector(const std::string& type, const std::string& name,
                        const IInterface* parent);
  virtual ~RungeKuttaIntersector() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /**IIntersector interface method for general Surface type */
  virtual std::optional<TrackSurfaceIntersection> intersectSurface(
      const Surface& surface, const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : PerigeeSurface */
  virtual std::optional<TrackSurfaceIntersection> approachPerigeeSurface(
      const PerigeeSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type :
   * StraightLineSurface */
  virtual std::optional<TrackSurfaceIntersection> approachStraightLineSurface(
      const StraightLineSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : CylinderSurface
   */
  virtual std::optional<TrackSurfaceIntersection> intersectCylinderSurface(
      const CylinderSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : DiscSurface */
  virtual std::optional<TrackSurfaceIntersection> intersectDiscSurface(
      const DiscSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for specific Surface type : PlaneSurface */
  virtual std::optional<TrackSurfaceIntersection> intersectPlaneSurface(
      const PlaneSurface& surface,
      const TrackSurfaceIntersection& trackIntersection,
      const double qOverP) const override;

  /**IIntersector interface method for validity check over a particular
   * extrapolation range */
  virtual bool isValid(Amg::Vector3D /*startPosition*/,
                       Amg::Vector3D /*endPosition*/) const override {
    return true;
  }

 private:
  // private methods
  void assignStepLength(const TrackSurfaceIntersection& isect,
                        double& stepLength) const;
  void debugFailure(TrackSurfaceIntersection&& isect,
                    const Surface& surface, const double qOverP,
                    const double rStart, const double zStart,
                    const bool trapped) const;
  double distanceToCylinder(const TrackSurfaceIntersection& isect,
                            const double cylinderRadius,
                            const double offsetRadius,
                            const Amg::Vector3D& offset,
                            double& stepLength) const;
  double distanceToDisc(const TrackSurfaceIntersection& isect,
                        const double discZ, double& stepLength) const;

  double distanceToLine(const TrackSurfaceIntersection& isect,
                        const Amg::Vector3D& linePosition,
                        const Amg::Vector3D& lineDirection,
                        double& stepLength) const;

  double distanceToPlane(const TrackSurfaceIntersection& isect,
                         const Amg::Vector3D& planePosition,
                         const Amg::Vector3D& planeNormal,
                         double& stepLength) const;

  Amg::Vector3D field(const Amg::Vector3D& point,
                      MagField::AtlasFieldCache& fieldCache) const;

  void initializeFieldCache(MagField::AtlasFieldCache& fieldCache) const;

  std::optional<TrackSurfaceIntersection> newIntersection(
      TrackSurfaceIntersection&& isect, const Surface& surface,
      const double qOverP, const double rStart, const double zStart) const;

  void shortStep(TrackSurfaceIntersection& isect,
                 const Amg::Vector3D& fieldValue, const double stepLength,
                 const double qOverP) const;

  void step(TrackSurfaceIntersection& isect, Amg::Vector3D& fieldValue,
            double& stepLength, const double qOverP,
            MagField::AtlasFieldCache& fieldCache) const;

  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey{
      this, "AtlasFieldCacheCondObj", "fieldCondObj",
      "Name of the Magnetic Field conditions object key"};

  // additional configuration
  BooleanProperty m_productionMode{this, "ProductionMode", true};

  // some precalculated constants:
  // r min for calo high field gradient region
  const double m_caloR0 = 1900.*Gaudi::Units::mm;
  // r max for calo high field gradient region
  const double m_caloR1 = 2500.*Gaudi::Units::mm;
  // r min for calo medium field gradient region
  const double m_caloR2 = 3500.*Gaudi::Units::mm;
  // r min for calo outer flux return region
  const double m_caloR3 = 3700.*Gaudi::Units::mm;
  // r max for calo medium field gradient region
  const double m_caloR4 = 3800.*Gaudi::Units::mm;
  // z min for calo medium field gradient region
  const double m_caloZ0 = 2350.*Gaudi::Units::mm;
  // z min for calo high field gradient region
  const double m_caloZ1 = 2600.*Gaudi::Units::mm;
  // z max for calo high field gradient region
  const double m_caloZ2 = 3600.*Gaudi::Units::mm;
  // z min for calo outer flux return region
  const double m_caloZ3 = 6000.*Gaudi::Units::mm;
  // end of central barrel near constant field region
  const double m_inDetR0 = 400.*Gaudi::Units::mm;
  // inner radius of middle/outer transition region
  const double m_inDetR1 = 350.*Gaudi::Units::mm;
  // outer radius of low field gradient field region
  const double m_inDetR2 = 800.*Gaudi::Units::mm;
  // end of central barrel near constant field region
  const double m_inDetZ0 = 350.*Gaudi::Units::mm;
  // start of well behaved transition region
  const double m_inDetZ1 = 420.*Gaudi::Units::mm;
  // start of endcap region
  const double m_inDetZ2 = 700.*Gaudi::Units::mm;
  // protection against loopers
  const double m_momentumThreshold = 1./20.*Gaudi::Units::MeV;
  // warning threshold for intersection failure
  const double m_momentumWarnThreshold = 1./450.*Gaudi::Units::MeV;
  // inner radius of barrel toroid region
  const double m_muonR0 = 4300.*Gaudi::Units::mm;
  // start of endcap toroid region
  const double m_muonZ0 = 6600.*Gaudi::Units::mm;
  double m_shortStepMax = 3.0*Gaudi::Units::mm;
  const double m_shortStepMin = 10.*Gaudi::Units::nanometer;
  // r max after coil (will take small steps near coil)
  const double m_solenoidR = 1300.*Gaudi::Units::mm;
  // z end of InDet region
  const double m_solenoidZ = 3500.*Gaudi::Units::mm;
  double m_stepMax0 = 8.0*Gaudi::Units::mm;
  double m_stepMax1 = 40.0*Gaudi::Units::mm;
  double m_stepMax2 = 80.0*Gaudi::Units::mm;
  double m_stepMax3 = 160.0*Gaudi::Units::mm;
  double m_stepMax4 = 320.0*Gaudi::Units::mm;
  int m_stepsUntilTrapped = 2000;
  const double m_third = 1./3.;
  // endcap toroid central field - inner radius
  const double m_toroidR0 = 1850.0*Gaudi::Units::mm;
  // endcap toroid central field - outer radius
  const double m_toroidR1 = 3500.0*Gaudi::Units::mm;
  // after inner barrel or outer endcap coil
  const double m_toroidR2 = 6000.0*Gaudi::Units::mm;
  // toroid region -  radius above which long steps OK
  const double m_toroidR3 = 6500.0*Gaudi::Units::mm;
  // endcap - near iron structure
  const double m_toroidZ0 = 7000.0*Gaudi::Units::mm;
  // endcap - high field gradient
  const double m_toroidZ1 = 8000.0*Gaudi::Units::mm;
  // barrel - before coil end loop
  const double m_toroidZ2 = 8700.0*Gaudi::Units::mm;
  // barrel - nearing coil end loop
  const double m_toroidZ3 = 9100.0*Gaudi::Units::mm;
  // endcap toroid full field and barrel coil regions
  const double m_toroidZ4 = 9500.0*Gaudi::Units::mm;
  // endcap toroid central field - inner z
  const double m_toroidZ5 = 9800.0*Gaudi::Units::mm;
  // endcap toroid central field - outer z
  const double m_toroidZ6 = 11400.0*Gaudi::Units::mm;
  // toroid exit fringe fields
  const double m_toroidZ7 = 12900.0*Gaudi::Units::mm;
  // essentially out of any toroid fields
  const double m_toroidZ8 = 14000.0*Gaudi::Units::mm;

  // counters
  mutable std::atomic<unsigned long long> m_countExtrapolations = 0;
  mutable std::atomic<unsigned long long> m_countShortStep = 0;
  mutable std::atomic<unsigned long long> m_countStep = 0;
  mutable std::atomic<unsigned long long> m_countStepReduction = 0;
};

//<<<<<< INLINE PRIVATE MEMBER FUNCTIONS                                >>>>>>

inline double RungeKuttaIntersector::distanceToCylinder(
    const TrackSurfaceIntersection& isect, const double cylinderRadius,
    const double offsetRadius, const Amg::Vector3D& offset,
    double& stepLength) const {
  const Amg::Vector3D& dir = isect.direction();

  double sinThsqinv = 1. / dir.perp2();
  stepLength = (offset.x() * dir.x() + offset.y() * dir.y()) * sinThsqinv;
  double deltaRSq = (cylinderRadius - offsetRadius) *
                        (cylinderRadius + offsetRadius) * sinThsqinv +
                    stepLength * stepLength;
  if (deltaRSq > 0.)
    stepLength += sqrt(deltaRSq);
  return std::abs(stepLength);
}

inline double RungeKuttaIntersector::distanceToDisc(
    const TrackSurfaceIntersection& isect, const double discZ,
    double& stepLength) const {
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double distance = discZ - pos.z();
  stepLength = distance / dir.z();
  return std::abs(distance);
}

inline double RungeKuttaIntersector::distanceToLine(
    const TrackSurfaceIntersection& isect, const Amg::Vector3D& linePosition,
    const Amg::Vector3D& lineDirection, double& stepLength) const {
  // offset joining track to line is given by
  //   offset = linePosition + a*lineDirection - trackPosition -
  //   b*trackDirection
  //
  // offset is perpendicular to both line and track at solution i.e.
  //   lineDirection.dot(offset) = 0
  //   trackDirection.dot(offset) = 0
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double cosAngle = lineDirection.dot(dir);
  stepLength = (linePosition - pos).dot(dir - lineDirection * cosAngle) /
               (1. - cosAngle * cosAngle);
  return std::abs(stepLength);
}

inline double RungeKuttaIntersector::distanceToPlane(
    const TrackSurfaceIntersection& isect, const Amg::Vector3D& planePosition,
    const Amg::Vector3D& planeNormal, double& stepLength) const {
  // take the normal component of the offset from track position to plane
  // position this is equal to the normal component of the required distance
  // along the track direction
  const Amg::Vector3D& pos = isect.position();
  const Amg::Vector3D& dir = isect.direction();

  double distance = planeNormal.dot(planePosition - pos);
  stepLength = distance / planeNormal.dot(dir);
  return std::abs(distance);
}

inline void RungeKuttaIntersector::initializeFieldCache(
    MagField::AtlasFieldCache& fieldCache) const {
  SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{
      m_fieldCacheCondObjInputKey};
  if (!fieldCondObj.isValid()) {
    ATH_MSG_FATAL("Failed to get magnetic field conditions data "
                  << m_fieldCacheCondObjInputKey.key());
  }
  fieldCondObj->getInitializedCache(fieldCache);
}

inline Amg::Vector3D RungeKuttaIntersector::field(
    const Amg::Vector3D& position,
    MagField::AtlasFieldCache& fieldCache) const {
  Amg::Vector3D fieldValue;
  fieldCache.getField(position.data(), fieldValue.data());
  return fieldValue;
}

inline std::optional<TrackSurfaceIntersection> RungeKuttaIntersector::newIntersection(
    TrackSurfaceIntersection&& isect, const Surface& surface,
    const double qOverP, const double rStart, const double zStart) const {
  // ensure intersection is valid (ie. on surface)
  Intersection SLIntersect = surface.straightLineIntersection(
      isect.position(), isect.direction(), false, false);
  if (SLIntersect.valid) {
    isect.position() = SLIntersect.position;
    return std::move(isect);
  }

  // invalid: take care to invalidate cache!
  if (msgLvl(MSG::DEBUG)){
    debugFailure(std::move(isect), surface, qOverP, rStart, zStart, false);
  }

  return std::nullopt;
}

}  // namespace Trk

#endif  // TRKEXRUNGEKUTTAINTERSECTOR_RUNGEKUTTAINTERSECTOR_H
