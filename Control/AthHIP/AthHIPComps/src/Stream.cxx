//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "Stream.h"

// Framework include(s).
#include "CxxUtils/checker_macros.h"

// System include(s).
#include <format>
#include <stdexcept>

/// Helper macro used for checking @c hipError_t type return values.
#define HIP_ERROR_CHECK(EXP)                                                   \
  do {                                                                         \
    hipError_t errorCode = EXP;                                                \
    if (errorCode != hipSuccess) {                                             \
      throw std::runtime_error(std::format("{}:{} Failed to execute: {} ({})", \
                                           __FILE__, __LINE__, #EXP,           \
                                           hipGetErrorString(errorCode)));     \
    }                                                                          \
  } while (false)

namespace AthHIP::Details {

Stream::Stream() {
  HIP_ERROR_CHECK(hipStreamCreate(&m_stream));
}

/// Let's not check for errors here. Since that would introduce a struct that
/// may throw in its destructor. Which we don't want.
Stream::~Stream() {
  [[maybe_unused]] hipError_t error = hipStreamDestroy(m_stream);
}

hipStream_t Stream::stream() const {

  // The HIP runtime promises thread safety for handling streams in parallel
  // from different CPU threads. Returning a non-const pointer of course allows
  // us to cause harm. But as long as user code is not trying to actively break
  // things, we should be fine.
  hipStream_t result ATLAS_THREAD_SAFE = m_stream;
  return result;
}

std::string Stream::name() const {

  // Get the device's properties.
  int device = -1;
  HIP_ERROR_CHECK(hipStreamGetDevice(stream(), &device));
  hipDeviceProp_t props;
  HIP_ERROR_CHECK(hipGetDeviceProperties(&props, device));

  // Construct a unique name out of those properties.
  return std::format("{} [id: {}, bus: {}, device: {}]", props.name, device,
                     props.pciBusID, props.pciDeviceID);
}

}  // namespace AthHIP::Details
