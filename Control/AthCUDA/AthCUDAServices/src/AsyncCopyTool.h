//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_ASYNCCOPYTOOL_H
#define ATHCUDASERVICES_ASYNCCOPYTOOL_H

// Framework include(s).
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/SlotSpecificObj.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/ICopyTool.h"

// CUDA include(s).
#include <cuda_runtime.h>

namespace AthCUDA {

/// Tool providing an asynchronous @c vecmem::copy object for CUDA devices
class AsyncCopyTool : public extends<AthAlgTool, AthDevice::ICopyTool> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c ICopyTool
  /// @{

  /// Get the provided @c vecmem::copy object
  virtual std::shared_ptr<const vecmem::copy> copy(
      const EventContext& ctx) const override;

  /// @}

 private:
  /// Helper structure for managing a CUDA stream in memory
  struct Stream {
    /// Constructor, creating the CUDA stream
    Stream();
    /// Destructor, destroying the CUDA stream
    ~Stream();
    /// The CUDA stream to use for asynchronous copies
    cudaStream_t m_stream{nullptr};
  };  // struct Stream

  /// Slot specific CUDA stream
  std::unique_ptr<const SG::SlotSpecificObj<Stream>> m_streams;

};  // class AsyncCopyTool

}  // namespace AthCUDA

#endif  // ATHCUDASERVICES_ASYNCCOPYTOOL_H
