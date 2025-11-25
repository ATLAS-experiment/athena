/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"

#include "GeoModelKernel/throwExcept.h"

// ATHENA
#include "ActsInterop/IdentityHelper.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/StripBoxDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"
#include "TrkSurfaces/AnnulusBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

// PACKAGE
#include "ActsGeometryInterfaces/GeometryContext.h"
// ACTS
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/PlyVisualization3D.hpp"


// STL
#include <mutex>
#include <variant>


using Acts::Surface;
using Acts::Transform3;

using namespace Acts::UnitLiterals;
using namespace ActsTrk;


constexpr double length_unit = 1_mm;

ActsDetectorElement::ActsDetectorElement(const InDetDD::SiDetectorElement &detElem) :
  GeoVDetectorElement{detElem.getMaterialGeom()},
  m_idHash(detElem.identifyHash()),
  m_type{detElem.isPixel() ? DetectorType::Pixel : DetectorType::Sct},
  m_detElement{&detElem},
  m_explicitIdentifier(detElem.identify())
{


  auto boundsType = detElem.bounds().type();

  m_thickness = detElem.thickness();


  if (boundsType == Trk::SurfaceBounds::Rectangle) {

    const InDetDD::SiDetectorDesign &design = detElem.design();
    double hlX = design.width() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto rectangleBounds = std::make_shared<const Acts::RectangleBounds>(hlX, hlY);

    m_bounds = rectangleBounds;
    m_surface = Acts::Surface::makeShared<Acts::PlaneSurface>(rectangleBounds, *this);

  } else if (boundsType == Trk::SurfaceBounds::Trapezoid) {

    const InDetDD::SiDetectorDesign &design = detElem.design();

    double minHlX = design.minWidth() / 2. * length_unit;
    double maxHlX = design.maxWidth() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto trapezoidBounds =
        std::make_shared<const Acts::TrapezoidBounds>(minHlX, maxHlX, hlY);

    m_bounds = trapezoidBounds;

    m_surface = Acts::Surface::makeShared<Acts::PlaneSurface>(trapezoidBounds, *this);


  } else if (boundsType == Trk::SurfaceBounds::Annulus) {

    const InDetDD::SiDetectorDesign &design = detElem.design();
    const auto *annulus = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&design);
    if (annulus == nullptr) {
      throw std::domain_error("ActsDetectorElement got inconsistent surface");
    }

    double phi = annulus->phiWidth();
    double phiS = annulus->stereo();
    double R = annulus->waferCentreR();
    double maxR = annulus->maxR();
    double minR = annulus->minR();

    // phiAvg is the bounds-internal local rotation. We don't want one
    double phiAvg = 0; 
    // phi is the total opening angle, set up symmetric phi bounds
    double phiMax = phi / 2.;
    double phiMin = -phiMax;


    Amg::Vector2D originStripXYRotated(R * (1 - std::cos(phiS)),
                                       R * std::sin(-phiS));

    auto annulusBounds = std::make_shared<Acts::AnnulusBounds>(
        minR, maxR, phiMin, phiMax, originStripXYRotated, phiAvg);
    m_bounds = annulusBounds;

    m_surface = Acts::Surface::makeShared<Acts::DiscSurface>(annulusBounds, *this);

  } else {
    std::cout << boundsType << std::endl;
    throw std::domain_error("ActsDetectorElement does not support this surface type");
  }
}

ActsDetectorElement::ActsDetectorElement(const Acts::Transform3 &trf, 
                                         const InDetDD::TRT_BaseElement &detElem, 
                                         const Identifier &id) :
    GeoVDetectorElement{detElem.getMaterialGeom()},
    m_idHash(detElem.identifyHash()),
    m_type{DetectorType::Trt},
    m_detElement{&detElem}, 
    m_trtTrf{std::make_unique<Amg::Transform3D>(trf)},
    m_explicitIdentifier(id)
{


  // we know this is a straw
  double length = detElem.strawLength() * 0.5 * length_unit;

  // we need to find the radius
  auto ecElem = dynamic_cast<const InDetDD::TRT_EndcapElement *>(&detElem);
  auto brlElem = dynamic_cast<const InDetDD::TRT_BarrelElement *>(&detElem);
  double innerTubeRadius{0.};
  if (ecElem) {
    innerTubeRadius = ecElem->getDescriptor()->innerTubeRadius() * length_unit;
  } else {
    if (brlElem) {
      innerTubeRadius =
          brlElem->getDescriptor()->innerTubeRadius() * length_unit;
    } else {
      THROW_EXCEPTION("Cannot get tube radius for element in ActsDetectorElement c'tor");
    }
  }

  auto lineBounds =
      std::make_shared<const Acts::LineBounds>(innerTubeRadius, length);
  m_bounds = lineBounds;

  m_surface = Acts::Surface::makeShared<Acts::StrawSurface>(lineBounds, *this);
}

