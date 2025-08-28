/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Navigator.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk inlcudes
#include "TrkExTools/Navigator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h"
#include "TrkExUtils/RungeKuttaUtils.h"
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrUtils/ObjectAccessor.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkVolumes/BoundarySurface.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"
// Data Model
#include "AthContainers/DataVector.h"
// Amg
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include <exception>
namespace{
const Trk::MagneticFieldProperties s_zeroMagneticField(Trk::NoField);
}

// constructor
Trk::Navigator::Navigator(const std::string &t, const std::string &n, const IInterface *p) :
  AthAlgTool(t, n, p)
  {
    declareInterface<INavigator>(this);
  }


// initialize
StatusCode
Trk::Navigator::initialize() {
  //We can use conditions when the key is not empty
  m_useConditions=!m_trackingGeometryReadKey.key().empty();
  // get the TrackingGeometry
  if (!m_useConditions) {
    if (m_trackingGeometrySvc.retrieve().isSuccess()) {
      ATH_MSG_DEBUG("Successfully retrieved " << m_trackingGeometrySvc);
      m_trackingGeometryName = m_trackingGeometrySvc->trackingGeometryName();
    } else {
      ATH_MSG_WARNING("Couldn't retrieve " << m_trackingGeometrySvc << ". ");
      ATH_MSG_WARNING(" -> Trying to retrieve default '"
                      << m_trackingGeometryName << "' from DetectorStore.");
    }
  }

  ATH_CHECK(m_trackingGeometryReadKey.initialize(m_useConditions));

  m_fieldProperties = m_fastField
                        ? Trk::MagneticFieldProperties(Trk::FastField)
                        : Trk::MagneticFieldProperties(Trk::FullField);

  return StatusCode::SUCCESS;
}

const Trk::TrackingVolume*
Trk::Navigator::volume(const EventContext& ctx, const Amg::Vector3D& gp) const
{
  return (trackingGeometry(ctx)->lowestTrackingVolume(gp));
}

const Trk::TrackingVolume*
Trk::Navigator::highestVolume(const EventContext& ctx) const
{
  return (trackingGeometry(ctx)->highestTrackingVolume());
}

const Trk::BoundarySurface<Trk::TrackingVolume>*
Trk::Navigator::nextBoundarySurface(const EventContext& ctx,
                                    const Trk::IPropagator& prop,
                                    const Trk::TrackParameters& parms,
                                    Trk::PropDirection dir) const
{
  const Trk::TrackingVolume* trackingVolume = volume(ctx,parms.position());
  if (trackingVolume) {
    return (nextBoundarySurface(ctx,prop, parms, dir, *trackingVolume));
  }
  return nullptr;
}

const Trk::BoundarySurface<Trk::TrackingVolume>*
Trk::Navigator::nextBoundarySurface(const EventContext& ctx,
                                    const Trk::IPropagator& prop,
                                    const Trk::TrackParameters& parms,
                                    Trk::PropDirection dir,
                                    const Trk::TrackingVolume& vol) const
{
  // get the surface accessor
  Trk::ObjectAccessor surfAcc = vol.boundarySurfaceAccessor(
    parms.position(), dir * parms.momentum().normalized());
  // initialize the currentBoundary surface
  const Trk::BoundarySurface<Trk::TrackingVolume>* currentBoundary = nullptr;
  bool outsideVolume = surfAcc.inverseRetrieval();
  // attempt counter
  int tryBoundary = 0;

  // set the prop direction according to inverseRetrieval result
  Trk::PropDirection searchDir = dir;
  if (outsideVolume) {
    searchDir =
      (dir == Trk::alongMomentum) ? Trk::oppositeMomentum : Trk::alongMomentum;
  }

  // debug version
  ATH_MSG_VERBOSE("g  [N] Starting parameters are :" << parms);

  // loop over the the boundary surfaces according to the accessor type
  for (const Trk::ObjectAccessor::value_type& surface_id : surfAcc) {
    ++tryBoundary;
    // ----------------- output to screen if outputLevel() says so --------
    ATH_MSG_VERBOSE("  [N] " << tryBoundary << ". try - BoundarySurface "
                             << surface_id << " of Volume: '"
                             << vol.volumeName() << "'.");
    // get the boundary Surface according to the surfaceAccessor
    currentBoundary = vol.boundarySurface(surface_id);
    const Trk::Surface& currentSurface =
      currentBoundary->surfaceRepresentation();

    // do either RungeKutta (always after first unsuccessful try) or straight
    // line
    auto trackPar =
      (!m_useStraightLineApproximation || tryBoundary > 1)
        ? prop.propagateParameters(
            ctx, parms, currentSurface, searchDir, true, m_fieldProperties)
        : prop.propagateParameters(
            ctx, parms, currentSurface, searchDir, true, s_zeroMagneticField);

    if (trackPar) {
      ATH_MSG_VERBOSE(
        "  [N] --> next BoundarySurface found with Parameters: " << *trackPar);
      return currentBoundary;
    }
  }
  return nullptr;
}

