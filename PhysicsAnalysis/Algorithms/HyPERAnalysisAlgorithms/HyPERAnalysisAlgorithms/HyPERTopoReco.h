/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HYPERANALYSISALGORITHMS_HYPERTOPORECO_H
#define HYPERANALYSISALGORITHMS_HYPERTOPORECO_H

/// @author Zihan Zhang, Diego Baron

#include <iostream>
#include <memory>
#include <vector>

#include "HyPERAnalysisAlgorithms/HyPERGraph.h"

using indices = std::vector<std::vector<int64_t>>;
using scores = std::vector<std::vector<float>>;

namespace EventReco {
void RecoTtbarAllHadronic(const HyPERGraph& hyperGraph,
                          const scores& edge_scores,
                          const scores& hyperedge_scores, indices& reco_indices,
                          std::vector<float>& reco_scores,
                          std::vector<std::string>& reco_labels);
void RecoTtbarLJets(const HyPERGraph& hyperGraph, const scores& edge_scores,
                    const scores& hyperedge_scores,
                    const scores& classification_score, indices& reco_indices,
                    std::vector<float>& reco_scores,
                    std::vector<std::string>& reco_labels,
                    std::vector<std::vector<int64_t>>& reco_ids,
                    float& reco_classification_score);
void RecoTtbarDiLepton(const HyPERGraph& hyperGraph, const scores& edge_scores,
                       const scores& hyperedge_scores,
                       const scores& classification_score,
                       indices& reco_indices, std::vector<float>& reco_scores,
                       std::vector<std::string>& reco_labels,
                       std::vector<std::vector<int64_t>>& reco_ids,
                       float& reco_classification_score);

// Utility functions that are shared with VyPER
bool anyCommonElement(const std::vector<int64_t>& vec1,
                      const std::vector<int64_t>& vec2);
bool isAllowedDecay(
    std::vector<int64_t>& decay_indices,
    const std::vector<std::vector<int64_t>>& allowed_decay_modes);

}  // namespace EventReco

#endif  // HYPERANALYSISALGORITHMS_HYPERTOPORECO_H
