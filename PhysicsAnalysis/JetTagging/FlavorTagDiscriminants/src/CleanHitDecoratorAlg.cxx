/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/CleanHitDecoratorAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "AthContainers/AuxElement.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace {

  struct HitPos {
    float z;
    float phi;
    unsigned long deid;
    int layer;
  };

  inline float deltaPhi(const HitPos& a, const HitPos& b) {
    float d = b.phi - a.phi;
    if (d > M_PI) d -= 2*M_PI;
    else if (d <= -M_PI) d += 2*M_PI;
    return d;
  }

  inline bool isGoodHit(
    const xAOD::TrackMeasurementValidation* h,
    const SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, char>& isFakeHandle,
    const SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>& hasBSErrHandle,
    const SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>& DCSStateHandle) {
    if (isFakeHandle(*h)) return false;
    if (hasBSErrHandle(*h)) return false;
    if (DCSStateHandle(*h)) return false;
    return true;
  }

}

namespace FlavorTagDiscriminants {

  CleanHitDecoratorAlg::CleanHitDecoratorAlg(const std::string& name, ISvcLocator* svcLoc)
    : AthReentrantAlgorithm(name, svcLoc) {}

  StatusCode CleanHitDecoratorAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name());

    ATH_CHECK(m_hitContainer.initialize());

    ATH_CHECK(m_cleanHitKey.initialize());

    // The goodness decorations exist only on pixel hits
    ATH_CHECK(m_isFakeKey.initialize(!m_isSCT));
    ATH_CHECK(m_hasBSErrKey.initialize(!m_isSCT));
    ATH_CHECK(m_DCSStateKey.initialize(!m_isSCT));

    // The geometry decorations are read only by the overlap removal
    ATH_CHECK(m_becKey.initialize(m_doOverlapRemoval));
    ATH_CHECK(m_deidKey.initialize(m_doOverlapRemoval));
    ATH_CHECK(m_layerKey.initialize(m_doOverlapRemoval));

    return StatusCode::SUCCESS;
  }

  StatusCode CleanHitDecoratorAlg::execute(const EventContext& ctx) const {
    // Read hits
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> hits(m_hitContainer, ctx);
    if (!hits.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_hitContainer.key());
      return StatusCode::FAILURE;
    }

    // Decoration handle
    SG::WriteDecorHandle<xAOD::TrackMeasurementValidationContainer, int>
      cleanDeco(m_cleanHitKey, ctx);

    std::optional<SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, char>>
      isFakeHandle;
    std::optional<SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>>
      hasBSErrHandle;
    std::optional<SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>>
      DCSStateHandle;
    if (!m_isSCT) {
      isFakeHandle.emplace(m_isFakeKey, ctx);
      hasBSErrHandle.emplace(m_hasBSErrKey, ctx);
      DCSStateHandle.emplace(m_DCSStateKey, ctx);
    }

    // Without overlap removal the flag depends only on the hit itself, so neither
    // the geometry nor the ordering is needed
    if (!m_doOverlapRemoval) {
      for (const xAOD::TrackMeasurementValidation* hit : *hits) {
        cleanDeco(*hit) =
          (m_isSCT || isGoodHit(hit, *isFakeHandle, *hasBSErrHandle, *DCSStateHandle)) ? 1 : 0;
      }
      return StatusCode::SUCCESS;
    }

    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>
      becHandle(m_becKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, unsigned long>
      deidHandle(m_deidKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackMeasurementValidationContainer, int>
      layerHandle(m_layerKey, ctx);

    std::vector<std::pair<float, const xAOD::TrackMeasurementValidation*>> hits_sorted;
    hits_sorted.reserve(hits->size());

    for (const xAOD::TrackMeasurementValidation* hit : *hits) {
      hits_sorted.emplace_back(hit->localX(), hit);
    }

    std::ranges::sort(hits_sorted);

    std::vector<std::vector<HitPos>> savedHits;

    for (const auto& [localX, hit] : hits_sorted) {

      int cleanFlag = 1;

      // Extract geometry
      HitPos hp;
      hp.z = hit->globalZ();
      hp.phi = std::atan2(hit->globalY(), hit->globalX());
      hp.deid = deidHandle(*hit);
      hp.layer = layerHandle(*hit);

      const bool isBarrel = (becHandle(*hit) == 0);

      // Overlap evaluation
      const float dZcut   = 20.f;
      const float dPhicut = isBarrel ? 0.004f : std::numeric_limits<float>::max();

      if (static_cast<size_t>(hp.layer) >= savedHits.size()) {
        savedHits.resize(hp.layer + 1);
      }

      auto& layerHits = savedHits[hp.layer];

      for (const HitPos& prev : layerHits) {

        // Skip hits from the same module
        if (prev.deid == hp.deid)
          continue;

        const float dphi = std::abs(deltaPhi(prev, hp));
        const float dz   = std::abs(prev.z - hp.z);

        if (dphi < dPhicut && dz < dZcut) {
          cleanFlag = 0;
          break;
        }
      }

      // Mark bad hits
      if (!m_isSCT && !isGoodHit(hit, *isFakeHandle, *hasBSErrHandle, *DCSStateHandle)) {
        cleanFlag = 0;
      }

      // Write decoration
      cleanDeco(*hit) = cleanFlag;

      // Save for next comparisons
      layerHits.push_back(hp);
    }

    return StatusCode::SUCCESS;
  }

}
