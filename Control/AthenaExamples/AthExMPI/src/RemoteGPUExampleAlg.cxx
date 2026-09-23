// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s)
#include "RemoteGPUExampleAlg.h"

#include "cuda/RPCGPU.h"

// System include(s)
#include <algorithm>
#include <array>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace RemoteCall {

StatusCode RemoteGPUExampleAlg::initialize() {
  ATH_CHECK(m_remoteGPUSvc.retrieve());
  ATH_CHECK(m_remoteGPUSvc->registerFunction(test_fn, "test_fn"));
  ATH_CHECK(m_remoteGPUSvc->registerFunction(test_range, "test_range"));
  if (m_gpuInputSize == 0 || m_gpuInputSize > GPU::maxInputs) {
    ATH_MSG_ERROR("GPUInputSize must be between 1 and " << GPU::maxInputs);
    return StatusCode::FAILURE;
  }
  if (m_testGPU) {
#ifdef ATHEXMPI_HAVE_CUDA
    if (m_remoteGPUSvc->isServer()) {
      try {
        ATH_MSG_INFO(
            "GPU RPC server using CUDA device 0: " << GPU::initialize());
        const auto device =
            m_remoteGPUSvc->registerDevice(&GPU::memoryResource());
        if (!device ||
            *device != static_cast<unsigned int>(Destination::Device)) {
          ATH_MSG_ERROR(
              "GPU test could not register the RPC device destination");
          return StatusCode::FAILURE;
        }
      } catch (const std::exception& error) {
        ATH_MSG_ERROR("GPU test initialization failed: " << error.what());
        return StatusCode::FAILURE;
      }
    }
    ATH_CHECK(m_remoteGPUSvc->registerFunction(test_gpu, "test_gpu"));
#else
    ATH_MSG_ERROR(
        "TestGPU requires a CUDA-enabled build; use TestGPU=False for CPU "
        "checks");
    return StatusCode::FAILURE;
#endif
  } else {
    ATH_CHECK(m_remoteGPUSvc->registerFunction(test_host, "test_host"));
  }
  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUExampleAlg::execute(const EventContext& ctx) const {
  if (m_remoteGPUSvc->isServer())
    return StatusCode::SUCCESS;

  int arg1 = 500;
  double arg2 = 15.3;
  auto [result] =
      m_remoteGPUSvc->call<double>(ctx, *this, "test_fn", &arg1, &arg2);
  if (*result != arg1 + arg2) {
    ATH_MSG_ERROR("Answer Incorrect! Scalar RPC result mismatch");
    return StatusCode::FAILURE;
  }

  // Vary the range length and values by event to check empty ranges and result
  // matching.
  std::array<double, 4> storage{double(ctx.evt()), 1.5, -2.0, 4.0};
  std::span<double> values(storage.data(), ctx.evt() % (storage.size() + 1));
  auto [doubled, count] = m_remoteGPUSvc->call<std::span<double>, std::size_t>(
      ctx, *this, "test_range", &values);
  if (doubled.size != values.size() || *count != values.size()) {
    ATH_MSG_ERROR("Answer Incorrect! Range RPC size mismatch");
    return StatusCode::FAILURE;
  }
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (doubled.ptr[i] != 2 * values[i]) {
      ATH_MSG_ERROR("Answer Incorrect! Range RPC value mismatch");
      return StatusCode::FAILURE;
    }
  }
  {
    std::vector<std::uint32_t> input(m_gpuInputSize);
    for (std::size_t i = 0; i < input.size(); ++i) {
      input[i] = 1 + (i * 17 + ctx.evt() % 251) % 251;
    }
    std::span<std::uint32_t> crunchValues(input);
    auto [crunchResult] = m_remoteGPUSvc->call<GPU::CrunchResult>(
        ctx, *this, m_testGPU ? "test_gpu" : "test_host", &crunchValues);
    GPU::CrunchResult expected{};
    expected.lower = *std::min_element(input.begin(), input.end());
    expected.upper =
        *std::max_element(input.begin(), input.end()) * GPU::expansion;
    for (const auto value : input) {
      for (unsigned int factor = 1; factor <= GPU::expansion; ++factor) {
        const auto bin = (std::uint64_t(value * factor) - expected.lower) *
                         GPU::bins / (expected.upper + 1 - expected.lower);
        ++expected.histogram[bin];
      }
    }
    if (crunchResult.size != 1 || crunchResult.ptr->lower != expected.lower ||
        crunchResult.ptr->upper != expected.upper) {
      ATH_MSG_ERROR("Answer Incorrect! Crunch RPC extrema mismatch");
      return StatusCode::FAILURE;
    }
    for (int bin = 0; bin < GPU::bins; ++bin) {
      if (crunchResult.ptr->histogram[bin] != expected.histogram[bin]) {
        ATH_MSG_ERROR("Answer Incorrect! Crunch RPC histogram bin "
                      << bin << ": expected " << expected.histogram[bin]
                      << ", received " << crunchResult.ptr->histogram[bin]);
        return StatusCode::FAILURE;
      }
    }
    ATH_MSG_INFO((m_testGPU ? "GPU" : "Host")
                 << " RPC result verified: " << input.size() * GPU::expansion
                 << " samples, extrema and all " << GPU::bins
                 << " histogram bins");
  }
  ATH_MSG_INFO("Answer Correct!");
  return StatusCode::SUCCESS;
}