Trk::NavigationCell
Trk::Navigator::nextTrackingVolume(const EventContext& ctx,
                                   const Trk::IPropagator& prop,
                                   const Trk::TrackParameters& parms,
                                   Trk::PropDirection dir,
                                   const Trk::TrackingVolume& vol) const
{

  bool first = false;
  bool second = false;

  // ---------------------------------------------------
  // get the object accessor from the Volume
  Trk::ObjectAccessor surfAcc = vol.boundarySurfaceAccessor(
    parms.position(), dir * parms.momentum().normalized());
  // the object accessor already solved the outside question
  bool outsideVolume = surfAcc.inverseRetrieval();
  // initialize the boundary pointer / tracking volume pointer
  const Trk::BoundarySurface<Trk::TrackingVolume>* currentBoundary = nullptr;
  const Trk::TrackingVolume* nextVolume = nullptr;

  // debug version
  ATH_MSG_VERBOSE("  [N] Starting parameters are : " << parms);
  ATH_MSG_VERBOSE("  [N] This corresponds to [r,z] = [ "
                  << parms.position().perp() << ", " << parms.position().z()
                  << "]");
  ATH_MSG_VERBOSE("  [N] Boundary Surface accessor : " << surfAcc);

  // set the prop direction according to inverseRetrieval result
  Trk::PropDirection searchDir = dir;
  if (outsideVolume) {
    ATH_MSG_VERBOSE("  [N] Parameters have been flagged as being outside !");
    searchDir =
      (dir == Trk::alongMomentum) ? Trk::oppositeMomentum : Trk::alongMomentum;
  }

  // loop over boundary surfaces
  int tryBoundary = 0;

  for (const Trk::ObjectAccessor::value_type& surface_id : surfAcc) {
    ++tryBoundary;
    // get the boundary surface associated to the surfaceAccessor
    currentBoundary = vol.boundarySurface(surface_id);

    // ----------------- output to screen if outputLevel() says so --------
    if (!currentBoundary) {
      ATH_MSG_WARNING("  [N] " << tryBoundary << ". try - BoundarySurface "
                               << surface_id << " of Volume: '"
                               << vol.volumeName() << "' NOT FOUND.");
      continue;
    }
      ATH_MSG_VERBOSE("  [N] " << tryBoundary << ". try - BoundarySurface "
                               << surface_id << " of Volume: '"
                               << vol.volumeName() << "'.");


    const Trk::Surface& currentSurface =
      currentBoundary->surfaceRepresentation();
    // try the propagation
    std::unique_ptr<Trk::TrackParameters> trackPar = nullptr;
    // do either RungeKutta (always after first unsuccessful try) or straight
    // line
    if (!currentSurface.isOnSurface(parms.position(), true, 0., 0.)) {
      trackPar =
        (!m_useStraightLineApproximation || tryBoundary > 1)
          ? prop.propagateParameters(
              ctx, parms, currentSurface, searchDir, true, m_fieldProperties)
          : prop.propagateParameters(
              ctx, parms, currentSurface, searchDir, true, s_zeroMagneticField);
    } else {
      trackPar.reset(parms.clone()); //to be revisited
    }
    if (trackPar) {
      // the next volume pointer
      nextVolume = currentBoundary->attachedVolume(
          trackPar->position(), trackPar->momentum().normalized(), dir);
      return {nextVolume, std::move(trackPar),
              Trk::BoundarySurfaceFace(surface_id)};
    }

    // ---------------------------------------------------
    if (!first && searchDir == Trk::alongMomentum) {
      first = true;
    } else if (!second && searchDir == Trk::alongMomentum) {
      second = true;
    } else if (searchDir == Trk::alongMomentum) {
    } else if (!first && searchDir == Trk::oppositeMomentum) {
      first = true;
    } else if (!second && searchDir == Trk::oppositeMomentum) {
      second = true;
    } else if (searchDir == Trk::oppositeMomentum) {
    }
    // ---------------------------------------------------
  }
  // return what you have : no idea
  return {nullptr, nullptr};
}

