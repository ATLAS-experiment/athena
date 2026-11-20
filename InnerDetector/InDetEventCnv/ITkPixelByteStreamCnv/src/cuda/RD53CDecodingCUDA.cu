#include "RD53CDecodingCUDA.h"
#include "RD53CDeviceEndec.cuh"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/StatusCode.h"
#include <itksw/pix/endec/BinaryTree.hpp>
#include <vecmem/containers/data/vector_buffer.hpp>
#include <vecmem/containers/device_vector.hpp>
#include <vecmem/utils/copy.hpp>
#include <cuda_runtime.h>
#include <thrust/execution_policy.h>
#include <thrust/scan.h>
#include <mutex>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>

/*! CUDA implementation of the Pixel byte stream decoding
 *
 * This file contains the high-level logic: the decodeRD53C() function is the
   entry point called from the Athena algorithm. It calls the two main GPU
   kernels: countRD53CHits() only count the hits in each stream and
   decodeRD53CHits() creates the Traccc cells. They use the same callback class,
   GPUCallback with different parameters.

   The low-level decoding logic comes from itksw's endec project.

   \authors Roxani Lazaridou <roxani.lazaridou@cern.ch>
            Fabrice Le Goff <fabrice.le.goff@cern.ch>
 */

namespace ITkPixelDecodingCUDA {

// Callback for GPU RD53C decoding.
// If output_cells is null, it only counts the hits.
// If output_cells is not null, cells are created there starting from count.
struct GPUCallback
{
    uint32_t * const decoding_error;
    unsigned int * const count;
    traccc::edm::silicon_cell_collection::device * const output_cells;

    // The following variables are only required when creating cells.
    // {
    // Index of the module in the Detray geometry description
    const unsigned int module_index;
    // Required for coordinate transform
    const ITkPixelCabling::TransformType transform_type;
    // Required for coordinate transform
    const uint8_t chip_id;
    // }

    __device__ void evt_init(uint8_t){}
    __device__ void evt_next(uint8_t){}
    __device__ void evt_done(){}
    __device__ void add_hit(uint16_t col, uint16_t row, uint16_t tot)
    {
        const unsigned int index = (*count)++;
        if (output_cells != nullptr) {
            ITkPixelCabling::chipToModuleTransform(transform_type, chip_id, col, row);
            traccc::edm::silicon_cell cell = output_cells->at(index);
            cell.channel0() = row;
            cell.channel1() = col;
            cell.module_index() = module_index;
            cell.time() = 0;
            // Pixel ToT is a value in [0;14] (15 is reserved for no data, but
            // non-activated cells are not encoded) Since a 0 activation value
            // would simply de-activate this cell in Traccc, for the moment, we
            // just shift it by one. Maybe something more clever needs to be
            // done.
            cell.activation() = tot + 1;
        }
    }
    __device__ void add_hmap(uint8_t, uint8_t, uint16_t, uint64_t){}
    __device__ void add_qcore(uint8_t, uint8_t, uint64_t){}
    __device__ uint8_t on_error(itksw::pix::endec::intf::EventError error) {
        if (decoding_error != nullptr) {
            *decoding_error = (static_cast<uint16_t>(error.code) << 16) + (error.qcol << 8) + (error.qrow);
        }
        return 0;
    }
};

namespace kernels {

__global__ void countRD53CHits(
    vecmem::data::vector_view<uint64_t> input_data_view,
    StreamInfoContainer::const_view stream_info_view,
    const size_t num_streams,
    const size_t frame_size,
    const itksw::pix::endec::intf::Options opt,
    unsigned int* stream_counts,
    uint32_t* decoding_error)
{
    const size_t index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= num_streams) return;

    vecmem::device_vector<uint64_t> input_data(input_data_view);
    StreamInfoContainer::const_device stream_info(stream_info_view);
    StreamInfo const si = stream_info.at(index);

    unsigned int count = 0;
    itksw_cuda::endec::MemStreamProvider provider(
        input_data.data() + si.dataOffset(), si.dataSize(), frame_size);
    itksw_cuda::endec::DecStreamState state;
    state.reset();

    // When counting transform type, chip ID, and module index are irrelevant
    constexpr auto transform_type = ITkPixelCabling::TransformType::NominalQuad;
    constexpr uint8_t chip_id = 0;
    constexpr unsigned int module_index = 0;

    GPUCallback callback{decoding_error, &count, nullptr, module_index,
        transform_type, chip_id};
    itksw_cuda::endec::DebugEventVoid debug_cb;
    itksw_cuda::endec::decode_stream(opt, callback, provider, debug_cb, state);
    itksw_cuda::endec::decode_stream_end(opt, callback, provider, debug_cb, state);

