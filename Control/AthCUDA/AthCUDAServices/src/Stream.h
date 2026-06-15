//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHCUDASERVICES_STREAM_H
#define ATHCUDASERVICES_STREAM_H

// CUDA include(s).
#include <cuda_runtime_api.h>

// System include(s).
#include <string>

namespace AthCUDA::Details {

/// Helper class for managing a CUDA stream in memory
class Stream {

 public:
  /// Constructor, creating the CUDA stream
  Stream();
  /// Destructor, destroying the CUDA stream
  ~Stream();

  /// Get the CUDA stream
  cudaStream_t stream() const;

  /// Get the name of the device associated with the stream
  std::string name() const;

 private:
  /// The CUDA stream to use for asynchronous copies
  cudaStream_t m_stream{nullptr};

};  // class Stream

}  // namespace AthCUDA::Details

#endif  // ATHCUDASERVICES_STREAM_H
