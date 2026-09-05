/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_INFERENCEUTILS_H
#define MUONINFERENCE_INFERENCEUTILS_H

#include "AthOnnxComps/OnnxRuntimeSessionToolCUDA.h"
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

/** @brief Common quality ordering for segment representatives.
 *
 * Higher hit/layer counts are preferred, followed by lower reduced chi2 and
 * finally the stable container index. Keeping this comparator here prevents
 * graph construction and component ranking from acquiring different
 * tie-break rules.
 */
struct SegmentQualityOrder {
  bool operator()(const xAOD::MuonSegment* first,
                  const xAOD::MuonSegment* second) const {
    if (first->nPrecisionHits() != second->nPrecisionHits()) {
      return first->nPrecisionHits() > second->nPrecisionHits();
    }
    if (first->nPhiLayers() != second->nPhiLayers()) {
      return first->nPhiLayers() > second->nPhiLayers();
    }
    if (first->nTrigEtaLayers() != second->nTrigEtaLayers()) {
      return first->nTrigEtaLayers() > second->nTrigEtaLayers();
    }
    const int chi2Order = compareFloat(reducedChi2(*first),
                                       reducedChi2(*second));
    if (chi2Order != 0) return chi2Order < 0;
    return first->index() < second->index();
  }
};

}  // namespace MuonML::InferenceUtils

#endif  // MUONINFERENCE_INFERENCEUTILS_H
