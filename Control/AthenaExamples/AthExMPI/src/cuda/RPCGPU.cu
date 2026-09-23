// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#include <cuda_runtime.h>

#include <algorithm>
#include <cub/device/device_histogram.cuh>
#include <cub/device/device_reduce.cuh>
#include <exception>
#include <stdexcept>
#include <string>

#include "RPCGPU.h"

namespace RemoteCall::GPU {
namespace {

/// Throw an exception identifying a failed CUDA operation.
void check(cudaError_t status, const char* operation) {
  if (status != cudaSuccess) {
    throw std::runtime_error(std::string(operation) + ": " +
                             cudaGetErrorString(status));
  }
}

/// Allocate device buffers for MPI receives and remote results.
class DeviceResource final : public std::pmr::memory_resource {
 private:
  /// Allocate at least one byte so empty MPI payloads also have valid storage.
  /// @throws std::bad_alloc for unsupported alignment or allocation failure.
  void* do_allocate(std::size_t bytes, std::size_t alignment) override {
    if (alignment == 0 || alignment > 256 ||
        (alignment & (alignment - 1)) != 0) {
      throw std::bad_alloc();
    }
    check(cudaSetDevice(0), "cudaSetDevice");
    void* result = nullptr;
    if (cudaMalloc(&result, std::max(bytes, std::size_t{1})) != cudaSuccess) {
      throw std::bad_alloc();
    }
    return result;
  }

  /// Free storage on its allocating device. Terminate if CUDA cleanup fails.
  void do_deallocate(void* ptr, std::size_t, std::size_t) override {
    if (cudaSetDevice(0) != cudaSuccess || cudaFree(ptr) != cudaSuccess) {
      std::terminate();
    }
  }

  /// Resource instances compare equal only when they are the same object.
  bool do_is_equal(
      const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }
};

/// Own a nonblocking stream for one remote invocation.
class Stream {
 public:
  /// Create the stream on the calling thread's current CUDA device.
  Stream() {
    check(cudaStreamCreateWithFlags(&m_stream, cudaStreamNonBlocking),
          "cudaStreamCreateWithFlags");
  }

  /// Finish pending work before destroying the stream, including on exceptions.
  ~Stream() {
    if (cudaStreamSynchronize(m_stream) != cudaSuccess ||
        cudaStreamDestroy(m_stream) != cudaSuccess) {
      std::terminate();
    }
  }

  /// Copying stream ownership is disabled.
  Stream(const Stream&) = delete;
  /// Copy assignment is disabled.
  Stream& operator=(const Stream&) = delete;
  /// Return the stream handle for kernel launch and synchronization.
  cudaStream_t get() const noexcept { return m_stream; }

 private:
  cudaStream_t m_stream = nullptr;  ///< Stream owned by this invocation.
};

/// Reject host buffers and buffers allocated on another CUDA device.
void requireDevicePointer(const void* ptr) {
  cudaPointerAttributes attributes{};
  check(cudaPointerGetAttributes(&attributes, ptr), "cudaPointerGetAttributes");
  if (attributes.type != cudaMemoryTypeDevice || attributes.device != 0) {
    throw std::runtime_error(
        "GPU RPC requires buffers in CUDA device zero memory");
  }
}

/// Own temporary storage whose allocation and release are ordered on one
/// stream.
class Scratch {
 public:
  /// Allocate storage before subsequent operations on the supplied stream.
  Scratch(std::size_t bytes, cudaStream_t stream) : m_stream(stream) {
    check(cudaMallocAsync(&m_ptr, std::max(bytes, std::size_t{1}), stream),
          "cudaMallocAsync");
  }
  /// Queue release after prior work. The stream must outlive this object.
  ~Scratch() {
    if (cudaFreeAsync(m_ptr, m_stream) != cudaSuccess)
      std::terminate();
  }
  /// Copying allocation ownership is disabled.
  Scratch(const Scratch&) = delete;
  /// Copy assignment is disabled.
  Scratch& operator=(const Scratch&) = delete;
  /// Return the device allocation.
  void* get() const noexcept { return m_ptr; }

