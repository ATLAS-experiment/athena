/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONVALIDATION_TRUTHMUONSELECTION_H
#define L1MUONVALIDATION_TRUTHMUONSELECTION_H

#include <cmath>
#include <limits>

namespace L1Muon {

/** @brief Subsystem-independent generator-muon selection used by validation.
 *
 * The selection is deliberately expressed only in terms of basic generator
 * quantities so that TGC/RPC/MDT reference-position branches can reuse the
 * exact same reference-muon definition without depending on one another.
 */
struct TruthMuonSelectionConfig {
  double minPtMeV{0.0};
  double minAbsEta{0.0};
  double maxAbsEta{std::numeric_limits<double>::max()};
  int requiredStatus{1};
  int maxAbsBarcode{9999};
};

inline bool acceptTruthMuon(const int pdgId, const int status,
                            const int barcode, const double ptMeV,
                            const double eta, const double phi,
                            const TruthMuonSelectionConfig& config) {
  return std::abs(pdgId) == 13 && status == config.requiredStatus &&
         std::abs(barcode) <= config.maxAbsBarcode &&
         std::isfinite(ptMeV) && std::isfinite(eta) && std::isfinite(phi) &&
         ptMeV >= config.minPtMeV &&
         std::abs(eta) >= config.minAbsEta &&
         std::abs(eta) <= config.maxAbsEta;
}

}  // namespace L1Muon

#endif
