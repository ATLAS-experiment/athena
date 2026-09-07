// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHHIPINTERFACES_ISTREAMPROVIDER_H
#define ATHHIPINTERFACES_ISTREAMPROVIDER_H

// Gaudi include(s).
#include "GaudiKernel/EventContext.h"

// HIP include(s).
#include <hip/hip_runtime_api.h>

namespace AthHIP {

/// Interface for components providing HIP streams to (reentrant) algorithms
class IStreamProvider {

 public:
  /// Destructor
  virtual ~IStreamProvider() = default;

  /// Get the HIP stream to use
  ///
  /// @param ctx The event context for which the stream is requested
  /// @returns The HIP stream to use for the specified event context
  ///
  virtual hipStream_t stream(const EventContext& ctx) const = 0;

};  // class IStreamProvider

}  // namespace AthHIP

#endif  // ATHHIPINTERFACES_ISTREAMPROVIDER_H
