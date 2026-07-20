/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_INFERENCEUTILS_H
#define MUONINFERENCE_INFERENCEUTILS_H

#include "AthOnnxComps/OnnxRuntimeSessionToolCUDA.h"

#include <cmath>

namespace MuonML::InferenceUtils {

struct SessionBackend {
  bool isCuda{false};
  int cudaDeviceId{0};
};

template <class SessionToolHandle>
SessionBackend sessionBackend(const SessionToolHandle& sessionTool) {
  if (const auto* cudaTool = dynamic_cast<const AthOnnx::OnnxRuntimeSessionToolCUDA*>(sessionTool.get())) {
    return SessionBackend{true, cudaTool->deviceId()};
  }
  return SessionBackend{};
}

inline float sigmoid(float x) {
  if (x >= 0.f) {
    const float z = std::exp(-x);
    return 1.f / (1.f + z);
  }
  const float z = std::exp(x);
  return z / (1.f + z);
}

}  // namespace MuonML::InferenceUtils

#endif  // MUONINFERENCE_INFERENCEUTILS_H
