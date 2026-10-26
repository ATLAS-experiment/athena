/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_INFERENCEUTILS_H
#define MUONINFERENCE_INFERENCEUTILS_H

#include "AthOnnxComps/OnnxRuntimeSessionToolCUDA.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "xAODMuon/MuonSegment.h"

#include <algorithm>
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

inline float reducedChi2(const xAOD::MuonSegment& segment) {
  return segment.chiSquared() / std::max(1.f, segment.numberDoF());
}

/// Three-way float comparison which orders NaN after all numeric values.
inline int compareFloat(float first, float second) {
  if (std::isnan(first)) return std::isnan(second) ? 0 : 1;
  if (std::isnan(second)) return -1;
  if (first < second) return -1;
  if (first > second) return 1;
  return 0;
}

/// Three-way descending comparison which also orders NaN last.
inline int compareFloatDescending(float first, float second) {
  if (std::isnan(first)) return std::isnan(second) ? 0 : 1;
  if (std::isnan(second)) return -1;
  if (first > second) return -1;
  if (first < second) return 1;
  return 0;
}

/// Common quality ordering for segment representatives, shared with MlMsTrackSeeder so that
/// graph construction, component ranking and seeding use the same tie-break rules.
using SegmentQualityOrder = MuonR4::SegmentQualitySorter;

}  // namespace MuonML::InferenceUtils

#endif  // MUONINFERENCE_INFERENCEUTILS_H
