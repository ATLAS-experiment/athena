//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHHIPCOMPS_STREAM_H
#define ATHHIPCOMPS_STREAM_H

// HIP include(s).
#include <hip/hip_runtime_api.h>

// System include(s).
#include <string>

namespace AthHIP::Details {

/// Helper class for managing a HIP stream in memory
class Stream {

 public:
  /// Constructor, creating the HIP stream
  Stream();
  /// Destructor, destroying the HIP stream
  ~Stream();

  /// Get the HIP stream
  hipStream_t stream() const;

  /// Get the name of the device associated with the stream
  std::string name() const;

 private:
  /// The HIP stream to use for asynchronous copies
  hipStream_t m_stream{nullptr};

};  // class Stream

}  // namespace AthHIP::Details

#endif  // ATHHIPCOMPS_STREAM_H
