/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITKPIXEL_RD53CDECODING_CUDA_H
#define ITKPIXEL_RD53CDECODING_CUDA_H

#include "GaudiKernel/StatusCode.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "itksw/pix/endec/intf/Options.hpp"
#include <vecmem/containers/data/vector_buffer.hpp>
#include <vecmem/containers/data/vector_view.hpp>
#include <vecmem/memory/memory_resource.hpp>
#include <vecmem/utils/copy.hpp>
#include "traccc/definitions/qualifiers.hpp"
#include <vecmem/edm/container.hpp>
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <cstdint>

/*! Host to GPU interface
 *
 * Functions called from host code, classes for objects provided by host code.
 * The implementation calls the GPU side of the code.
 * 
 * \authors Roxani Lazaridou <roxani.lazaridou@cern.ch>
 *          Fabrice Le Goff <fabrice.le.goff@cern.ch>  
 */

// Throws on a failed CUDA call (C++17-compatible; no <format>).
#define CUDA_ERROR_CHECK(EXP)                                                  \
  do {                                                                         \
    cudaError_t errorCode = EXP;                                               \
    if (errorCode != cudaSuccess) {                                            \
      throw std::runtime_error(std::string(__FILE__) + ":" +                   \
                               std::to_string(__LINE__) +                      \
                               " Failed to execute: " #EXP " (" +              \
                               cudaGetErrorString(errorCode) + ")");           \
    }                                                                          \
  } while (false)


namespace ITkPixelDecodingCUDA {

// Stream based info needed for decoding and provided by the algorithm
template <typename BASE>
struct StreamInfo : public BASE {
  using BASE::BASE;
  using BASE::operator=;

  // size_t: index at which the stream starts 
  TRACCC_HOST_DEVICE auto& dataOffset() { return BASE::template get<0>(); }
  TRACCC_HOST_DEVICE const auto& dataOffset() const { return BASE::template get<0>(); }
  // size_t: size of the stream data
  TRACCC_HOST_DEVICE auto& dataSize() { return BASE::template get<1>(); }
  TRACCC_HOST_DEVICE const auto& dataSize() const { return BASE::template get<1>(); }
  // unsigned int: detray module index associated with the stream
  TRACCC_HOST_DEVICE auto& moduleIndex() { return BASE::template get<2>(); }
  TRACCC_HOST_DEVICE const auto& moduleIndex() const { return BASE::template get<2>(); }
  // ITkPixelCabling::TransformType: coordinate transform associated with the stream
  TRACCC_HOST_DEVICE auto& transformType() { return BASE::template get<3>(); }
  TRACCC_HOST_DEVICE const auto& transformType() const { return BASE::template get<3>(); }
  // uint_8: chip ID associated with the stream
  TRACCC_HOST_DEVICE auto& chipId() { return BASE::template get<4>(); }
  TRACCC_HOST_DEVICE const auto& chipId() const { return BASE::template get<4>(); }
};

using StreamInfoContainer = vecmem::edm::container<
  StreamInfo
  , vecmem::edm::type::vector<size_t> // data offset
  , vecmem::edm::type::vector<size_t> // data size
  , vecmem::edm::type::vector<unsigned int> // module index
  , vecmem::edm::type::vector<ITkPixelCabling::TransformType> // transform type
  , vecmem::edm::type::vector<uint8_t> // chip ID
  >;

// Decodes RD53C streams from input_data into traccc cells
StatusCode decodeRD53C(
  cudaStream_t stream,
  vecmem::memory_resource& mr,
  vecmem::memory_resource& host_mr,
  vecmem::copy const& copy,
  vecmem::data::vector_view<uint64_t> const & input_data,
  StreamInfoContainer::const_view const & stream_info_view,
  size_t frame_size,
  const itksw::pix::endec::intf::Options& options,
  std::unique_ptr<traccc::edm::silicon_cell_collection::buffer>& output_cells);

} // namespace PixelDecodingCUDA

#endif // ITKPIXEL_RD53CDECODING_CUDA_H