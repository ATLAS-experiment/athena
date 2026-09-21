/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGNN_DETAIL_GNNFEATURES_H
#define ACTSGNN_DETAIL_GNNFEATURES_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <numeric>
#include <span>
#include <vector>

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Utilities/VectorHelpers.hpp"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

#include "src/GnnPipelineTool.h"

namespace ActsTrk::detail {

inline int computeOverlapSpFlag(int etaModuleCl1, int phiModuleCl1,
                                int etaModuleCl2, int phiModuleCl2) {
  if (etaModuleCl1 == etaModuleCl2 && phiModuleCl1 == phiModuleCl2) {
    return 0;  // not an overlap spacepoint
  }
  if (etaModuleCl1 != etaModuleCl2 && phiModuleCl1 == phiModuleCl2) {
    return 1;  // overlap in eta only
  }
  if (etaModuleCl1 == etaModuleCl2 && phiModuleCl1 != phiModuleCl2) {
    return 2;  // overlap in phi only
  }
  return 3;  // overlap in eta and phi
}

}  // namespace ActsTrk::detail

namespace ActsTrk {

inline StatusCode GnnPipelineTool::buildFeatures(
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    std::vector<float>& features, std::vector<std::uint64_t>& moduleIds,
    std::vector<int>& ids,
    std::vector<const xAOD::SpacePoint*>& allSPPtrs,
    std::size_t nFeatures) const {

  std::size_t nSP = 0;
  for (const auto* spc : spacePointCollections) {
    nSP += spc->size();
  }
  moduleIds.reserve(nSP);
  allSPPtrs.reserve(nSP);

  std::size_t skipped = 0;
  for (const auto* spc : spacePointCollections) {
    for (const auto* sp : *spc) {
      const auto* cl1 = sp->measurements().front();
      Identifier atlasIdCl1(
          static_cast<Identifier::value_type>(cl1->identifier()));

      if (!m_usePhiOverlapSps.value() && sp->measurements().size() == 2) {
        const auto* cl2 = sp->measurements().at(1);
        Identifier atlasIdCl2(
            static_cast<Identifier::value_type>(cl2->identifier()));

        int overlapFlag = detail::computeOverlapSpFlag(
            m_stripIdHelper->eta_module(atlasIdCl1),
            m_stripIdHelper->phi_module(atlasIdCl1),
            m_stripIdHelper->eta_module(atlasIdCl2),
            m_stripIdHelper->phi_module(atlasIdCl2));

        if (overlapFlag == 2 || overlapFlag == 3) {
          ++skipped;
          ACTS_VERBOSE("Skip phi overlap spacepoint (flag=" << overlapFlag
                                                            << ")");
          continue;
        }
      }

      Identifier waferIdCl1 =
          cl1->type() == xAOD::UncalibMeasType::PixelClusterType
              ? m_pixelIdHelper->wafer_id(atlasIdCl1)
              : m_stripIdHelper->wafer_id(atlasIdCl1);
      moduleIds.push_back(waferIdCl1.get_compact());
      allSPPtrs.push_back(sp);
    }
  }

  ACTS_DEBUG("Skipped " << skipped << " SPs because of phi overlap");
  nSP = allSPPtrs.size();
  ACTS_DEBUG("Keep " << nSP << " SPs for feature creation");

  std::vector<std::size_t> idxs(nSP);
  std::iota(idxs.begin(), idxs.end(), 0);
  std::ranges::sort(
      idxs, [&](auto a, auto b) { return moduleIds.at(a) < moduleIds.at(b); });
  std::ranges::sort(moduleIds);

  // Reorder the space point pointers to match the sorted module ids, so that
  // the node ids returned by the pipeline index allSPPtrs directly
  std::vector<const xAOD::SpacePoint*> sortedSPPtrs(nSP);
  for (std::size_t k = 0; k < nSP; ++k) {
    sortedSPPtrs.at(k) = allSPPtrs.at(idxs.at(k));
  }
  allSPPtrs.swap(sortedSPPtrs);

  features.assign(nFeatures * nSP, 0.f);
  ids.resize(nSP);

  for (std::size_t k = 0; k < nSP; ++k) {
    ids.at(k) = static_cast<int>(k);

    std::span<float> f(features.data() + k * nFeatures, nFeatures);
    const auto& sp = *allSPPtrs.at(k);

    using namespace Acts::VectorHelpers;

    Acts::Vector3 spp{sp.x(), sp.y(), sp.z()};

    if (sp.measurements().size() == 1) {
      for (std::size_t j = 0; j < nFeatures; j += 4) {
        f[j + 0] = perp(spp) / 1000.f;
        f[j + 1] = phi(spp) / std::numbers::pi_v<float>;
        f[j + 2] = sp.z() / 1000.f;
        f[j + 3] = eta(spp);
      }
    } else {
      std::size_t j = 0;
      f[j + 0] = perp(spp) / 1000.f;
      f[j + 1] = phi(spp) / std::numbers::pi_v<float>;
      f[j + 2] = sp.z() / 1000.f;
      f[j + 3] = eta(spp);

      for (const auto* m : sp.measurements()) {
        const auto* cl = static_cast<const xAOD::StripCluster*>(m);
        auto gp = cl->globalPosition();
        j += 4;
        f[j + 0] = perp(gp) / 1000.f;
        f[j + 1] = phi(gp) / std::numbers::pi_v<float>;
        f[j + 2] = gp.z() / 1000.f;
        f[j + 3] = eta(gp);
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk

#endif
