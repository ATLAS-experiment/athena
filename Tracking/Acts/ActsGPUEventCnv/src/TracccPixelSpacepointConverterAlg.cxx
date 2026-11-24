/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccPixelSpacepointConverterAlg.h"

#include <cmath>
#include <exception>
#include <memory>

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
namespace ActsTrk {
StatusCode TracccPixelSpacepointConverterAlg::initialize() {
  ATH_CHECK(m_copy.retrieve());
  ATH_CHECK(m_hostMR.retrieve());
  ATH_CHECK(m_input.initialize());
  ATH_CHECK(m_clusters.initialize());
  ATH_CHECK(m_mapping.initialize());
  ATH_CHECK(m_output.initialize());
  return StatusCode::SUCCESS;
}
StatusCode TracccPixelSpacepointConverterAlg::execute(const EventContext& ctx) const {
  // Read the GPU SP buffer and the host cluster associations for this event.
  auto input = SG::makeHandle(m_input, ctx);
  auto clusters = SG::makeHandle(m_clusters, ctx);
  auto mapping = SG::makeHandle(m_mapping, ctx);
  ATH_CHECK(input.isValid());
  ATH_CHECK(clusters.isValid());
  ATH_CHECK(mapping.isValid());
  try {
    // Copy the GPU SPs to host memory and wait before reading their values.
    const auto copy = m_copy->copy(ctx);
    traccc::edm::spacepoint_collection::buffer host(copy->get_size(*input), m_hostMR->mr());
    copy->setup(host)->wait();
    (*copy)(*input, host)->wait();
    const traccc::edm::spacepoint_collection::const_device points(host);
    // Build xAOD SPs from the copied positions/variances and linked clusters.
    auto output = std::make_unique<xAOD::SpacePointContainer>();
    auto aux = std::make_unique<xAOD::SpacePointAuxContainer>();
    output->setStore(aux.get());
    output->reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
      const auto sp = points.at(i);
      // Reject invalid associations or values rather than silently dropping SPs.
      const auto measurement = sp.measurement_index_1();
      if (measurement >= mapping->size() || mapping->at(measurement) >= clusters->size()) {
        ATH_MSG_ERROR("GPU Pixel SP measurement does not map to a Pixel cluster");
        return StatusCode::FAILURE;
      }
      const auto* cluster = clusters->at(mapping->at(measurement));
      const Eigen::Vector3f position(sp.x(), sp.y(), sp.z());
      if (!position.allFinite() || !std::isfinite(sp.radius_variance()) || !std::isfinite(sp.z_variance()) ||
          sp.radius_variance() < 0 || sp.z_variance() < 0) {
        ATH_MSG_ERROR("Non-finite or invalid GPU Pixel SP position/variance");
        return StatusCode::FAILURE;
      }
      auto point = std::make_unique<xAOD::SpacePoint>();
      output->push_back(point.release());
      output->back()->setSpacePoint(cluster->identifierHash(), position, sp.radius_variance(), sp.z_variance(),
                                    {cluster});
    }
    ATH_MSG_DEBUG("Converted " << output->size() << " GPU pixel spacepoints to " << m_output.key());
    // Publish the container and its auxiliary store only after conversion succeeds.
    auto handle = SG::makeHandle(m_output, ctx);
    ATH_CHECK(handle.record(std::move(output), std::move(aux)));
  } catch (const std::exception& error) {
    ATH_MSG_ERROR("GPU Pixel SP conversion failed: " << error.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
