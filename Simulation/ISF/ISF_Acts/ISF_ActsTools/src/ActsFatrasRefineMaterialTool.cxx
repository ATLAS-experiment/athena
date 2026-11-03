/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsFatrasRefineMaterialTool.h"

#include "ActsGeometry/ActsDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SolidStateDetectorElementBase.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/ISurfaceMaterial.hpp"
#include "Acts/Material/MaterialSlab.hpp"

#include <memory>

StatusCode ISF::ActsFatrasRefineMaterialTool::initialize() {
  using namespace Acts::UnitConstants;
  m_silicon = Acts::Material::fromMassDensity(
      m_x0.value() * mm, m_l0.value() * mm, m_ar.value(), m_z.value(),
      m_rho.value() * g / cm3);
  ATH_MSG_INFO("Assigning silicon (X0=" << m_x0.value() << " mm, rho="
               << m_rho.value() << " g/cm3) to sensitive surfaces"
               << (m_overwrite ? ", overwriting existing material" : ""));
  return StatusCode::SUCCESS;
}

StatusCode ISF::ActsFatrasRefineMaterialTool::finalize() {
  ATH_MSG_INFO("Sensitive surfaces visited: " << m_nSensitive
               << ", silicon assigned: " << m_nAssigned
               << " (SiDetectorElement [ITk/PLR]: " << m_nSiElement
               << ", other solid-state [HGTD]: " << m_nOtherSolidState << ")"
               << ", kept existing material: " << m_nSkipped
               << ", replaced empty material: " << m_nReplacedEmpty
               << ", skipped non-Si: " << m_nNonSiSkipped
               << ", used fallback thickness: " << m_nFallbackThickness);
  return StatusCode::SUCCESS;
}

void ISF::ActsFatrasRefineMaterialTool::visitSurface(Acts::Surface& surface) {
  if (!surface.isSensitive()) {
    return;
  }
  ++m_nSensitive;

  // Keep only real material
  const Acts::ISurfaceMaterial* existing = surface.surfaceMaterial();
  if (existing != nullptr && !m_overwrite) {
    const Acts::MaterialSlab existingSlab = existing->materialSlab(Acts::Vector2::Zero());
    if (existingSlab.thicknessInX0() > 0.) {
      ++m_nSkipped;
      ATH_MSG_VERBOSE("Surface " << surface.geometryId()
                      << " already carries material (" << existingSlab.thicknessInX0()
                      << " X0), keeping it");
      return;
    }
    ++m_nReplacedEmpty;
    ATH_MSG_VERBOSE("Surface " << surface.geometryId()
                    << " carries empty/placeholder material, replacing it");
  }

  // All silicon sensors derive from InDetDD::SolidStateDetectorElementBase:
  //   - InDetDD::SiDetectorElement    : ITkPixel, ITkStrip, PLR
  //   - InDetDD::HGTD_DetectorElement : HGTD
  // ATLAS and ACTS both use mm
  double thickness = 0.;
  const ActsDetectorElement* actsDE = getActsDetectorElement(&surface);
  if (actsDE != nullptr) {
    const auto* solidDE = dynamic_cast<const InDetDD::SolidStateDetectorElementBase*>(
        actsDE->upstreamDetectorElement());
    if (solidDE != nullptr) {
      thickness = solidDE->thickness();
      if (dynamic_cast<const InDetDD::SiDetectorElement*>(solidDE) != nullptr) {
        ++m_nSiElement;   // ITk / PLR
      } else {
        ++m_nOtherSolidState;   // HGTD
      }
    } else {
      ATH_MSG_DEBUG("Surface " << surface.geometryId()
                    << " has no solid-state (silicon) detector element behind it");
    }
  } else {
    ATH_MSG_DEBUG("Surface " << surface.geometryId()
                  << " has no ActsDetectorElement");
  }
  if (!(thickness > 0.)) {
    // None silicon sensor : only put Si on it when explicitly requested
    if (!m_fallbackForNonSi) {
      ++m_nNonSiSkipped;
      return;
    }
    thickness = m_defaultThickness.value() * Acts::UnitConstants::mm;
    ++m_nFallbackThickness;
  }

  const Acts::MaterialSlab slab(*m_silicon, thickness);
  surface.assignSurfaceMaterial(
      std::make_shared<const Acts::HomogeneousSurfaceMaterial>(slab));
  ++m_nAssigned;

  ATH_MSG_VERBOSE("Assigned " << thickness << " mm Si to " << surface.geometryId());
}