bool
Trk::Navigator::atVolumeBoundary(const Trk::TrackParameters* parms,
                                 const Trk::TrackingVolume* vol,
                                 Trk::PropDirection dir,
                                 const Trk::TrackingVolume*& nextVol,
                                 double tol) const
{
  bool isAtBoundary = false;
  nextVol = nullptr;
  if (!vol) {
    return isAtBoundary;
  }
  const auto& bounds = vol->boundarySurfaces();
  for (unsigned int ib = 0; ib < bounds.size(); ib++) {
    const Trk::Surface &surf = bounds[ib]->surfaceRepresentation();
    if (surf.isOnSurface(parms->position(), true, tol, tol)) {

      // sanity check to enforce the desired tolerance
      Trk::DistanceSolution distSol = surf.straightLineDistanceEstimate(parms->position(),
                                                                        dir * parms->momentum().unit());
      if (distSol.currentDistance(false) < tol && distSol.numberOfSolutions() > 0) {
        isAtBoundary = true;
        const Trk::TrackingVolume* attachedVol =
          (bounds[ib])
            ->attachedVolume(parms->position(), parms->momentum(), dir);
        if (!nextVol && attachedVol) {
          nextVol = attachedVol;
        }
        // double good solution indicate tangential intersection : revert the attached volumes
        if (distSol.numberOfSolutions() > 1 && std::abs(distSol.first()) < tol && std::abs(distSol.second()) < tol) {
         if (!nextVol) {
           ATH_MSG_WARNING("Tracking volume "
                           << (*vol)
                           << " has loose ends. because the navigation of "
                           << std::endl
                           << (*parms) << std::endl
                           << " failed. Please consult the experts or have a "
                              "look at ATLASRECTS-7147");
           continue;
          }
          //surfing the beampipe seems to happen particularly often in a Trigger test.
          //see https://its.cern.ch/jira/browse/ATR-24234
          //in this case, I downgrade the 'warning' to 'verbose'
          const bool surfingTheBeamPipe = (vol->geometrySignature() == Trk::BeamPipe) or (nextVol->geometrySignature() == Trk::BeamPipe);
          if (not surfingTheBeamPipe) {
            ATH_MSG_WARNING("navigator detects tangential intersection: switch of volumes reverted ");
          } else {
            ATH_MSG_VERBOSE("navigator detects particle entering and re-entering the beampipe");
          }
          if (nextVol and (not surfingTheBeamPipe)) {
            ATH_MSG_WARNING(vol->volumeName() << "->" << nextVol->volumeName() << "->" << vol->volumeName());
          }
          isAtBoundary = false;
          // revert attached volume
          nextVol = vol;
        }
      }
    }
  }

  return isAtBoundary;
}

