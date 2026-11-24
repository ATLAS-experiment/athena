/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccStripSpacepointConverterAlg.h"

#include <cmath>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

namespace ActsTrk {
namespace {
// Reconstruct endpoints from linked clusters only when UseGPUShapes is false.
// Follow the CPU StripSpacePointFormationTool convention, using row 0 for
// endcap strips.
std::optional<std::pair<Amg::Vector3D, Amg::Vector3D>> stripEnds(const xAOD::StripCluster& cluster,
                                                                 const InDetDD::SiDetectorElementCollection& elements,
                                                                 std::string& error) {
  const auto* element = elements.getDetectorElement(cluster.identifierHash());
  if (!element) {
    error = "Missing strip detector element";
    return std::nullopt;
  }
  const double local = cluster.localPosition<1>()(0, 0);
  if (!std::isfinite(local)) {
    error = "Non-finite strip position";
    return std::nullopt;
  }
  if (element->isEndcap()) {
    const auto* design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&element->design());
    if (!design) {
      error = "Unsupported endcap strip design";
      return std::nullopt;
    }
    const auto count = design->diodesInRow(0);
    if (count <= 0) {
      error = "Invalid strip count";
      return std::nullopt;
    }
    const double pitch = design->phiWidth() / count;
    if (!(pitch > 0.)) {
      error = "Invalid strip pitch";
      return std::nullopt;
    }
    const double index = -std::floor(local / pitch) + count * 0.5 - 0.5;
    if (index < 0. || index >= count) {
      error = "Strip index outside design";
      return std::nullopt;
    }
    const auto strip = static_cast<int>(index);
    return std::make_pair(element->globalPosition(design->stripPosAtR(strip, 0, design->minR())),
                          element->globalPosition(design->stripPosAtR(strip, 0, design->maxR())));
  }
  return element->endsOfStrip(InDetDD::SiLocalPosition(0., local, 0.));
}
}  // namespace

