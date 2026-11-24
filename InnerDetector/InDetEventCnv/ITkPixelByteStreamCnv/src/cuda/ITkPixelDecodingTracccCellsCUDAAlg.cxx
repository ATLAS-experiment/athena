/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingTracccCellsCUDAAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/WriteHandle.h"
#include "RD53CDecodingCUDA.h"

#include <GaudiKernel/StatusCode.h>
#include <cuda_runtime.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vecmem/containers/data/jagged_vector_buffer.hpp>
#include <vecmem/containers/data/vector_buffer.hpp>
#include <vecmem/containers/vector.hpp>
#include <vecmem/utils/copy.hpp>
#include <vector>

StatusCode ITkPixelDecodingTracccCellsCUDAAlg::initialize()
{
    ATH_CHECK(m_decodingTool.retrieve());
    ATH_CHECK(detStore()->retrieve(m_geoIdMapping, m_geoIdMappingObjectName.value()));
    ATH_CHECK(m_tracccCellsKey.initialize());

    ATH_CHECK(m_MRs.retrieve());
    ATH_CHECK(m_copies.retrieve());
    ATH_CHECK(m_streamTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingTracccCellsCUDAAlg::execute(const EventContext& ctx) const
{
    auto& hostMR = *m_MRs->hostMR();
    std::vector<uint64_t> bytestream_data;
    ITkPixelDecodingCUDA::StreamInfoContainer::host streams_info(hostMR);
    try {
        ATH_CHECK(m_decodingTool->aggregateData(ctx, bytestream_data, streams_info,
            std::function<StatusCode (uint32_t, uint32_t&)>{
                [this](uint32_t module_id, uint32_t& detray_index) {
                    return moduleIdToDetrayIndex(module_id, detray_index);
                }}));
    } catch (std::runtime_error const & exc) {
        ATH_MSG_ERROR("aggregateData error: " << exc.what());
        return StatusCode::FAILURE;
    }

    if (bytestream_data.empty() || streams_info.size() == 0) {
        ATH_MSG_ERROR("input data is empty");
        return StatusCode::FAILURE;
    }

    // Usable bits per 64-bit frame, matching the CPU DecCore configuration.
    // In the CPU implementation, DecCore calculates the frame size internally
    // when core.decode(payload64) creates the MemStreamProvider.
    // Here each CUDA thread decodes its assigned stream using decode_stream(),
    // so we construct MemStreamProvider explicitly and use the same frame-size
    // calculation as DecCore.
    std::unique_ptr<traccc::edm::silicon_cell_collection::buffer> traccc_cells;

    auto device_copy = m_copies->deviceCopy(ctx);
    auto host_copy = m_copies->hostCopy(ctx);
    auto& deviceMR = m_MRs->mainMR();
    cudaStream_t stream = static_cast<cudaStream_t>(m_streamTool->stream(ctx));

    // Copy input data into GPU: copy to pinned memory first, then copy from
    // pinned memory to GPU to allows async copy
    auto input_data_size = static_cast<vecmem::edm::details::size_type>(
        bytestream_data.size());
    vecmem::data::vector_buffer<uint64_t> input_data_buffer_host{
        input_data_size, *m_MRs->hostMR()};
    host_copy->setup(input_data_buffer_host)->ignore();
    (*host_copy)(vecmem::get_data(bytestream_data), input_data_buffer_host)->wait();

    vecmem::data::vector_buffer<uint64_t> input_data_buffer{
        input_data_size, m_MRs->mainMR()};
    device_copy->setup(input_data_buffer)->ignore();
    (*device_copy)(vecmem::ignore_event, input_data_buffer_host, input_data_buffer);

    // Copy streams data to GPU
    ITkPixelDecodingCUDA::StreamInfoContainer::buffer streams_info_buffer(
        streams_info.size(), m_MRs->mainMR());
    device_copy->setup(streams_info_buffer)->ignore();
    (*device_copy)(vecmem::get_data(streams_info), streams_info_buffer)->ignore();

    // Decode all prepared RD53C inputs as one GPU batch.
    // The GPU allocates the output container, performs the parallel decoding,
    // and creates the cells in device memory.
    try {
        ATH_CHECK(ITkPixelDecodingCUDA::decodeRD53C(stream, deviceMR, hostMR,
            *device_copy, input_data_buffer, streams_info_buffer, m_frameSize,
            s_decodingOptions, traccc_cells));
    } catch(std::runtime_error const & exc) {
        ATH_MSG_ERROR(exc.what());
        return StatusCode::FAILURE;
    }

    // Register device buffer containing decoded traccc cells in SG
    const unsigned int nb_cells = traccc_cells->capacity();
    auto traccc_cells_handles = SG::makeHandle(m_tracccCellsKey, ctx);
    ATH_CHECK(traccc_cells_handles.record(std::move(traccc_cells)));

    ATH_MSG_DEBUG("nb cells created: " << nb_cells
        << ", input data size [B]: " << bytestream_data.size() * sizeof(bytestream_data[0]));
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingTracccCellsCUDAAlg::moduleIdToDetrayIndex(
    uint32_t module_id, uint32_t & detray_index) const
{
    const std::optional<size_t> module_index_opt =
        m_geoIdMapping->athenaToDetDescIndex(Identifier(module_id));
    if (!module_index_opt.has_value()) {
        ATH_MSG_ERROR("no module index for " << Identifier(module_id));
        return StatusCode::FAILURE;
    }
    detray_index = module_index_opt.value();
    return StatusCode::SUCCESS;
}
