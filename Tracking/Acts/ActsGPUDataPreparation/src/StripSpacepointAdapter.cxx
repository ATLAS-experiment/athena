/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StripSpacepointAdapter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "ActsGPUEvent/GeometryIdMapping.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"

namespace ActsTrk {
namespace {
// Athena-only staging data: design and neighbour conventions never cross the
// traccc input boundary. Keep cut preparation independent of GPU execution.
constexpr unsigned int max_strip_rows = 4u;
struct athena_strip_surface_info {
  /// Surface link identifying this record. Records are sorted by this value.
  std::uint64_t surface_link;
  /// Whether this entry corresponds to an endcap strip surface.
  unsigned int is_endcap;
  /// Spacepoint covariance terms used by the seed doublet/triplet tests.
  traccc::scalar spacepoint_variance_r;
  traccc::scalar spacepoint_variance_z;
  /// Minimum radius from the endcap annulus boundary.
  double min_r;
  /// Maximum radius from the endcap annulus boundary.
  double max_r;
  /// Whether the Athena detector element uses an annulus design.
  unsigned int is_annulus;
  /// Surface frame used for endpoint reconstruction.
  std::array<double, 3> origin;
  std::array<double, 3> eta_axis;
  std::array<double, 3> phi_axis;
  /// Transform data used for the strip-gap correction.
  std::array<double, 3> transform_local_x;
  std::array<double, 3> transform_normal;
  std::array<double, 3> transform_translation;
  std::array<double, 3> element_center;
  /// StripBoxDesign parameters used by cellIdOfPosition()/endsOfStrip().
  unsigned int barrel_n_rows;
  unsigned int barrel_n_strips;
  double barrel_pitch;
  double barrel_row_length;
  /// Row-0 parameters used by the radial endpoint and strip-index mappings.
  unsigned int endcap_n_strips;
  double endcap_phi_pitch;
  double endcap_stereo_angle;
  double endcap_wafer_center_r;
  /// Cached constants used verbatim by StripStereoAnnulusDesign.
  double endcap_phi_width;
  double endcap_length_bf;
  double endcap_sin_stereo;
  double endcap_cos_stereo;
  /// Whether the endcap surface uses polar local coordinates.
  unsigned int endcap_uses_polar_coordinates;
  /// Signed barrel/endcap identifier (-2, 0, or +2).
  int barrel_ec;
  /// Whether this surface is the non-stereo reference side.
  unsigned int is_reference_surface;
  /// Detray surface links of the offline module relationships.
  std::uint64_t opposite_surface_link;
  std::uint64_t eta_minus_surface_link;
  std::uint64_t eta_plus_surface_link;
  std::uint64_t phi_minus_surface_link;
  std::uint64_t phi_plus_surface_link;
  /// Final offline compatibility ranges for opposite and eta neighbours.
  traccc::scalar opposite_min;
  traccc::scalar opposite_max;
  traccc::scalar eta_minus_min;
  traccc::scalar eta_minus_max;
  traccc::scalar eta_plus_min;
  traccc::scalar eta_plus_max;
  /// Final offline edge ranges for the reference and phi neighbour.
  traccc::scalar phi_minus_reference_min;
  traccc::scalar phi_minus_reference_max;
  traccc::scalar phi_minus_candidate_min;
  traccc::scalar phi_minus_candidate_max;
  traccc::scalar phi_plus_reference_min;
  traccc::scalar phi_plus_reference_max;
  traccc::scalar phi_plus_candidate_min;
  traccc::scalar phi_plus_candidate_max;
};

/// Reproduce the ACTS CPU StripSpacePointFormationTool::offset calculation
/// using the exact transform columns and element center cached on the host.
traccc::scalar strip_length_gap_tolerance(
    const athena_strip_surface_info& first_info,
    const athena_strip_surface_info& second_info,
    const traccc::scalar strip_gap_parameter = 0.0015f) {

  if (strip_gap_parameter == 0.f) {
    return 0.f;
  }

  // Relative local-x orientation and displacement along the reference normal
  // reproduce the CPU tool's inputs to the strip-gap correction. Keep these
  // host calculations in double precision until the final Traccc input cast.
  const double x12 =
      first_info.transform_local_x[0] * second_info.transform_local_x[0] +
      first_info.transform_local_x[1] * second_info.transform_local_x[1] +
      first_info.transform_local_x[2] * second_info.transform_local_x[2];
  const double dx = first_info.transform_translation[0] -
                    second_info.transform_translation[0];
  const double dy = first_info.transform_translation[1] -
                    second_info.transform_translation[1];
  const double dz = first_info.transform_translation[2] -
                    second_info.transform_translation[2];
  const double surface_separation = dx * first_info.transform_normal[0] +
                                    dy * first_info.transform_normal[1] +
                                    dz * first_info.transform_normal[2];
  // The CPU offset uses the element centre for Annulus designs and the
  // transform origin otherwise; these positions need not coincide.
  const double radius_x = first_info.is_annulus != 0u
                              ? first_info.element_center[0]
                              : first_info.transform_translation[0];
  const double radius_y = first_info.is_annulus != 0u
                              ? first_info.element_center[1]
                              : first_info.transform_translation[1];
  const double surface_reference_r =
      std::sqrt(radius_x * radius_x + radius_y * radius_y);
  const double dm = static_cast<double>(strip_gap_parameter) *
                    surface_reference_r * std::abs(surface_separation * x12);

  // Convert the transverse gap allowance into a strip-length allowance.
  // Preserve the CPU tool's Annulus factor (0.04) and its relative-axis
  // expression for other designs rather than introducing a new cut here.
  double tolerance = 0.;
  if (first_info.is_annulus != 0u) {
    tolerance = dm / 0.04;
  } else {
    const double denominator2 = (1. - x12) * (1. + x12);
    if (denominator2 > 0.) {
      tolerance = dm / std::sqrt(denominator2);
    }
  }

  // Apply the CPU tool's extra radial/longitudinal projection for surfaces
  // whose normal is predominantly along z. Avoid division at z = 0.
  if ((std::abs(first_info.transform_normal[2]) > 0.7) &&
      (std::abs(first_info.transform_translation[2]) > 0.)) {
    tolerance *=
        surface_reference_r / std::abs(first_info.transform_translation[2]);
  }
  return static_cast<traccc::scalar>(tolerance);
}

}  // namespace

StripSpacepointInputs makeStripSpacepointInputs(
    const InDetDD::SiDetectorElementCollection& elements,
    const SCT_ID& strip_id, const InDet::SiElementPropertiesTable& properties,
    const GeometryIdMapping& mapping, vecmem::memory_resource& mr) {
  // Build host-only staging records first, then translate them into the
  // surface records and directed pairing rules consumed by Traccc. Athena
  // design objects and neighbour lookups stay on the host.
  StripSpacepointInputs result(mr);
  std::vector<athena_strip_surface_info> legacy_infos;
  // Express Athena element relationships in the same Detray surface IDs as
  // the measurements. Null neighbours cannot contribute a pair; an unmapped
  // element is an error.
  const auto detray_surface_link =
      [&](const InDetDD::SiDetectorElement* element) {
        if (element == nullptr) {
          return traccc::invalid_strip_surface_link;
        }
        const auto link = mapping.athenaToDetray(element->identify());
        if (!link) {
          throw std::runtime_error(
              "Missing Detray mapping for strip element " +
              std::to_string(element->identify().get_compact()));
        }
        return *link;
      };

  // Widen the CPU pairing-coordinate windows to account for the separation
  // and relative orientation of two surfaces. This is the transverse offset;
  // strip_length_gap_tolerance converts it for the later intersection test.
  const auto strip_range_offset =
      [](const InDetDD::SiDetectorElement* reference,
         const InDetDD::SiDetectorElement* candidate) {
        constexpr double strip_gap_parameter = 0.0015;
        const Amg::Transform3D& first = reference->transform();
        const Amg::Transform3D& second = candidate->transform();
        const Amg::Vector3D center = reference->center();
        const bool is_annulus = reference->isEndcap();
        const double x12 = first(0, 0) * second(0, 0) +
                           first(1, 0) * second(1, 0) +
                           first(2, 0) * second(2, 0);
        const double radius = is_annulus ? std::hypot(center.x(), center.y())
                                         : std::hypot(first(0, 3), first(1, 3));
        const double separation = (first(0, 3) - second(0, 3)) * first(0, 2) +
                                  (first(1, 3) - second(1, 3)) * first(1, 2) +
                                  (first(2, 3) - second(2, 3)) * first(2, 2);
        return strip_gap_parameter * radius * std::abs(separation * x12);
      };

  // Reproduce the CPU correctPolarRange convention for endcap windows:
  // resolve edge strip IDs at a representative row-0 radius, then express
  // the coordinate bounds as angles. Phi-overlap rules use the strip IDs;
  // opposite-side rules use the angular bounds.
  const auto correct_polar_range = [](const InDetDD::SiDetectorElement* element,
                                      double& min_value, double& max_value,
                                      std::size_t& min_strip,
                                      std::size_t& max_strip) {
    if ((element == nullptr) || element->isBarrel()) {
      return;
    }
    const auto* design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(
        &element->design());
    if ((design == nullptr) || (design->diodesInRow(0) == 0)) {
      return;
    }
    const auto first_position =
        0.5 * (design->localPositionOfCell(design->strip1Dim(0, 0)) +
               design->localPositionOfCell(
                   design->strip1Dim(design->diodesInRow(0) - 1, 0)));
    const double radius = first_position.xEta();
    InDetDD::SiCellId min_cell = element->cellIdOfPosition(
        InDetDD::SiLocalPosition(radius, min_value, 0.));
    InDetDD::SiCellId max_cell = element->cellIdOfPosition(
        InDetDD::SiLocalPosition(radius, max_value, 0.));
    // Match the reference fallback when a window edge lies outside the
    // active region: use the first/last strip of the row.
    if (!min_cell.isValid()) {
      min_cell = InDetDD::SiCellId(0);
    }
    if (!max_cell.isValid()) {
      max_cell = InDetDD::SiCellId(design->diodesInRow(0) - 1);
    }
    min_strip = min_cell.strip();
    max_strip = max_cell.strip();
    min_value = std::atan2(min_value, radius);
    max_value = std::atan2(max_value, radius);
  };

  // Prepare the CPU tool's opposite, eta-overlap and phi-overlap windows.
  // Only the non-stereo side initiates pairs, avoiding a reverse duplicate.
  const auto fill_offline_relationships =
      [&](athena_strip_surface_info& info,
          const InDetDD::SiDetectorElement* reference,
          const IdentifierHash reference_hash) {
        if ((reference == nullptr) || reference->isStereo()) {
          return;
        }
        info.is_reference_surface = 1u;
        // These window sizes follow the reference CPU pairing cuts (mm
        // before any endcap conversion), not new GPU-specific selections.
        constexpr double opposite_limit = 2.8;
        constexpr double phi_overlap = 5.64;
        constexpr double eta_min = 1.68;
        constexpr double eta_max = 3.0;
        const double half_width = properties.halfWidth(reference_hash);
        const auto candidate_from_hash = [&](const IdentifierHash hash) {
          return elements.getDetectorElement(hash);
        };
        const auto make_range =
            [&](const InDetDD::SiDetectorElement* candidate,
                const InDetDD::SiDetectorElement* polar_element,
                double min_value, double max_value, traccc::scalar& output_min,
                traccc::scalar& output_max, bool use_strip_index) {
              const double offset = strip_range_offset(reference, candidate);
              min_value -= offset;
              max_value += offset;
              std::size_t min_strip = 0u;
              std::size_t max_strip = 0u;
              correct_polar_range(polar_element, min_value, max_value,
                                  min_strip, max_strip);
              output_min = static_cast<traccc::scalar>(
                  use_strip_index ? min_strip : min_value);
              output_max = static_cast<traccc::scalar>(
                  use_strip_index ? max_strip : max_value);
            };

        // The other side of the module supplies the standard SP partner.
        // Overlap partners are neighbours of that opposite side.
        IdentifierHash opposite_hash;
        if (strip_id.get_other_side(reference_hash, opposite_hash) != 0) {
          return;
        }
        const InDetDD::SiDetectorElement* opposite =
            candidate_from_hash(opposite_hash);
        const std::uint64_t opposite_link = detray_surface_link(opposite);
        if (opposite_link == traccc::invalid_strip_surface_link) {
          return;
        }
        info.opposite_surface_link = opposite_link;
        make_range(opposite, reference, -opposite_limit, opposite_limit,
                   info.opposite_min, info.opposite_max, false);

        // Phi overlaps require separate edge windows on the reference and
        // candidate surfaces. Endcap windows are stored as strip-index cuts.
        const auto fill_phi_neighbour = [&](const IdentifierHash neighbour_hash,
                                            const bool is_minus) {
          const InDetDD::SiDetectorElement* candidate =
              candidate_from_hash(neighbour_hash);
          const std::uint64_t candidate_link = detray_surface_link(candidate);
          if (candidate_link == traccc::invalid_strip_surface_link) {
            return;
          }
          if (is_minus) {
            info.phi_minus_surface_link = candidate_link;
            make_range(candidate, reference, -half_width,
                       -half_width + phi_overlap, info.phi_minus_reference_min,
                       info.phi_minus_reference_max, reference->isEndcap());
            make_range(candidate, candidate, half_width - phi_overlap,
                       half_width, info.phi_minus_candidate_min,
                       info.phi_minus_candidate_max, candidate->isEndcap());
          } else {
            info.phi_plus_surface_link = candidate_link;
            make_range(candidate, reference, half_width - phi_overlap,
                       half_width, info.phi_plus_reference_min,
                       info.phi_plus_reference_max, reference->isEndcap());
            make_range(candidate, candidate, -half_width,
                       -half_width + phi_overlap, info.phi_plus_candidate_min,
                       info.phi_plus_candidate_max, candidate->isEndcap());
          }
        };

        IdentifierHash neighbour_hash;
        if (strip_id.get_prev_in_phi(opposite_hash, neighbour_hash) == 0) {
          fill_phi_neighbour(neighbour_hash, true);
        }
        if (strip_id.get_next_in_phi(opposite_hash, neighbour_hash) == 0) {
          fill_phi_neighbour(neighbour_hash, false);
        }

        // The reference CPU path adds eta-neighbour overlap pairs only for
        // barrel elements; endcap elements retain the phi-neighbour pairs.
        if (reference->isBarrel()) {
          const auto fill_eta_neighbour = [&](const IdentifierHash eta_hash,
                                              const bool is_minus) {
            const InDetDD::SiDetectorElement* candidate =
                candidate_from_hash(eta_hash);
            const std::uint64_t candidate_link = detray_surface_link(candidate);
            if (candidate_link == traccc::invalid_strip_surface_link) {
              return;
            }
            if (is_minus) {
              info.eta_minus_surface_link = candidate_link;
              make_range(candidate, reference, eta_min, eta_max,
                         info.eta_minus_min, info.eta_minus_max, false);
            } else {
              info.eta_plus_surface_link = candidate_link;
              make_range(candidate, reference, eta_min, eta_max,
                         info.eta_plus_min, info.eta_plus_max, false);
            }
          };
          if (strip_id.get_prev_in_eta(opposite_hash, neighbour_hash) == 0) {
            fill_eta_neighbour(neighbour_hash, true);
          }
          if (strip_id.get_next_in_eta(opposite_hash, neighbour_hash) == 0) {
            fill_eta_neighbour(neighbour_hash, false);
          }
        }
      };

  // Collect aligned frames, design parameters and neighbour windows for
  // each mapped strip surface. Zero-initialisation leaves absent neighbours
  // inactive until a valid surface link is assigned below.
  for (auto element_it = elements.begin(); element_it != elements.end();
       ++element_it) {
    const InDetDD::SiDetectorElement* element = *element_it;
    const std::uint64_t surface_link = detray_surface_link(element);
    if ((element == nullptr) ||
        (surface_link == traccc::invalid_strip_surface_link)) {
      continue;
    }

    athena_strip_surface_info info{};
    info.surface_link = surface_link;
    info.opposite_surface_link = traccc::invalid_strip_surface_link;
    info.eta_minus_surface_link = traccc::invalid_strip_surface_link;
    info.eta_plus_surface_link = traccc::invalid_strip_surface_link;
    info.phi_minus_surface_link = traccc::invalid_strip_surface_link;
    info.phi_plus_surface_link = traccc::invalid_strip_surface_link;
    info.is_endcap = element->isEndcap() ? 1u : 0u;
    // Match StripSpacePointFormationTool::makeStripSpacePoint.
    info.spacepoint_variance_r = info.is_endcap != 0u ? 5.12f : 0.1f;
    info.spacepoint_variance_z = info.is_endcap != 0u ? 0.1f : 5.12f;
    info.barrel_ec = strip_id.barrel_ec(element->identify());

    info.is_annulus = 0u;

    // Preserve the exact frame used by
    // SolidStateDetectorElementBase::globalPosition(Vector2D).
    const Amg::Vector3D origin = element->globalPosition(Amg::Vector2D(0., 0.));
    const Amg::Vector3D eta_axis = element->etaAxis();
    const Amg::Vector3D phi_axis = element->phiAxis();
    const Amg::Transform3D& transform = element->transform();
    const Amg::Vector3D element_center = element->center();
    info.origin = {origin.x(), origin.y(), origin.z()};
    info.eta_axis = {eta_axis.x(), eta_axis.y(), eta_axis.z()};
    info.phi_axis = {phi_axis.x(), phi_axis.y(), phi_axis.z()};
    info.transform_local_x = {transform(0, 0), transform(1, 0),
                              transform(2, 0)};
    info.transform_normal = {transform(0, 2), transform(1, 2), transform(2, 2)};
    info.transform_translation = {transform(0, 3), transform(1, 3),
                                  transform(2, 3)};
    info.element_center = {element_center.x(), element_center.y(),
                           element_center.z()};

    // Barrel strips use straight segments with a length and linear pitch;
    // endcap strips require the annulus row layout and stereo-frame geometry.
    if (element->isBarrel()) {
      const auto* design = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(
          &element->design());
      if ((design == nullptr) || (design->diodesInRow(0) <= 0)) {
        throw std::runtime_error("Unsupported strip design for surface " +
                                 std::to_string(surface_link));
      }
      info.barrel_n_strips = static_cast<unsigned int>(design->diodesInRow(0));
      info.barrel_n_rows =
          static_cast<unsigned int>(design->diodes()) / info.barrel_n_strips;
      info.barrel_pitch = design->stripPitch();
      info.barrel_row_length =
          design->length() / static_cast<double>(info.barrel_n_rows);
    } else {
      const auto* design =
          dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(
              &element->design());
      if ((design == nullptr) || (design->diodesInRow(0) <= 0)) {
        throw std::runtime_error("Unsupported strip design for surface " +
                                 std::to_string(surface_link));
      }
      // Check the complete supported row layout, but retain only the row-0
      // parameters used by the existing Traccc endpoint/index convention.
      const unsigned int total_diodes =
          static_cast<unsigned int>(design->diodes());
      unsigned int first_strip = 0u;
      for (unsigned int row = 0u;
           (row < max_strip_rows) && (first_strip < total_diodes); ++row) {
        const unsigned int n_strips =
            static_cast<unsigned int>(design->diodesInRow(row));
        if (n_strips == 0u) {
          break;
        }
        if (row == 0u) {
          info.endcap_n_strips = n_strips;
          info.endcap_phi_pitch =
              design->phiPitchPhi(InDetDD::SiCellId(0, row));
        }
        first_strip += n_strips;
      }
      if (first_strip != total_diodes) {
        throw std::runtime_error(
            "Unsupported or incomplete annulus row layout for "
            "surface " +
            std::to_string(surface_link) + ": mapped " +
            std::to_string(first_strip) + " of " +
            std::to_string(total_diodes) + " strips");
      }
      info.min_r = design->minR();
      info.max_r = design->maxR();
      const double stereo_angle = design->stereo();
      const double wafer_center_r = design->waferCentreR();
      info.endcap_stereo_angle = stereo_angle;
      info.endcap_wafer_center_r = wafer_center_r;
      info.endcap_phi_width = design->phiWidth();
      // Cache the constants used by the Athena strip-frame transformation,
      // including whether the local coordinates are polar or Cartesian.
      info.endcap_length_bf =
          2. * wafer_center_r * std::sin(0.5 * stereo_angle);
      info.endcap_sin_stereo = std::sin(stereo_angle);
      info.endcap_cos_stereo = std::cos(stereo_angle);
      info.endcap_uses_polar_coordinates =
          design->shape() == InDetDD::PolarAnnulus ? 1u : 0u;
      // StripSpacePointFormationTool::offset() uses the
      // Annulus shape specifically, not every end-cap design.
      info.is_annulus = design->shape() == InDetDD::Annulus ? 1u : 0u;
    }

    const IdentifierHash module_hash = strip_id.wafer_hash(element->identify());
    fill_offline_relationships(info, element, module_hash);
    legacy_infos.push_back(info);
  }

  // Traccc looks up surfaces by ID. Sorting also permits neighbour lookup
  // by lower_bound; duplicate IDs would make that lookup ambiguous.
  std::sort(legacy_infos.begin(), legacy_infos.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.surface_link < rhs.surface_link;
            });
  const auto duplicate =
      std::adjacent_find(legacy_infos.begin(), legacy_infos.end(),
                         [](const auto& lhs, const auto& rhs) {
                           return lhs.surface_link == rhs.surface_link;
                         });
  if (duplicate != legacy_infos.end()) {
    throw std::runtime_error("Duplicate strip surface link in static geometry");
  }

