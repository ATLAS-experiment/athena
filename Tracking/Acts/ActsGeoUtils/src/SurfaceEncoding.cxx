/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "ActsGeoUtils/SurfaceEncoding.h"

#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {
/// Bound values are stored as float, so a half phi sector of pi round-trips off
/// by up to half a float ulp (~1.2e-7 rad) - either side, and far above the
/// 1e-9 rad ACTS uses to recognise full azimuth. Snap back onto pi so 2 pi
/// bounds stay closed, and clamp into the [0, pi] range ACTS requires.
double decodeHalfPhiSector(float halfPhiSector) {
  constexpr double fullAzimuthSnap =
      std::numeric_limits<float>::epsilon() * M_PI;
  const double value = halfPhiSector;
  if (std::abs(value - M_PI) < fullAzimuthSnap) {
    return M_PI;
  }
  return std::clamp(value, 0., M_PI);
}
}  // namespace

namespace ActsTrk {
void encodeSurface(xAOD::SurfaceType& surfaceType,
                   std::vector<float>& translation,
                   std::vector<float>& rotation,
                   std::vector<float>& boundValues,
                   const Acts::Surface* surface,
                   const Acts::GeometryContext& geoContext) {
  // return if surf is a nullptr
  if (!surface) {
    return;
  }
  switch (surface->type()){
     using enum Acts::Surface::SurfaceType;
     case Cone:
        surfaceType = xAOD::SurfaceType::Cone;
        break;
      case Cylinder: 
        surfaceType = xAOD::SurfaceType::Cylinder;
        break;
      case Disc:
        surfaceType = xAOD::SurfaceType::Disc;
        break;
      case Perigee:
        surfaceType = xAOD::SurfaceType::Perigee;
        break;
      case Plane:
        surfaceType = xAOD::SurfaceType::Plane;
        break;
      case Straw: 
        surfaceType = xAOD::SurfaceType::Straw;
        break;
      case Curvilinear:
        surfaceType = xAOD::SurfaceType::Curvilinear;
        break;
      case Point:
      case Other:
        surfaceType = xAOD::SurfaceType::Other;
        break;
  }

  Acts::RotationMatrix3 lRotation =
      surface->localToGlobalTransform(geoContext).rotation();
  Acts::Vector3 eulerAngles = lRotation.eulerAngles(2, 1, 0);
  Acts::Vector3 lTranslation = surface->center(geoContext);

  for (int i = 0; i < 3; ++i) {
    rotation.push_back(eulerAngles[i]);
    translation.push_back(lTranslation[i]);
  }
  // copy and transform double->float
  const std::vector<double>& values = surface->bounds().values();
  boundValues.insert(boundValues.end(), values.begin(), values.end());
}

void encodeSurface(xAOD::TrackSurfaceAuxContainer* s, size_t i,
                   const Acts::Surface* surface,
                   const Acts::GeometryContext& geo) {
  encodeSurface(s->surfaceType[i], s->translation[i], s->rotation[i],
                s->boundValues[i], surface, geo);
}

void encodeSurface(xAOD::TrackSurface* s, const Acts::Surface* surface,
                   const Acts::GeometryContext& geo) {
  xAOD::SurfaceType surfaceType;
  std::vector<float> translation, rotation, bounds;
  encodeSurface(surfaceType, translation, rotation, bounds, surface, geo);

  s->setSurfaceType(surfaceType);
  s->setTranslation(translation);
  s->setRotation(rotation);
  s->setBoundValues(bounds);
}

std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::SurfaceType surfaceType, const std::vector<float>& translation,
    const std::vector<float>& rotation, const std::vector<float>& boundValues) {

  // Translation and rotation

  // create the transformation matrix
  Amg::Isometry3D transform = 
                   Amg::getTranslate3D(translation[0], translation[1], translation[2]) *
                   Amg::getRotateZ3D(rotation[0]) *
                   Amg::getRotateY3D(rotation[1]) *
                   Amg::getRotateX3D(rotation[2]);

  switch (surfaceType) {
      using enum xAOD::SurfaceType;
      case Cone:
        return Acts::Surface::makeShared<Acts::ConeSurface>(std::move(transform),
                    boundValues[0], boundValues[1], boundValues[2],
                    decodeHalfPhiSector(boundValues[3]));
      case Cylinder:
        // Older files store two trailing bevel values, which are always zero.
        return Acts::Surface::makeShared<Acts::CylinderSurface>(std::move(transform),
                boundValues[0], boundValues[1], decodeHalfPhiSector(boundValues[2]),
                boundValues[3]);
      case Disc:
        return Acts::Surface::makeShared<Acts::DiscSurface>(std::move(transform),
                boundValues[0], boundValues[1], decodeHalfPhiSector(boundValues[2]));
        case Perigee: 
          return Acts::Surface::makeShared<Acts::PerigeeSurface>(std::move(transform));
        case Plane: {
          Acts::Vector2 min(boundValues[0], boundValues[1]),
                        max(boundValues[2], boundValues[3]);
          auto rBounds = std::make_shared<const Acts::RectangleBounds>(min, max);
          return Acts::Surface::makeShared<Acts::PlaneSurface>(std::move(transform), rBounds);
        } case Straw: 
          return Acts::Surface::makeShared<Acts::StrawSurface>(std::move(transform), 
                                                               boundValues[0], boundValues[1]);
          case Curvilinear:
          case Other: 
            THROW_EXCEPTION("EncodeSurface this type " <<static_cast<int>(surfaceType)<< 
                           " of xAOD::surface cannot be converted into an Acts one");
  }
  
  return nullptr;
}

std::shared_ptr<const Acts::Surface> decodeSurface(const xAOD::TrackSurface* s) {
  return decodeSurface(s->surfaceType(), s->translation(), s->rotation(),
                       s->boundValues());
}

std::shared_ptr<const Acts::Surface> decodeSurface(const xAOD::TrackSurfaceAuxContainer* s, 
                                                    size_t i) {
  return decodeSurface(s->surfaceType[i], s->translation[i], s->rotation[i],
                       s->boundValues[i]);
}

}  // namespace ActsTrk
#endif