    stream_counts[index] = count;
}

__global__ void decodeRD53CHits(
    vecmem::data::vector_view<uint64_t> input_data_view,
    StreamInfoContainer::const_view stream_info_view,
    const size_t num_streams,
    const size_t frame_size,
    const itksw::pix::endec::intf::Options opt,
    const unsigned int* output_offsets,
    traccc::edm::silicon_cell_collection::view output_cells_view)
{
    const size_t index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= num_streams) return;

    vecmem::device_vector<uint64_t> input_data(input_data_view);
    StreamInfoContainer::const_device stream_info(stream_info_view);
    StreamInfo const si = stream_info.at(index);

    traccc::edm::silicon_cell_collection::device output_cells(output_cells_view);
    unsigned int write_index = output_offsets[index];
    itksw_cuda::endec::MemStreamProvider provider(
        input_data.data() + si.dataOffset(), si.dataSize(), frame_size);
    itksw_cuda::endec::DecStreamState state;
    state.reset();

    // errors were already checked when counting, so we don't do it here.
    uint32_t* decoding_error = nullptr;

    GPUCallback callback{decoding_error, &write_index, &output_cells,
        si.moduleIndex(), si.transformType(), si.chipId()};
    itksw_cuda::endec::DebugEventVoid debug_cb;
    itksw_cuda::endec::decode_stream(opt, callback, provider, debug_cb, state);
    itksw_cuda::endec::decode_stream_end(opt, callback, provider, debug_cb, state);
}

} // namespace kernels

// This is imported from Endec v3.0.1. It was later removed. Once we rework the
// GPU decoder to directly use endec this will go away with the rest.
namespace EndecBinaryTreeLUT {
    using HitLutEntry = std::pair<uint8_t, uint8_t>;
    using HitLutRow = std::array<HitLutEntry, 16>;
    using HitLutType = std::array<HitLutRow, 1 << 16>;
    auto create_hit_lut_length_() {
        std::array<uint8_t, 1 << 16> result{};
        for (uint32_t i = 0; i < (1 << 16); i++) result[i] = std::popcount(i);
        return result;
    }
    auto create_hit_lut_() {
        HitLutType result{};
        for (uint32_t hitmap = 0; hitmap < (1 << 16); ++hitmap) {
            int index{};
            for (uint8_t bit = 0; bit < 16; ++bit) {
                if ((hitmap & (1 << bit)) == 0) continue;
                result[hitmap][index++] = {bit % 8, bit / 8};
            }
        }
        return result;
    }
} // namespace EndecBinaryTreeLUT

// Copies the release BinaryTreeLUT tables (same values) into the device tables.
static void initDeviceTables()
{
    using Release = itksw::pix::endec::BinaryTreeLUT;
    namespace dev = itksw_cuda::endec;
    static std::once_flag once;
    std::call_once(once, [] {

        EndecBinaryTreeLUT::HitLutType endec_hit_lut{EndecBinaryTreeLUT::create_hit_lut_()};
        std::array<uint8_t, (1 << 16)> endec_hit_lut_length{EndecBinaryTreeLUT::create_hit_lut_length_()};

        std::vector<dev::HitLutRow> hit_lut(dev::HIT_LUT_SIZE);
        std::vector<uint8_t> hit_lut_length(dev::HIT_LUT_SIZE);
        for (size_t i = 0; i < dev::HIT_LUT_SIZE; ++i) {
            for (size_t j = 0; j < 16; ++j) {
                hit_lut[i].e[j].first = endec_hit_lut[i][j].first;
                hit_lut[i].e[j].second = endec_hit_lut[i][j].second;
            }
            hit_lut_length[i] = endec_hit_lut_length[i];
        }

        std::vector<dev::LutTopEntry> lut_top(dev::LUT_TOP_SIZE);
        for (size_t i = 0; i < dev::LUT_TOP_SIZE; ++i) {
            lut_top[i] = {std::get<0>(Release::lut_top[i]),
                          std::get<1>(Release::lut_top[i]),
                          std::get<2>(Release::lut_top[i])};
        }

        std::vector<dev::LutBottomEntry> lut_bottom(dev::LUT_BOTTOM_SIZE);
        for (size_t i = 0; i < dev::LUT_BOTTOM_SIZE; ++i) {
            lut_bottom[i] = {std::get<0>(Release::lut_bottom[i]),
                             std::get<1>(Release::lut_bottom[i])};
        }

        CUDA_ERROR_CHECK(cudaMemcpyToSymbol(dev::tables::hit_lut,
            hit_lut.data(), hit_lut.size() * sizeof(dev::HitLutRow)));
        CUDA_ERROR_CHECK(cudaMemcpyToSymbol(dev::tables::hit_lut_length,
            hit_lut_length.data(), hit_lut_length.size()));
        CUDA_ERROR_CHECK(cudaMemcpyToSymbol(dev::tables::lut_top,
            lut_top.data(), lut_top.size() * sizeof(dev::LutTopEntry)));
        CUDA_ERROR_CHECK(cudaMemcpyToSymbol(dev::tables::lut_bottom,
            lut_bottom.data(), lut_bottom.size() * sizeof(dev::LutBottomEntry)));
    });
}

