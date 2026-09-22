// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXMPI_RPCGPU_H
#define ATHEXMPI_RPCGPU_H
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <string>

namespace RemoteCall::GPU {
/// Number of multiples generated from each input.
inline constexpr unsigned int expansion = 256;
/// Number of equal-width histogram bins.
inline constexpr int bins = 10;
/// Largest input count whose expanded histogram fits signed integer counters.
inline constexpr unsigned int maxInputs =
    std::numeric_limits<int>::max() / expansion;

/// Fixed-size device result transferred by MPI and checked on the client.
struct CrunchResult {
  std::uint32_t lower;  ///< Smallest expanded sample.
  std::uint32_t upper;  ///< Largest expanded sample, included in the final bin.
  int histogram[bins];  ///< Counts from the final histogram pass.
};

/// Initialize visible device zero and return its name.
/// @throws std::runtime_error if CUDA or stream-ordered allocation is
/// unavailable.
std::string initialize();

/// Return the process-lifetime resource used for MPI device buffers.
/// Allocations and deallocations select visible device zero on the calling
/// thread.
std::pmr::memory_resource& memoryResource();

/// Expand input into multiples 1 through 256, reduce extrema and histogram ten
/// times. Input values must be in [1, 251], with count in [1, maxInputs]. Both
/// pointers must refer to device-zero allocations. Output holds one
/// CrunchResult. Each call owns its stream and scratch storage and completes
/// before returning. Only the two histogram bounds are copied to host memory
/// during computation.
/// @throws std::runtime_error on CUDA failure or std::invalid_argument for
/// invalid count.
void crunch(const std::uint32_t* input, CrunchResult* output,
            std::size_t count);
}  // namespace RemoteCall::GPU
#endif  // ATHEXMPI_RPCGPU_H
