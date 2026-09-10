// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXMPI_REMOTEGPUEXAMPLEALG_H
#define ATHEXMPI_REMOTEGPUEXAMPLEALG_H

// Local include(s)
#include "HostDevicePtr.h"
#include "RemoteGPUSvc.h"

// Athena include(s)
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// Gaudi include(s)
#include "GaudiKernel/ServiceHandle.h"

namespace RemoteCall {

/// Skeleton asynchronous algorithm for testing remote GPU communication.
class RemoteGPUExampleAlg : public AthAsynchronousAlgorithm {
 public:
  /// Inherit the base class constructor(s).
  using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

  /// Retrieve the services used by the algorithm.
  virtual StatusCode initialize() override;

  /// Execute the remote GPU communication for one event.
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
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