ActsDetectorElement::ActsDetectorElement(const InDetDD::HGTD_DetectorElement &detElem, const Identifier &id) :
    GeoVDetectorElement{detElem.getMaterialGeom()},
    m_idHash(detElem.identifyHash()),
    m_type{DetectorType::Hgtd},
    m_detElement{&detElem}, 
    m_thickness{detElem.thickness()}, 
    m_explicitIdentifier{id}
{

  auto boundsType = detElem.bounds().type();

  if (boundsType == Trk::SurfaceBounds::Rectangle) {

    const InDetDD::HGTD_ModuleDesign &design = detElem.design();
    double hlX = design.width() / 2. * length_unit;
    double hlY = design.length() / 2. * length_unit;

    auto rectangleBounds =
        std::make_shared<const Acts::RectangleBounds>(hlX, hlY);

    m_bounds = rectangleBounds;

    m_surface = Acts::Surface::makeShared<Acts::PlaneSurface>(rectangleBounds, *this);
        
  } else {
    throw std::domain_error(
        "ActsDetectorElement: the surface type of HGTD is not does not Rectangle, it is wrong");
  }
}

Amg::Transform3D ActsDetectorElement::transform(const ActsTrk::DetectorAlignStore* store) const {
   
    GeoAlignmentStore* geoModelStore = store ? store->geoModelAlignment.get() : nullptr;
    Amg::Transform3D l2g{Amg::Transform3D::Identity()};
   switch (m_type) {
      case DetectorType::Hgtd:{
         l2g= m_detElement->getMaterialGeom()->getAbsoluteTransform(geoModelStore);
         break;
      } case DetectorType::Trt: {
         l2g = (*m_trtTrf);
         break;
      }
      /// Pixel or Sct
      default: {
          const auto& detElem = static_cast<const InDetDD::SiDetectorElement&>(*m_detElement);
          const InDetDD::SiDetectorDesign&design = detElem.design();
          const Trk::SurfaceBounds::BoundsType boundsType = detElem.bounds().type();

          // extra shift for split row modules
          Amg::Transform3D extraTransform{Amg::CLHEPTransformToEigen(detElem.recoToHitTransform())};    
          if (boundsType == Trk::SurfaceBounds::Rectangle &&
              typeid(design) == typeid(InDetDD::StripBoxDesign) ) {            
              extraTransform = design.moduleShift() * extraTransform;
          } else if (boundsType == Trk::SurfaceBounds::Annulus) {
              // need to rotate pi/2 to reproduce ABXY orientation, phiS so that phi=0
              // is center and symmetric
              const double phiShift = M_PI_2 - static_cast<const InDetDD::StripStereoAnnulusDesign&>(design).stereo();

              const Amg::Vector2D origin2D = static_cast<const Acts::AnnulusBounds&>(m_surface->bounds()).moduleOrigin();
              const Amg::Translation3D transl{origin2D.x(), origin2D.y(), 0};
              const Amg::Transform3D originTrf{transl * Amg::getRotateZ3D(-phiShift)};
              extraTransform = extraTransform * originTrf.inverse();
          }
          l2g = m_detElement->getMaterialGeom()->getAbsoluteTransform(geoModelStore) * extraTransform;
      }
   };
   // need to make sure translation has correct units
   l2g.translation() *= 1.0 / CLHEP::mm * length_unit;

   return l2g;

}
IdentityHelper ActsDetectorElement::identityHelper() const {
  if (detectorType() == DetectorType::Pixel || detectorType() == DetectorType::Sct) {
        return IdentityHelper(static_cast<const InDetDD::SiDetectorElement *>(m_detElement));
  } else {
    throw std::domain_error("Cannot get IdentityHelper for TRT element");
  }
}

const Acts::Transform3 &ActsDetectorElement::transform(const Acts::GeometryContext &anygctx) const {
    return m_trfCache.transform(anygctx);
}

unsigned int ActsDetectorElement::storeAlignedTransforms(const ActsTrk::DetectorAlignStore& store) const {
    if (store.detType != detectorType()) return 0;
    m_trfCache.getTransform(&store);
    return 1;
}

const Acts::Transform3 & ActsDetectorElement::getDefaultTransform() const {
    return m_trfCache.getTransform(nullptr);
}

const Acts::Surface &ActsDetectorElement::surface() const {
  return (*m_surface);
}

Acts::Surface &ActsDetectorElement::surface() {
  return (*m_surface);
}

const Trk::Surface &ActsDetectorElement::atlasSurface() const {
  if (const auto *detElem =
          dynamic_cast<const InDetDD::SiDetectorElement *>(m_detElement);
      detElem != nullptr) {
    return detElem->surface();
  } else {
    throw std::domain_error("Cannot get surface for TRT element");
  }
}

double ActsDetectorElement::thickness() const { return m_thickness; }

Identifier ActsDetectorElement::identify() const {
   return m_explicitIdentifier;
}

const GeoVDetectorElement *
ActsDetectorElement::upstreamDetectorElement() const {
  return m_detElement;
}
DetectorType ActsDetectorElement::detectorType() const { return m_type; }