 private:
  void* m_ptr = nullptr;  ///< Owned temporary device allocation.
  cudaStream_t m_stream;  ///< Stream that orders this allocation's lifetime.
};

/// Generate 256 integer multiples of each input without staging through host
/// memory.
__global__ void expandKernel(const std::uint32_t* input, std::uint32_t* output,
                             std::size_t count) {
  const std::size_t first = std::size_t(blockIdx.x) * blockDim.x + threadIdx.x;
  const std::size_t stride = std::size_t(blockDim.x) * gridDim.x;
  for (std::size_t i = first; i < count * expansion; i += stride) {
    output[i] = input[i / expansion] * (i % expansion + 1);
  }
}

}  // namespace

std::string initialize() {
  check(cudaSetDevice(0), "cudaSetDevice");
  cudaDeviceProp properties{};
  check(cudaGetDeviceProperties(&properties, 0), "cudaGetDeviceProperties");
  int poolsSupported = 0;
  check(cudaDeviceGetAttribute(&poolsSupported, cudaDevAttrMemoryPoolsSupported,
                               0),
        "CUDA memory pool support");
  if (!poolsSupported)
    throw std::runtime_error(
        "GPU test requires stream-ordered allocation support");
  // Create the context before the MPI receive thread starts using the resource.
  check(cudaFree(nullptr), "CUDA context initialization");
  return properties.name;
}

std::pmr::memory_resource& memoryResource() {
  static DeviceResource resource;
  return resource;
}

void crunch(const std::uint32_t* input, CrunchResult* output,
            std::size_t count) {
  if (count == 0 || count > maxInputs)
    throw std::invalid_argument("Invalid GPU input count");
  check(cudaSetDevice(0), "cudaSetDevice");
  requireDevicePointer(input);
  requireDevicePointer(output);
  Stream stream;
  const auto samples = static_cast<int>(count * expansion);
  Scratch expanded(std::size_t(samples) * sizeof(std::uint32_t), stream.get());
  auto* data = static_cast<std::uint32_t*>(expanded.get());
  const auto blocks =
      static_cast<unsigned int>(std::min(count, std::size_t{65535}));
  expandKernel<<<blocks, expansion, 0, stream.get()>>>(input, data, count);
  check(cudaGetLastError(), "GPU expansion kernel");

  std::size_t minBytes = 0, maxBytes = 0;
  check(cub::DeviceReduce::Min(nullptr, minBytes, data, &output->lower, samples,
                               stream.get()),
        "CUB Min workspace");
  check(cub::DeviceReduce::Max(nullptr, maxBytes, data, &output->upper, samples,
                               stream.get()),
        "CUB Max workspace");
  Scratch reduction(std::max(minBytes, maxBytes), stream.get());
  check(cub::DeviceReduce::Min(reduction.get(), minBytes, data, &output->lower,
                               samples, stream.get()),
        "CUB Min");
  check(cub::DeviceReduce::Max(reduction.get(), maxBytes, data, &output->upper,
                               samples, stream.get()),
        "CUB Max");
  std::uint32_t lower = 0, upper = 0;
  check(cudaMemcpyAsync(&lower, &output->lower, sizeof(lower),
                        cudaMemcpyDeviceToHost, stream.get()),
        "Copy lower bound");
  check(cudaMemcpyAsync(&upper, &output->upper, sizeof(upper),
                        cudaMemcpyDeviceToHost, stream.get()),
        "Copy upper bound");
  check(cudaStreamSynchronize(stream.get()), "GPU bounds completion");
  if (lower == 0 || upper > 251 * expansion)
    throw std::runtime_error("GPU samples outside test bounds");

  std::size_t histogramBytes = 0;
  check(cub::DeviceHistogram::HistogramEven(nullptr, histogramBytes, data,
                                            output->histogram, bins + 1, lower,
                                            upper + 1, samples, stream.get()),
        "CUB histogram workspace");
  Scratch histogram(histogramBytes, stream.get());
  for (int pass = 0; pass < 10; ++pass) {
    auto available = histogramBytes;
    check(cub::DeviceHistogram::HistogramEven(
              histogram.get(), available, data, output->histogram, bins + 1,
              lower, upper + 1, samples, stream.get()),
          "CUB histogram");
  }
  check(cudaStreamSynchronize(stream.get()), "GPU histogram completion");
}

}  // namespace RemoteCall::GPU
