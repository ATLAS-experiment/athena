/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TritonTracccTrackMaker.h"

#include "TracccTritonClient/TracccEdmSerialization.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <vecmem/memory/host_memory_resource.hpp>
#include <vecmem/utils/copy.hpp>

#include <chrono>
#include <exception>
#include <memory>

StatusCode TritonTracccTrackMaker::initialize()
{
    ATH_CHECK(m_tracccCellsKey.initialize());
    ATH_CHECK(m_measurementsKey.initialize());
    ATH_CHECK(m_clustersKey.initialize());
    ATH_CHECK(m_tracksKey.initialize());

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_tracccTrackingTool.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode TritonTracccTrackMaker::execute(const EventContext& ctx) const
{
    // The recorded buffers come from the (caching, shared) host memory
    // resource; the temporary host containers use a plain one, since the
    // jagged cluster collection alone is ~10^5 small allocations
    vecmem::memory_resource& mr = m_hostMR->mr();
    vecmem::host_memory_resource tmp_mr;
    // everything here lives in host memory
    vecmem::copy copy;

    // Serialize the cells produced by RDOtoTracccCellConverterAlg
    auto cells_handle = SG::makeHandle(m_tracccCellsKey, ctx);
    ATH_CHECK(cells_handle.isValid());

    traccc::edm::silicon_cell_collection::host cells{tmp_mr};
    copy(*cells_handle, cells)->wait();

    std::vector<uint8_t> cells_buffer;
    TracccTriton::serialize(cells, cells_buffer);
    ATH_MSG_DEBUG("Serialized " << cells.size() << " traccc cells into "
                  << cells_buffer.size() << " bytes");

    // Run the reconstruction on the server
    auto traccc_start = std::chrono::high_resolution_clock::now();
    TracccTritonResult result;
    ATH_CHECK(m_tracccTrackingTool->getTracks(cells_buffer, result));
    auto traccc_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> traccc_time = traccc_end - traccc_start;
    ATH_MSG_INFO("Traccc total inference time: " << traccc_time.count() << " ms");

    // Deserialize what came back
    traccc::edm::measurement_collection::host measurements{tmp_mr};
    traccc::edm::silicon_cluster_collection::host clusters{tmp_mr};
    traccc_track_container::host tracks{tmp_mr};
    TracccTriton::deserialize(result.measurements.data(),
                                result.measurements.size(), measurements);
    TracccTriton::deserialize(result.clusters.data(),
                                result.clusters.size(), clusters);
    TracccTriton::deserialize(result.tracks.data(), result.tracks.size(),
                                tracks.tracks);
    TracccTriton::deserialize(result.trackStates.data(),
                                result.trackStates.size(), tracks.states);
    ATH_MSG_DEBUG("Received " << measurements.size() << " measurements, "
                  << clusters.size() << " clusters, " << tracks.tracks.size()
                  << " tracks and " << tracks.states.size() << " track states");

    // Record output as host buffers
    constexpr auto h2h = vecmem::copy::type::host_to_host;

    auto measurementsHandle = SG::makeHandle(m_measurementsKey, ctx);
    ATH_CHECK(measurementsHandle.record(
        std::make_unique<traccc::edm::measurement_collection::buffer>(
            copy.to(vecmem::get_data(measurements), mr, nullptr, h2h))));

    auto clustersHandle = SG::makeHandle(m_clustersKey, ctx);
    ATH_CHECK(clustersHandle.record(
        std::make_unique<traccc::edm::silicon_cluster_collection::buffer>(
            copy.to(vecmem::get_data(clusters), mr, &mr, h2h))));

    auto tracksBuffer = std::make_unique<traccc_track_container::buffer>();
    tracksBuffer->tracks = copy.to(vecmem::get_data(tracks.tracks), mr, &mr, h2h);
    tracksBuffer->states = copy.to(vecmem::get_data(tracks.states), mr, nullptr, h2h);
    tracksBuffer->measurements = *measurementsHandle;
    auto tracksHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(tracksHandle.record(std::move(tracksBuffer)));

    return StatusCode::SUCCESS;
}