const Trk::TrackParameters*
Trk::Navigator::closestParameters(const Trk::Track& trk,
                                  const Trk::Surface& sf) const
{

  // search with dedicated algorithms for cylinder/sl/perigee
  // surface
  const Trk::TrackParameters *closestTrackParameters = nullptr;

  // policy change --- only measured parameters are taken
  DataVector<const TrackParameters>::const_iterator it = trk.trackParameters()->begin();
  std::vector<const Trk::TrackParameters *> measuredParameters;
  measuredParameters.reserve(trk.trackParameters()->size());
  for (; it != trk.trackParameters()->end(); ++it) {
    // dynamic cast the Measured ones
    const Trk::TrackParameters *mtp = *it;
    if (!mtp || !mtp->covariance()) {
      continue;
    }
    measuredParameters.push_back(*it);
  }

  // new policy --- take only measured parameters
  if (measuredParameters.empty()) {
    return nullptr;
  }

  if (m_searchWithDistance) {
    // loop over the track parameters and get the distance
    std::vector<const Trk::TrackParameters *>::const_iterator tpIter = measuredParameters.begin();
    std::vector<const Trk::TrackParameters *>::const_iterator tpIterEnd = measuredParameters.end();
    // set a maximum distance
    double closestDistance = 10e10;
    const Trk::TrackParameters *currentClosestParameters = nullptr;

    for (; tpIter != tpIterEnd; ++tpIter) {
      // forward-backward solution
      Amg::Vector3D tpDirection = (*tpIter)->momentum().normalized();

      Trk::DistanceSolution currentDistance = sf.straightLineDistanceEstimate((*tpIter)->position(), tpDirection);
      if (currentDistance.numberOfSolutions() > 0) {
        // get the one/two solution(s)
        double firstDistance = std::abs(currentDistance.first());
        double secondDistance = currentDistance.numberOfSolutions() >
                                1 ? std::abs(currentDistance.second()) : firstDistance;
        // now do the check
        if (firstDistance < closestDistance || secondDistance < closestDistance) {
          currentClosestParameters = (*tpIter);
          closestDistance = firstDistance <= secondDistance ? firstDistance : secondDistance;
        }
      }
    }

    // return what has shown to be closest
    return currentClosestParameters;
  }

  if (sf.type() == Trk::SurfaceType::Cylinder) {
    Trk::TrkParametersComparisonFunction tParFinderCylinder(sf.bounds().r());
    closestTrackParameters =
        *(std::min_element(measuredParameters.begin(), measuredParameters.end(),
                           tParFinderCylinder));
    return closestTrackParameters;
  }

  if (sf.type() == Trk::SurfaceType::Line ||
      sf.type() == Trk::SurfaceType::Perigee) {
    Trk::TrkParametersComparisonFunction tParFinderLine(
        sf.center(), sf.transform().rotation().col(2));
    closestTrackParameters = *(std::min_element(
        measuredParameters.begin(), measuredParameters.end(), tParFinderLine));
    return closestTrackParameters;
  }

  Trk::TrkParametersComparisonFunction tParFinderCenter(sf.center());
  closestTrackParameters = *(std::min_element(measuredParameters.begin(), measuredParameters.end(), tParFinderCenter));

  return closestTrackParameters;
}

const Trk::TrackingGeometry*
Trk::Navigator::trackingGeometry(const EventContext& ctx) const
{
  if (m_useConditions) {
    SG::ReadCondHandle<TrackingGeometry> handle(m_trackingGeometryReadKey, ctx);
    if (!handle.isValid()) {
      throw std::runtime_error{
        "Could not retrieve TrackingGeometry from Conditions Store."
      };
    }
    return handle.cptr();
  } else {
    const TrackingGeometry* trackingGeometry = nullptr;
    if (detStore()
          ->retrieve(trackingGeometry, m_trackingGeometryName)
          .isFailure()) {
      throw std::runtime_error{
        "Could not retrieve TrackingGeometry from Detector Store."
      };
    }
    return trackingGeometry;
  }
}