RPCRet<Host, double> RemoteGPUExampleAlg::test_fn(RPCArg<Host, int> arg1,
                                                  RPCArg<Host, double> arg2) {
  return {new double(*arg1.ptr + *arg2.ptr)};
}

std::tuple<RPCRet<Host, std::span<double>>, RPCRet<Host, std::size_t>>
RemoteGPUExampleAlg::test_range(RPCArg<Host, std::span<double>> values) {
  RPCRet<Host, std::span<double>> result(new double[values.size], values.size);
  for (std::size_t i = 0; i < values.size; ++i)
    result.ptr[i] = 2 * values.ptr[i];
  return {std::move(result),
          RPCRet<Host, std::size_t>(new std::size_t(values.size))};
}

RPCRet<Device, GPU::CrunchResult> RemoteGPUExampleAlg::test_gpu(
    RPCArg<Device, std::span<std::uint32_t>> values) {
#ifdef ATHEXMPI_HAVE_CUDA
  auto& resource = GPU::memoryResource();
  auto* output = static_cast<GPU::CrunchResult*>(
      resource.allocate(sizeof(GPU::CrunchResult), alignof(GPU::CrunchResult)));
  RPCRet<Device, GPU::CrunchResult> result(output, 1, resource);
  GPU::crunch(values.ptr, result.ptr, values.size);
  return result;
#else
  (void)values;
  throw std::runtime_error("GPU RPC requires a CUDA-enabled build");
#endif
}

RPCRet<Host, GPU::CrunchResult> RemoteGPUExampleAlg::test_host(
    RPCArg<Host, std::span<std::uint32_t>> values) {
  if (values.size == 0 || values.size > GPU::maxInputs) {
    throw std::invalid_argument("Host crunch input count is out of range");
  }
  RPCRet<Host, GPU::CrunchResult> result(new GPU::CrunchResult{});
  const std::span<const std::uint32_t> input(values.ptr, values.size);
  const auto [lower, upper] = std::minmax_element(input.begin(), input.end());
  result.ptr->lower = *lower;
  result.ptr->upper = *upper * GPU::expansion;
  for (const auto value : input) {
    for (unsigned int factor = 1; factor <= GPU::expansion; ++factor) {
      const auto bin = (std::uint64_t(value * factor) - result.ptr->lower) *
                       GPU::bins / (result.ptr->upper + 1 - result.ptr->lower);
      ++result.ptr->histogram[bin];
    }
  }
  return result;
}

}  // namespace RemoteCall