StatusCode TracccStripSpacepointConverterAlg::initialize() {
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_standard.initialize());
  ATH_CHECK(m_overlap.initialize());
  ATH_CHECK(m_shapes.initialize(m_useGPUShapes));
  ATH_CHECK(m_overlapShapes.initialize(m_useGPUShapes));
  ATH_CHECK(m_clusters.initialize());
  ATH_CHECK(m_mapping.initialize());
  ATH_CHECK(m_elements.initialize(!m_useGPUShapes));
  ATH_CHECK(m_outputStandard.initialize());
  ATH_CHECK(m_outputOverlap.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TracccStripSpacepointConverterAlg::execute(const EventContext& ctx) const {
  // Read standard/overlap GPU SP buffers and their host cluster associations.
  auto standard = SG::makeHandle(m_standard, ctx);
  auto overlap = SG::makeHandle(m_overlap, ctx);
  auto clusters = SG::makeHandle(m_clusters, ctx);
  auto mapping = SG::makeHandle(m_mapping, ctx);

  ATH_CHECK(standard.isValid());
  ATH_CHECK(overlap.isValid());
  ATH_CHECK(clusters.isValid());
  ATH_CHECK(mapping.isValid());
  // Detector elements are needed only for the CPU shape-reconstruction fallback.
  std::unique_ptr<SG::ReadCondHandle<InDetDD::SiDetectorElementCollection>> elements;
  if (!m_useGPUShapes) {
    elements = std::make_unique<SG::ReadCondHandle<InDetDD::SiDetectorElementCollection>>(m_elements, ctx);
    ATH_CHECK(elements->isValid());
  }
  const auto copy = m_copy->copy(ctx);
  struct StripGeometry {
    Amg::Vector3D vector;
    double length;
    Amg::Vector3D center;
  };
  // Reuse CPU-reconstructed cluster geometry between standard and overlap SPs
  // within this event; the cache is not shared between events.
  std::vector<std::optional<StripGeometry>> geometryCache;
  const auto geometryFor = [&](unsigned int index) -> const StripGeometry* {
    auto& geometry = geometryCache[index];
    if (!geometry) {
      std::string error;
      const auto ends = stripEnds(*clusters->at(index), ***elements, error);
      if (!ends) {
        ATH_MSG_ERROR("Cannot reconstruct strip geometry for cluster " << index << ": " << error);
        return nullptr;
      }
      const Amg::Vector3D vector = ends->second - ends->first;
      const double length = vector.norm();
      if (!(length > 0.) || !std::isfinite(length)) {
        ATH_MSG_ERROR("Invalid strip endpoints for cluster " << index);
        return nullptr;
      }
      geometry.emplace(StripGeometry{vector, length, 0.5 * (ends->first + ends->second)});
    }
    return &*geometry;
  };
  const auto convert = [&](const auto& input, const auto& outputKey, const auto& shapeKey) -> StatusCode {
    // Copy SP values to host memory; shape buffers use the same SP ordering.
    traccc::edm::spacepoint_collection::buffer host(copy->get_size(input), m_hostMR->mr());
    copy->setup(host)->wait();
    (*copy)(input, host)->wait();
    const traccc::edm::spacepoint_collection::const_device points(host);
    traccc::strip_spacepoint_shape_collection_types::buffer hostShapes;
    if (m_useGPUShapes) {
      SG::ReadHandle<traccc::strip_spacepoint_shape_collection_types::buffer> shapes(shapeKey, ctx);
      ATH_CHECK(shapes.isValid());
      if (copy->get_size(*shapes) != points.size()) {
        ATH_MSG_ERROR("Shape and SP output sizes differ");
        return StatusCode::FAILURE;
      }
      hostShapes = traccc::strip_spacepoint_shape_collection_types::buffer(points.size(), m_hostMR->mr());
      copy->setup(hostShapes)->wait();
      if (points.size() != 0u) (*copy)(*shapes, hostShapes)->wait();
    }
    const traccc::strip_spacepoint_shape_collection_types::const_device shapes(hostShapes);
    auto output = std::make_unique<xAOD::SpacePointContainer>();
    auto aux = std::make_unique<xAOD::SpacePointAuxContainer>();
    output->setStore(aux.get());
    // Allocate SP elements together so the auxiliary store is resized once.
    output->push_new(points.size(), [] { return new xAOD::SpacePoint; });
    for (std::size_t i = 0; i < points.size(); ++i) {
      const auto sp = points.at(i);
      const auto first = sp.measurement_index_1();
      const auto second = sp.measurement_index_2();
      // Validate both measurements before resolving the two source clusters.
      if (first == second || first >= mapping->size() || second >= mapping->size()) {
        ATH_MSG_ERROR("Invalid strip SP measurement pair at SP " << i);
        return StatusCode::FAILURE;
      }
      const auto a = mapping->at(first);
      const auto b = mapping->at(second);
      if (a == b || a >= clusters->size() || b >= clusters->size()) {
        ATH_MSG_ERROR("Strip SP measurement does not map to a strip cluster at SP " << i);
        return StatusCode::FAILURE;
      }
      const auto* top = clusters->at(a);
      const auto* bottom = clusters->at(b);
      traccc::strip_spacepoint_shape shape{};
      // Reuse retained GPU shapes, or reconstruct each cluster geometry once per event.
      if (m_useGPUShapes) {
        shape = shapes.at(i);
      } else {
        const auto* topGeometry = geometryFor(a);
        if (!topGeometry) return StatusCode::FAILURE;
        const auto* bottomGeometry = geometryFor(b);
        if (!bottomGeometry) return StatusCode::FAILURE;
        shape.top_half_length = 0.5 * topGeometry->length;
        shape.bottom_half_length = 0.5 * bottomGeometry->length;
        const auto topDirection = (topGeometry->vector / topGeometry->length).eval();
        const auto bottomDirection = (bottomGeometry->vector / bottomGeometry->length).eval();
        for (unsigned int k = 0; k < 3u; ++k) {
          shape.top_direction[k] = topDirection[k];
          shape.bottom_direction[k] = bottomDirection[k];
          shape.top_center[k] = topGeometry->center[k];
          shape.center_distance[k] = topGeometry->center[k] - bottomGeometry->center[k];
        }
      }
      const auto toEigen = [](const auto& v) {
        return Eigen::Vector3f{static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2])};
      };
      if (!(shape.top_half_length > 0) || !(shape.bottom_half_length > 0) || !std::isfinite(shape.top_half_length) ||
          !std::isfinite(shape.bottom_half_length) || !toEigen(shape.top_direction).allFinite() ||
          !toEigen(shape.bottom_direction).allFinite() || !toEigen(shape.top_center).allFinite() ||
          !toEigen(shape.center_distance).allFinite()) {
        ATH_MSG_ERROR("Invalid strip shape at SP " << i);
        return StatusCode::FAILURE;
      }
      const Eigen::Vector3f position(sp.x(), sp.y(), sp.z());
      if (!position.allFinite() || !std::isfinite(sp.radius_variance()) || !std::isfinite(sp.z_variance()) ||
          sp.radius_variance() < 0 || sp.z_variance() < 0) {
        ATH_MSG_ERROR("Invalid GPU strip SP position/variance at SP " << i);
        return StatusCode::FAILURE;
      }
      output->at(i)->setSpacePoint(
          std::vector<xAOD::DetectorIDHashType>{top->identifierHash(), bottom->identifierHash()}, position,
          sp.radius_variance(), sp.z_variance(), {top, bottom}, shape.top_half_length, shape.bottom_half_length,
          toEigen(shape.top_direction), toEigen(shape.bottom_direction), toEigen(shape.center_distance),
          toEigen(shape.top_center));
    }
    ATH_MSG_DEBUG("Converted " << output->size() << " strip spacepoints to " << outputKey.key());
    // Publish this container only after all its SPs have passed validation.
    auto handle = SG::makeHandle(outputKey, ctx);
    ATH_CHECK(handle.record(std::move(output), std::move(aux)));
    return StatusCode::SUCCESS;
  };
  try {
    if (!m_useGPUShapes) geometryCache.resize(clusters->size());
    ATH_CHECK(convert(*standard, m_outputStandard, m_shapes));
    ATH_CHECK(convert(*overlap, m_outputOverlap, m_overlapShapes));
  } catch (const std::exception& error) {
    ATH_MSG_ERROR("Strip SP conversion failed: " << error.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
