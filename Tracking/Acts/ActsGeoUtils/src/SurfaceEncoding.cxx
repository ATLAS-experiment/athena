/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SIMULATIONBASE

#include "ActsGeoUtils/SurfaceEncoding.h"

#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"

namespace ActsTrk {
void encodeSurface(xAOD::SurfaceType& surfaceType,
                   std::vector<float>& translation,
                   std::vector<float>& rotation,
                   std::vector<float>& boundValues,
                   const Acts::Surface* surface,
                   const Acts::GeometryContext& geoContext) {
  // return if surf is a nullptr
  if (surface == nullptr) {
    return;
  }

  // surface type specifics
  if (surface->type() == Acts::Surface::Cone) {
    surfaceType = xAOD::SurfaceType::Cone;
  } else if (surface->type() == Acts::Surface::Cylinder) {
    surfaceType = xAOD::SurfaceType::Cylinder;
  } else if (surface->type() == Acts::Surface::Disc) {
    surfaceType = xAOD::SurfaceType::Disc;
  } else if (surface->type() == Acts::Surface::Perigee) {
    surfaceType = xAOD::SurfaceType::Perigee;
  } else if (surface->type() == Acts::Surface::Plane) {
    surfaceType = xAOD::SurfaceType::Plane;
  } else if (surface->type() == Acts::Surface::Straw) {
    surfaceType = xAOD::SurfaceType::Straw;
  } else {
    throw std::out_of_range(
        "encodeSurface this type " +
        std::to_string(static_cast<int>(surface->type())) +
        " of Acts Surface can not be saved in xAOD::TrackSurface");
    return;
  }

  Acts::RotationMatrix3 lRotation =
      surface->transform(geoContext).rotation();
  Acts::Vector3 eulerAngles = lRotation.eulerAngles(2, 1, 0);
  Acts::Vector3 lTranslation = surface->center(geoContext);

  for (int i = 0; i < 3; i++) {
    rotation.push_back(eulerAngles[i]);
    translation.push_back(lTranslation[i]);
  }

  // copy and transform double->float
  const std::vector<double>& values = surface->bounds().values();
  for (double v: values)
    boundValues.push_back(v);
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
    const std::vector<float>& rotation, const std::vector<float>& boundValues,
    const Acts::GeometryContext&) {

  // Translation and rotation

  // create the transformation matrix
  auto transform = Acts::Transform3(
      Acts::Translation3(translation[0], translation[1], translation[2]));
  transform *=
      Acts::AngleAxis3(rotation[0], Acts::Vector3(0., 0., 1.));  // rotZ
  transform *=
      Acts::AngleAxis3(rotation[1], Acts::Vector3(0., 1., 0.));  // rotY
  transform *=
      Acts::AngleAxis3(rotation[2], Acts::Vector3(1., 0., 0.));  // rotX

  // cone
  if (surfaceType == xAOD::Cone) {
    auto surface = Acts::Surface::makeShared<Acts::ConeSurface>(
        transform, boundValues[0], boundValues[1], boundValues[2],
        boundValues[3]);
    return surface;
  }
  // Cylinder
  else if (surfaceType == xAOD::Cylinder) {
    // phi/2 must be slightly < Pi to avoid crashing
    float fixedPhi =
        boundValues[2] > M_PI - 0.001 ? M_PI - 0.001 : boundValues[2];
    auto surface = Acts::Surface::makeShared<Acts::CylinderSurface>(
        transform, boundValues[0], boundValues[1], fixedPhi, boundValues[3],
        boundValues[4]);
    return surface;
  }
  // Disc
  else if (surfaceType == xAOD::Disc) {
    auto surface = Acts::Surface::makeShared<Acts::DiscSurface>(
        transform, boundValues[0], boundValues[1], boundValues[2]);
    return surface;
  }
  // Perigee
  else if (surfaceType == xAOD::Perigee) {
    auto surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(transform);
    return surface;
  }
  // Plane
  else if (surfaceType == xAOD::Plane) {
    Acts::Vector2 min(boundValues[0], boundValues[1]),
        max(boundValues[2], boundValues[3]);
    auto rBounds = std::make_shared<const Acts::RectangleBounds>(min, max);
    auto surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(transform, rBounds);
    return surface;
  }
  // Straw
  else if (surfaceType == xAOD::Straw) {
    auto surface = Acts::Surface::makeShared<Acts::StrawSurface>(
        transform, boundValues[0], boundValues[1]);
    return surface;
  } else {
    throw std::out_of_range(
        "encodeSurface this type " +
        std::to_string(static_cast<int>(surfaceType)) +
        " of Acts Surface can not be saved in xAOD::TrackSurface");
    return nullptr;
  }
  return nullptr;
}

std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::TrackSurface* s, const Acts::GeometryContext& geo) {
  return decodeSurface(s->surfaceType(), s->translation(), s->rotation(),
                       s->boundValues(), geo);
}

std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::TrackSurfaceAuxContainer* s, size_t i,
    const Acts::GeometryContext& geo) {
  return decodeSurface(s->surfaceType[i], s->translation[i], s->rotation[i],
                       s->boundValues[i], geo);
}

}  // namespace ActsTrk
#endif