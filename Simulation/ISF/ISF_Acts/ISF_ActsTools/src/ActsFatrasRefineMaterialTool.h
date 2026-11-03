/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_ACTSTOOLS_ACTSFATRASREFINEMATERIALTOOL_H
#define ISF_ACTSTOOLS_ACTSFATRASREFINEMATERIALTOOL_H

// Enables the ATLAS Eigen plugin first
#include "GeoPrimitives/GeoPrimitives.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsGeometryInterfaces/IRefineTrackingGeoTool.h"

#include "Acts/Material/Material.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include <cstddef>
#include <optional>

namespace ISF {

/** @class ActsFatrasRefineMaterialTool
 *
 *  Refinement visitor for the ACTS TrackingGeometry: assigns a homogeneous
 *  silicon MaterialSlab to every sensitive surface.*
 *  The material is a single slab of silicon whose thickness is taken from
 *  the underlying detector element where available, and from the
 *  DefaultThickness property otherwise.
 *  The tool is a no-op for non-sensitive surfaces and, by default, leaves
 *  existing non-empty material untouched.
 *  Plugged into ActsTrk::TrackingGeometrySvc through the "RefinementTools"
 *  SingleParticleSimulation is constructed.
 */
class ActsFatrasRefineMaterialTool
    : public extends<AthAlgTool, ActsTrk::IRefineTrackingGeoTool> {
 public:
  using base_class::base_class;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /// Called by TrackingGeometry::apply() for every surface in the geometry
  virtual void visitSurface(Acts::Surface& surface) override;

 private:
  /// Sensitive surfaces without a silicon (solid-state) detector element,
  /// e.g. TRT straws, are skipped unless this is set
  Gaudi::Property<bool> m_fallbackForNonSi{this, "FallbackForNonSi", false,
      "Assign DefaultThickness of Si also to sensitive surfaces that are not Si sensors"};
  /// Sensor thickness used when it cannot be taken from the detector element
  Gaudi::Property<double> m_defaultThickness{this, "DefaultThickness", 0.15,
      "Fallback silicon thickness in mm, used when the detector element gives none"};
  /// Replace material that is already attached to a sensitive surface
  Gaudi::Property<bool> m_overwrite{this, "OverwriteExistingMaterial", false,
      "Overwrite surface material already present on sensitive surfaces"};
  /// Silicon parameters (PDG values), exposed for tuning
  Gaudi::Property<double> m_x0{this, "RadiationLength", 93.70,
      "Silicon radiation length X0 in mm"};
  Gaudi::Property<double> m_l0{this, "InteractionLength", 465.2,
      "Silicon nuclear interaction length L0 in mm"};
  Gaudi::Property<double> m_ar{this, "AtomicMass", 28.0855,
      "Silicon relative atomic mass"};
  Gaudi::Property<double> m_z{this, "AtomicNumber", 14.,
      "Silicon atomic number"};
  Gaudi::Property<double> m_rho{this, "MassDensity", 2.329,
      "Silicon mass density in g/cm3"};

  std::optional<Acts::Material> m_silicon{};

  std::size_t m_nSensitive{0};
  std::size_t m_nAssigned{0};
  std::size_t m_nSiElement{0};
  std::size_t m_nOtherSolidState{0};
  std::size_t m_nSkipped{0};
  std::size_t m_nReplacedEmpty{0};
  std::size_t m_nNonSiSkipped{0};
  std::size_t m_nFallbackThickness{0};
};

}  // namespace ISF

#endif