StatusCode decodeRD53C(
    cudaStream_t stream,
    vecmem::memory_resource& mr,
    vecmem::memory_resource& host_mr,
    vecmem::copy const& copy,
    vecmem::data::vector_view<uint64_t> const & input_data_view,
    StreamInfoContainer::const_view const & stream_info_view,
    const size_t frame_size,
    const itksw::pix::endec::intf::Options& options,
    std::unique_ptr<traccc::edm::silicon_cell_collection::buffer>& output_cells)
{
    // Load the decoder look up table, run only once internally
    initDeviceTables();

    StreamInfoContainer::const_device streams_info(stream_info_view);
    const size_t num_streams = streams_info.size();

    constexpr unsigned int block_size = 256;
    const unsigned int num_blocks = (num_streams + block_size - 1) / block_size;

    vecmem::data::vector_buffer<unsigned int> stream_counts(num_streams, mr);
    copy.setup(stream_counts)->ignore();

    vecmem::data::vector_buffer<uint32_t> decoding_error_buffer(1, mr);
    vecmem::device_vector<uint32_t> decoding_error(decoding_error_buffer);

    kernels::countRD53CHits<<<num_blocks, block_size, 0, stream>>>(
        input_data_view, stream_info_view, num_streams, frame_size, options,
        stream_counts.ptr(), decoding_error.data());

    CUDA_ERROR_CHECK(cudaGetLastError());
    CUDA_ERROR_CHECK(cudaStreamSynchronize(stream));

    vecmem::data::vector_buffer<uint32_t> decoding_error_host_buffer(1, host_mr);
    copy(vecmem::ignore_event, decoding_error_buffer, decoding_error_host_buffer);
    vecmem::device_vector<uint32_t> decoding_error_host(decoding_error_host_buffer);
    if (decoding_error_host[0] != 0) {
        std::ostringstream oss;
        oss << "decoding error: code " << (decoding_error_host[0] >> 16)
            << ", qcol " << ((decoding_error_host[0] >> 8) & 0xF)
            << ", qrow " << (decoding_error_host[0] & 0xF);
        throw std::runtime_error(oss.str());
    }

    // Compute the per-stream output offsets on the GPU using a prefix sum.
    // Only the final values are copied back to the host to determine the total
    // number of decoded hits.

    vecmem::data::vector_buffer<unsigned int> output_offsets(num_streams, mr);
    copy.setup(output_offsets)->ignore();

    thrust::exclusive_scan(
        thrust::cuda::par.on(stream),
        stream_counts.ptr(), stream_counts.ptr() + num_streams,
        output_offsets.ptr());
    CUDA_ERROR_CHECK(cudaGetLastError());

    // Total hits = last offset + last count (the scan was *exclusive*)
    // allocate pinned memory for the async transfer
    vecmem::data::vector_buffer<unsigned int> last_offset_count_host_buffer(2, host_mr);
    unsigned int * const last_offset_ptr = last_offset_count_host_buffer.ptr();
    unsigned int * const last_count_ptr = last_offset_count_host_buffer.ptr() + 1;

    CUDA_ERROR_CHECK(cudaMemcpyAsync(last_offset_ptr,
        output_offsets.ptr() + (num_streams - 1),
        sizeof(unsigned int), cudaMemcpyDeviceToHost, stream));
    CUDA_ERROR_CHECK(cudaMemcpyAsync(last_count_ptr,
        stream_counts.ptr() + (num_streams - 1),
        sizeof(unsigned int), cudaMemcpyDeviceToHost, stream));
    CUDA_ERROR_CHECK(cudaStreamSynchronize(stream));

    const unsigned int output_size = *last_offset_ptr + *last_count_ptr;
    output_cells = std::make_unique<traccc::edm::silicon_cell_collection::buffer>(
        output_size, mr);
    copy.setup(*output_cells)->ignore();

    if (output_size > 0) {
        kernels::decodeRD53CHits<<<num_blocks, block_size, 0, stream>>>(
            input_data_view, stream_info_view, num_streams, frame_size, options,
            output_offsets.ptr(), *output_cells);
    }

    CUDA_ERROR_CHECK(cudaGetLastError());
    CUDA_ERROR_CHECK(cudaStreamSynchronize(stream));
    return StatusCode::SUCCESS;
}

} // namespace PixelDecodingCUDA
