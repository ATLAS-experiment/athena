/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceStripSPFormationAlg.h"

#include <exception>

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

namespace ActsTrk {
StatusCode DeviceStripSPFormationAlg::initialize() {
  ATH_CHECK(m_provider.retrieve());
  ATH_CHECK(m_measurements.initialize());
  ATH_CHECK(m_standard.initialize());
  ATH_CHECK(m_overlap.initialize());
  ATH_CHECK(m_shapes.initialize(m_retainShapes));
  ATH_CHECK(m_overlapShapes.initialize(m_retainShapes));
  ATH_CHECK(m_inputs.initialize());
  ATH_CHECK(m_beamSpot.initialize());
  return StatusCode::SUCCESS;
}

StatusCode DeviceStripSPFormationAlg::execute(const EventContext& ctx) const {
  auto measurements = SG::makeHandle(m_measurements, ctx);
  ATH_CHECK(measurements.isValid());
  SG::ReadCondHandle<StripSpacepointConditions> inputs(m_inputs, ctx);
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpot(m_beamSpot, ctx);
  ATH_CHECK(inputs.isValid());
  ATH_CHECK(beamSpot.isValid());
  try {
    auto algorithm = m_provider->getAlgorithm(ctx);
    const auto& copy = algorithm.copy();
    const auto& vertex = beamSpot->beamVtx().position();
    const traccc::point3 beam{static_cast<traccc::scalar>(vertex.x()),
                              static_cast<traccc::scalar>(vertex.y()),
                              static_cast<traccc::scalar>(vertex.z())};
    auto output = (*algorithm)(*measurements, inputs->surfaces, inputs->rules,
                               beam, m_retainShapes.value());
    // Formation/gather finish before returning fixed-size output buffers.
    ATH_MSG_DEBUG("Strip SP: "
                  << copy.get_size(output.spacepoints) << " standard, "
                  << copy.get_size(output.overlap_spacepoints) << " overlap");
    // Do not rely on debug message evaluation for synchronization.
    (void)copy.get_size(output.spacepoints);
    (void)copy.get_size(output.overlap_spacepoints);
    if (m_retainShapes) {
      auto shapes = SG::makeHandle(m_shapes, ctx);
      auto overlapShapes = SG::makeHandle(m_overlapShapes, ctx);
      ATH_CHECK(shapes.record(
          std::make_unique<
              traccc::strip_spacepoint_shape_collection_types::buffer>(
              std::move(output.shapes))));
      ATH_CHECK(overlapShapes.record(
          std::make_unique<
              traccc::strip_spacepoint_shape_collection_types::buffer>(
              std::move(output.overlap_shapes))));
    }
    auto standard = SG::makeHandle(m_standard, ctx);
    auto overlap = SG::makeHandle(m_overlap, ctx);
    ATH_CHECK(standard.record(
        std::make_unique<traccc::edm::spacepoint_collection::buffer>(
            std::move(output.spacepoints))));
    ATH_CHECK(overlap.record(
        std::make_unique<traccc::edm::spacepoint_collection::buffer>(
            std::move(output.overlap_spacepoints))));
  } catch (const std::exception& error) {
    ATH_MSG_ERROR("GPU Strip SP formation failed: " << error.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
}  // namespace ActsTrk