  // Translate the staging records into Traccc's scalar geometry inputs.
  // Keep the existing casts and strip-index conventions unchanged.
  result.surface_infos.reserve(legacy_infos.size());
  result.pairing_rules.reserve(legacy_infos.size() * 3u);
  for (const auto& source : legacy_infos) {
    traccc::strip_measurement_surface_info info{};
    info.surface_link = source.surface_link;
    for (std::size_t axis = 0; axis < 3u; ++axis) {
      info.origin[axis] = static_cast<traccc::scalar>(source.origin[axis]);
      info.local_u[axis] = static_cast<traccc::scalar>(source.eta_axis[axis]);
      info.local_v[axis] = static_cast<traccc::scalar>(source.phi_axis[axis]);
    }
    info.spacepoint_variance_r = source.spacepoint_variance_r;
    info.spacepoint_variance_z = source.spacepoint_variance_z;
    if (source.is_endcap == 0u) {
      info.geometry_model = traccc::strip_geometry_model::linear;
      info.linear = {source.barrel_n_rows, source.barrel_n_strips,
                     static_cast<traccc::scalar>(source.barrel_pitch),
                     static_cast<traccc::scalar>(source.barrel_row_length), 0.};
      // Barrel measurements use local0 and a positive channel direction.
      info.index_mapping = {
          0u,
          source.barrel_n_strips,
          static_cast<traccc::scalar>(source.barrel_pitch),
          1.,
          static_cast<traccc::scalar>(source.barrel_n_strips) * 0.5f,
          false};
    } else {
      // Preserve the validated row-0 getStripEnds convention. Selection
      // of an additional row/design belongs to this adapter.
      info.geometry_model = traccc::strip_geometry_model::radial;
      info.radial = {source.endcap_n_strips,
                     static_cast<traccc::scalar>(source.endcap_phi_pitch),
                     static_cast<traccc::scalar>(source.min_r),
                     static_cast<traccc::scalar>(source.max_r),
                     static_cast<traccc::scalar>(source.endcap_stereo_angle),
                     static_cast<traccc::scalar>(source.endcap_wafer_center_r),
                     static_cast<traccc::scalar>(source.endcap_length_bf),
                     static_cast<traccc::scalar>(source.endcap_sin_stereo),
                     static_cast<traccc::scalar>(source.endcap_cos_stereo),
                     source.endcap_uses_polar_coordinates != 0u
                         ? traccc::strip_local_frame::polar
                         : traccc::strip_local_frame::cartesian};
      // Endcap measurements use local1 (angle), reverse the strip direction
      // and apply the CPU row-0 offset. The mapping pitch is phiWidth/count.
      info.index_mapping = {
          1u,
          source.endcap_n_strips,
          static_cast<traccc::scalar>(
              source.endcap_phi_width /
              static_cast<double>(source.endcap_n_strips)),
          -1.,
          static_cast<traccc::scalar>(source.endcap_n_strips) * 0.5f - 0.5f,
          true};
    }
    result.surface_infos.push_back(info);

    if (source.is_reference_surface == 0u) {
      continue;
    }
    // Emit at most one directed rule per reference/candidate relationship.
    // Entry 0 is the standard partner; eta and phi neighbours are overlaps.
    const std::array<std::uint64_t, 5> links{
        source.opposite_surface_link, source.eta_minus_surface_link,
        source.eta_plus_surface_link, source.phi_minus_surface_link,
        source.phi_plus_surface_link};
    for (std::size_t i = 0; i < links.size(); ++i) {
      const auto link = links[i];
      // Preserve the original first-matching relationship precedence.
      if (link == traccc::invalid_strip_surface_link ||
          std::find(links.begin(), links.begin() + i, link) !=
              links.begin() + i) {
        continue;
      }
      const auto candidate =
          std::lower_bound(legacy_infos.begin(), legacy_infos.end(), link,
                           [](const auto& item, std::uint64_t value) {
                             return item.surface_link < value;
                           });
      if (candidate == legacy_infos.end() || candidate->surface_link != link ||
          candidate->is_endcap != source.is_endcap) {
        continue;
      }
      traccc::strip_pairing_rule rule{};
      rule.reference_surface_link = source.surface_link;
      rule.candidate_surface_link = link;
      rule.category = i == 0u ? traccc::strip_pair_category::standard
                              : traccc::strip_pair_category::overlap;
      rule.strip_length_gap_tolerance =
          strip_length_gap_tolerance(source, *candidate);
      rule.strip_length_tolerance = static_cast<traccc::scalar>(0.01f);
      // Opposite/eta partners use a coordinate-difference cut. Phi partners
      // instead require both clusters to lie in their respective edge windows.
      if (i < 3u) {
        rule.mode = traccc::strip_pairing_mode::difference;
        rule.reference_coordinate = rule.candidate_coordinate =
            source.is_endcap != 0u ? traccc::strip_pairing_coordinate::local1
                                   : traccc::strip_pairing_coordinate::local0;
        rule.difference_scale = candidate->barrel_ec < 0 ? -1.f : 1.f;
        const traccc::scalar minima[]{source.opposite_min, source.eta_minus_min,
                                      source.eta_plus_min};
        const traccc::scalar maxima[]{source.opposite_max, source.eta_minus_max,
                                      source.eta_plus_max};
        rule.difference_min = minima[i];
        rule.difference_max = maxima[i];
      } else {
        rule.mode = traccc::strip_pairing_mode::windows;
        rule.reference_coordinate = rule.candidate_coordinate =
            source.is_endcap != 0u
                ? traccc::strip_pairing_coordinate::strip_index
                : traccc::strip_pairing_coordinate::local0;
        const bool minus = i == 3u;
        rule.reference_min = minus ? source.phi_minus_reference_min
                                   : source.phi_plus_reference_min;
        rule.reference_max = minus ? source.phi_minus_reference_max
                                   : source.phi_plus_reference_max;
        rule.candidate_min = minus ? source.phi_minus_candidate_min
                                   : source.phi_plus_candidate_min;
        rule.candidate_max = minus ? source.phi_minus_candidate_max
                                   : source.phi_plus_candidate_max;
      }
      result.pairing_rules.push_back(rule);
    }
  }
  // Group rules by reference/candidate ID for Traccc's surface-pair lookup.
  std::sort(
      result.pairing_rules.begin(), result.pairing_rules.end(),
      [](const auto& a, const auto& b) {
        return std::tie(a.reference_surface_link, a.candidate_surface_link) <
               std::tie(b.reference_surface_link, b.candidate_surface_link);
      });
  return result;
}

}  // namespace ActsTrk
