// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXMPI_REMOTEGPUEXAMPLEALG_H
#define ATHEXMPI_REMOTEGPUEXAMPLEALG_H

// Local include(s)
#include "HostDevicePtr.h"
#include "RemoteGPUSvc.h"
#include "cuda/RPCGPU.h"

// Athena include(s)
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// Gaudi include(s)
#include "GaudiKernel/ServiceHandle.h"

namespace RemoteCall {

/// Check remote CPU and CUDA computations through the MPI RPC service.
class RemoteGPUExampleAlg : public AthAsynchronousAlgorithm {
 public:
  /// Inherit the base class constructor(s).
  using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

  /// Retrieve the services used by the algorithm.
  virtual StatusCode initialize() override;

  /// Execute the remote GPU communication for one event.
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// Require a GPU RPC in addition to the CPU checks on every client event.
  /// The server uses visible CUDA device zero. Set false to run the same-sized
  /// request and result through host memory instead.
  Gaudi::Property<bool> m_testGPU{
      this, "TestGPU", true,
      "Test MPI transfers to and from CUDA device memory"};

  /// Number of inputs expanded into 256 samples each in the host or GPU test.
  Gaudi::Property<unsigned int> m_gpuInputSize{
      this, "GPUInputSize", 16384, "Number of GPU workload inputs per event"};

  /// Reduce and histogram a range received in GPU memory, returning a device
  /// result. The invocation synchronizes its CUDA stream before returning to
  /// MPI.
  /// @throws std::runtime_error if CUDA is unavailable or the operation fails.
  static RPCRet<Device, GPU::CrunchResult> test_gpu(
      RPCArg<Device, std::span<std::uint32_t>> values);

  /// Compute extrema and a histogram on the CPU with the GPU RPC's wire sizes.
  /// Both the received input and returned result use host memory.
  static RPCRet<Host, GPU::CrunchResult> test_host(
      RPCArg<Host, std::span<std::uint32_t>> values);

  /// Service implementing the client and server communication.
  ServiceHandle<RemoteGPUSvc> m_remoteGPUSvc{
      this, "RemoteGPUSvc", "RemoteCall::RemoteGPUSvc", "Remote GPU service"};

  /// Example CPU offloaded function
  static RPCRet<Host, double> test_fn(RPCArg<Host, int> arg1,
                                      RPCArg<Host, double> arg2);

  /// Return doubled range elements and their count in owned host buffers.
  static std::tuple<RPCRet<Host, std::span<double>>, RPCRet<Host, std::size_t>>
  test_range(RPCArg<Host, std::span<double>> values);
};

}  // namespace RemoteCall

#endif  // ATHEXMPI_REMOTEGPUEXAMPLEALG_H
