/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingTracccCellsCUDAAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "Identifier/Identifier.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandle.h"
#include "eformat/ROBFragment.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "RD53CDecodingCUDA.h"

#include <GaudiKernel/StatusCode.h>
#include <cuda_runtime.h>
#include <cstdint>
#include <map>
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
    ATH_CHECK(m_robDataProviderSvc.retrieve());
    ATH_CHECK(m_pixelCablingKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_geoIdMapping, m_geoIdMappingObjectName.value()));
    ATH_CHECK(m_tracccCellsKey.initialize());

    ATH_CHECK(m_MRs.retrieve());
    ATH_CHECK(m_copies.retrieve());
    ATH_CHECK(m_streamTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingTracccCellsCUDAAlg::execute(const EventContext& ctx) const
{
    SG::ReadCondHandle<ITkPixelCablingData> cablingData(m_pixelCablingKey, ctx);
    const ITkPixelCablingData* cabling = *cablingData;
    if (cabling == nullptr) {
        ATH_MSG_ERROR("could not retrieve ITkPixelCablingData");
        return StatusCode::FAILURE;
    }
    
    std::vector<const eformat::ROBFragment<const uint32_t*>*> ROBs;
    m_robDataProviderSvc->getROBData(ctx, cabling->sourceIDs(), ROBs);
    ATH_MSG_DEBUG("retrieved "<< ROBs.size()<< " ROB fragments");

    auto& hostMR = *m_MRs->hostMR();
    std::vector<uint64_t> input_data_64bits;
    input_data_64bits.reserve(m_ExpectedDataSize / 8);
    ITkPixelDecodingCUDA::StreamInfoContainer::host streams_info(hostMR);

    for (const auto* ROB : ROBs) {
        const uint32_t* payload = ROB->rod_data();
        const uint32_t length = ROB->rod_ndata();

        auto comp =[](const uint32_t& a, const uint32_t& b){
                const uint32_t aLSB = a & 0x00FFFFFF;
                const uint32_t bLSB = b & 0x00FFFFFF;
                if (aLSB != bLSB) {return aLSB < bLSB;}
                return a < b;};

        std::map<uint32_t,std::pair<size_t, uint16_t>, decltype(comp)>GBTFragments(comp);

        // total GBT data payload size in this ROB
        size_t rob_data_size_64bits = 0;
        size_t idx = 0;
        while (idx < length){
            const uint16_t GBTFragmentSize =static_cast<uint16_t>((payload[idx] & 0xFFFF0000)>> 16);
            const uint32_t detectorResourceID = payload[idx + 1];
            GBTFragments.insert({detectorResourceID,{idx, GBTFragmentSize}});
            idx += GBTFragmentSize;
            // The frames are always 64 bits split into 32-bit words, so always
            // divisible by 2. The first 32-bit words are the GBT header.
            rob_data_size_64bits += (GBTFragmentSize - 2) / 2;
        }

        // This is where we are going to write
        size_t payload64_idx = input_data_64bits.size();
        // We need to store the start of index each stream
        size_t payload64_start = input_data_64bits.size();
        // Make room for this ROB's data
        input_data_64bits.resize(payload64_idx + rob_data_size_64bits);

        uint32_t current_module_id = 0xFFFFFFFF;
        unsigned int current_module_index = 0;

        for (const auto&[detectorResourceID, range]: GBTFragments){
            const uint32_t module_id = ITkPixelCabling::dridToModuleID(detectorResourceID);
            if (module_id != current_module_id){ 
                current_module_id = module_id;

                const std::optional<size_t> module_index_opt =
                    m_geoIdMapping->athenaToDetDescIndex(Identifier(current_module_id));
                if (!module_index_opt.has_value()) {
                    ATH_MSG_ERROR("no module index for " << Identifier(current_module_id));
                    return StatusCode::FAILURE;
                }
                current_module_index = module_index_opt.value();
            }

            for (uint32_t word = range.first + 2; word < range.first + range.second; word += 2){
                input_data_64bits[payload64_idx] =
                        (static_cast<uint64_t>(payload[word])<< 32) 
                        | static_cast<uint64_t>(payload[word + 1]);
                ++payload64_idx;
            }

            size_t const stream_size_64bits = (range.second - 2) / 2;

            streams_info.push_back({payload64_start, stream_size_64bits,
                current_module_index, cabling->transformType(current_module_id),
                ITkPixelCabling::dridToChipID(detectorResourceID)});

            // This will be the start index of the next stream
            payload64_start += stream_size_64bits;
        }
    }

    if (input_data_64bits.empty() || streams_info.size() == 0) {
        ATH_MSG_ERROR("input data is empty");
        return StatusCode::FAILURE;
    }

    // Usable bits per 64-bit frame, matching the CPU DecCore configuration.
    // In the CPU implementation, DecCore calculates the frame size internally
    // when core.decode(payload64) creates the MemStreamProvider.
    // Here each CUDA thread decodes its assigned stream using decode_stream(),
    // so we construct MemStreamProvider explicitly and use the same frame-size
    // calculation as DecCore.
    const size_t frameSize = static_cast<size_t>(64 -
        itksw::pix::endec::intf::reserved_bits(m_decodingOptions));
    std::unique_ptr<traccc::edm::silicon_cell_collection::buffer> traccc_cells;

    auto device_copy = m_copies->deviceCopy(ctx);
    auto host_copy = m_copies->hostCopy(ctx);
    auto& deviceMR = m_MRs->mainMR();
    cudaStream_t stream = static_cast<cudaStream_t>(m_streamTool->stream(ctx));

    // Copy input data into GPU: copy to pinned memory first, then copy from
    // pinned memory to GPU to allows async copy
    auto input_data_size = static_cast<vecmem::edm::details::size_type>(
        input_data_64bits.size());
    vecmem::data::vector_buffer<uint64_t> input_data_buffer_host{
        input_data_size, *m_MRs->hostMR()};
    host_copy->setup(input_data_buffer_host)->ignore();
    (*host_copy)(vecmem::get_data(input_data_64bits), input_data_buffer_host)->wait();

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
            *device_copy, input_data_buffer, streams_info_buffer, frameSize,
            m_decodingOptions, traccc_cells));
    } catch(std::runtime_error const & exc) {
        ATH_MSG_ERROR(exc.what());
        return StatusCode::FAILURE;
    }

    // Register device buffer containing decoded traccc cells in SG
    const unsigned int nb_cells = traccc_cells->capacity();
    auto traccc_cells_handles = SG::makeHandle(m_tracccCellsKey, ctx);
    ATH_CHECK(traccc_cells_handles.record(std::move(traccc_cells)));

    ATH_MSG_DEBUG("nb cells created: " << nb_cells
        << ", input data size [B]: " << input_data_64bits.size() * sizeof(input_data_64bits[0]));
    return StatusCode::SUCCESS;
}
