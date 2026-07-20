// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHCUDAINTERFACES_ISTREAMPROVIDER_H
#define ATHCUDAINTERFACES_ISTREAMPROVIDER_H

// Gaudi include(s).
#include "GaudiKernel/EventContext.h"

// CUDA include(s).
#include <cuda_runtime_api.h>

namespace AthCUDA {

/// Interface for components providing CUDA streams to (reentrant) algorithms
class IStreamProvider {

 public:
  /// Destructor
  virtual ~IStreamProvider() = default;

  /// Get the CUDA stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The CUDA stream to use for the specified event context
  ///
  virtual cudaStream_t stream(const EventContext& ctx) const = 0;

};  // class IStreamProvider

}  // namespace AthCUDA

#endif  // ATHCUDAINTERFACES_ISTREAMPROVIDER_H
