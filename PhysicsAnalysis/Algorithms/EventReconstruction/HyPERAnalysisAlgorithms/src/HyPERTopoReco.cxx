/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <AsgMessaging/MessageCheck.h>  // To access ANA_MSG


#include "HyPERAnalysisAlgorithms/HyPERTopoReco.h"
#include "HyPERAnalysisAlgorithms/HyPERUtils.h"

using indices = std::vector<std::vector<int64_t>>;
using scores = std::vector<std::vector<float>>;

namespace EventReco {
bool anyCommonElement(const std::vector<int64_t>& vec1,
                      const std::vector<int64_t>& vec2) {
  for (const auto& v1 : vec1) {
    for (const auto& v2 : vec2) {
      if (v1 == v2)
        return true;
    }
  }
  return false;
}

bool isAllowedDecay(
    std::vector<int64_t>& decay_indices,
    const std::vector<std::vector<int64_t>>& allowed_decay_modes) {
  // Search for the decay mode in the allowed decay modes
  // Create a sorted copy of the decay indices and the decay mode
  std::vector<int64_t> decay_indices_sorted = decay_indices;
  std::sort(decay_indices_sorted.begin(), decay_indices_sorted.end());
  for (const auto& decay_mode : allowed_decay_modes) {
    std::vector<int64_t> decay_mode_sorted = decay_mode;
    std::sort(decay_mode_sorted.begin(), decay_mode_sorted.end());
    // Check if the decay mode is compatible with the allowed decay mode
    if (decay_indices_sorted == decay_mode_sorted)
      return true;
  }
  return false;
}

bool isHadTop(const std::vector<int64_t>& decay_indices) {
  // Check if the decay mode is compatible with a hadronic top quark
  if (decay_indices.size() != 3)
    throw std::runtime_error(
        "Number of decay particles is not consistent with a hadronic top "
        "decay!");
  for (const auto& decay_index : decay_indices) {
    if (decay_index != EventReco::HyPERParticleID::jet)
      return false;
  }
  return true;
}

std::vector<int64_t> findWInTopLep(const std::vector<int64_t>& decay_indices,
                                   const std::vector<int64_t>& decay_ids) {
  std::vector<int64_t> w_candidate = {};
  if (decay_indices.size() != decay_ids.size())
    throw std::runtime_error(
        "Number of decay particles and IDs are not consistent!");
  if (decay_indices.size() != 3)
    throw std::runtime_error(
        "Number of decay particles is not consistent with a leptonic top "
        "decay!");

  for (std::size_t i = 0; i < decay_indices.size(); i++) {
    if (decay_ids.at(i) == EventReco::HyPERParticleID::jet)
      continue;
    w_candidate.push_back(decay_indices.at(i));
  }

  if (w_candidate.size() != 2)
    throw std::runtime_error(
        "Number of W boson decay particles is not consistent with a leptonic "
        "top decay!");
  return w_candidate;
}

int countParticlesSameType(const HyPERGraph& hyperGraph,
                           EventReco::HyPERParticleID particleType) {
  int count = 0;
  for (std::size_t i = 0; i < static_cast<std::size_t>(hyperGraph.nNodes());
       i++) {
    // Particle ID should be at the position before the last.
    std::size_t beforeLastPosition = hyperGraph.getNodeFeats(i).size() - 2;
    if (hyperGraph.getNodeFeats(i).at(beforeLastPosition) ==
        static_cast<float>(particleType))
      count++;
  }

  return count;
}

// This overload is for the DiLepton topology where the IDs are scaled by 2 in
// the input features. See
int countParticlesSameType(const HyPERGraph& hyperGraph, float particleType) {
  int count = 0;
  for (std::size_t i = 0; i < static_cast<std::size_t>(hyperGraph.nNodes());
       i++) {
    // Particle ID should be at the position before the last.
    std::size_t beforeLastPosition = hyperGraph.getNodeFeats(i).size() - 2;
    if (hyperGraph.getNodeFeats(i).at(beforeLastPosition) ==
        static_cast<float>(particleType))
      count++;
  }

  return count;
}

/**
 * @brief Re-scale the indices of the reconstructed leptonic top decay to
 * reflect the position in the particle-specific containers.
 */
void reescaleIndicesTtbarLJets(const HyPERGraph& hyperGraph,
                               std::vector<int64_t>& top_indices,
                               std::vector<int64_t>& w_indices,
                               const std::vector<int64_t>& ids) {
  int nJets = countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::jet);
  int nElectrons = countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::e);
  // int nMuons = countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::mu);
  // // When tau is supported...
  int nMETs = countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::met);

  // Sanity check
  if (nMETs != 1)
    throw std::runtime_error("Number of METs is not consistent!");

  // Reescale the indices
  for (std::size_t i = 0; i < top_indices.size(); i++) {
    if (ids.at(i) ==
        EventReco::HyPERParticleID::jet) {  // For jets do nothing since they are
                                      // filled first and then their positions
                                      // are the same as in the original
                                      // container.
      continue;
    } else if (ids.at(i) == EventReco::HyPERParticleID::e) {
      top_indices.at(i) -= nJets;
      w_indices.at(0) = top_indices.at(
          i);  // Always put the lepton as the first in the w_indices.
    } else if (ids.at(i) == EventReco::HyPERParticleID::mu) {
      top_indices.at(i) -= (nJets + nElectrons);
      w_indices.at(0) = top_indices.at(
          i);  // Always put the lepton as the first in the w_indices.
    } else if (ids.at(i) == EventReco::HyPERParticleID::met) {
      top_indices.at(i) =
          0;  // Just assign 0 here since MET is always the first particle.
      w_indices.at(1) =
          0;  // always put the MET as the second in the w_indices.
    }
  }
}

void reescaleIndicesTtbarDiLepton(const HyPERGraph& hyperGraph,
                                  std::vector<int64_t>& top_indices,
                                  const std::vector<int64_t>& ids) {
  // Divide by 2 because of the feature scaling applied to the IDs in
  // RunHyPERAlg::buildTtbarDiLeptonGraph. See the comments there for more
  // details.
  int nJets =
      countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::jet / 2.0f);
  int nElectrons =
      countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::e / 2.0f);
  // int nMuons = countParticlesSameType(hyperGraph,
  // EventReco::HyPERParticleID::mu/2.0f);
  // // When tau is supported...

  // Sanity check. In di-lepton there should be no METs counted as particles.
  int nMETs =
      countParticlesSameType(hyperGraph, EventReco::HyPERParticleID::met / 2.0f);
  if (nMETs != 1)
    throw std::runtime_error("Number of METs is not consistent!");

  // Reescale the indices
  for (std::size_t i = 0; i < top_indices.size(); i++) {
    if (ids.at(i) ==
        EventReco::HyPERParticleID::jet) {  // For jets do nothing since they are
                                      // filled first and then their positions
                                      // are the same as in the original
                                      // container.
      continue;
    } else if (ids.at(i) == EventReco::HyPERParticleID::e) {
      top_indices.at(i) -= nJets;
    } else if (ids.at(i) == EventReco::HyPERParticleID::mu) {
      top_indices.at(i) -= (nJets + nElectrons);
    }  // When tau is supported...
  }
}

/**
 * @brief Reconstruct the hadronic top quark (with the highest score) and W
 * boson from the hypergraph score outputs. Tops already reconstructed are
 * masked out.
 */
void RecoHadronicTop(
    const HyPERGraph& hyperGraph, const scores& edge_scores,
    const scores&
        hyperedge_scores,  // This is the hyperedge output from the model.
    std::vector<int64_t>&
        hyperedge_masks,  // This is a vector of 0s and 1s, where 0 means the
                          // hyperedge is already in use.
    std::vector<int64_t>& top_reco_indices,  // This we want to fill with the
                                             // indices of the top quark.
    std::vector<int64_t>& w_reco_indices,    // This we want to fill with the
                                             // indices of the W boson.
    float& top_reco_score,  // This we want to fill with the score of the top
                            // quark.
    float&
        w_reco_score) {  // This we want to fill with the score of the W boson.

  if (hyperGraph.nHyperEdges() != static_cast<int>(hyperedge_scores.size()))
    throw std::runtime_error(
        "Number of defined hyperedges does not match with the ones from model "
        "output!");
  if (hyperGraph.nEdges() != static_cast<int>(edge_scores.size()))
    throw std::runtime_error(
        "Number of defined graph edges does not match with the ones from model "
        "output!");

  for (std::size_t i = 0; i < hyperedge_scores.size(); i++) {
    if (hyperedge_masks.at(i) == 0)
      continue;  // Skip the hyperedge if it is masked

    if (hyperedge_scores.at(i).at(0) > top_reco_score) {
      top_reco_score = hyperedge_scores.at(i).at(0);
      top_reco_indices = {hyperGraph.getHyperEdgeIndices(i).at(0),
                          hyperGraph.getHyperEdgeIndices(i).at(1),
                          hyperGraph.getHyperEdgeIndices(i).at(2)};
      w_reco_score = 0.0;  // Reset the Ws if a new best top is found.
      w_reco_indices = {-1, -1};

      std::vector<std::vector<int64_t>> w_candidates =
          buildCombinations<int64_t>(top_reco_indices, 2);
      for (std::size_t j = 0; j < w_candidates.size(); j++) {
        // There are two directional edges connecting each pair of nodes
        float bi_directional_sum = 0;
        for (std::size_t k = 0; k < edge_scores.size(); k++) {
          if (w_candidates.at(j).at(0) ==
                  hyperGraph.getEdgeIndicesVector().at(k).first &&
              w_candidates.at(j).at(1) ==
                  hyperGraph.getEdgeIndicesVector().at(k).second) {
            bi_directional_sum += edge_scores.at(k).at(0);
          }
          if (w_candidates.at(j).at(1) ==
                  hyperGraph.getEdgeIndicesVector().at(k).first &&
              w_candidates.at(j).at(0) ==
                  hyperGraph.getEdgeIndicesVector().at(k).second) {
            bi_directional_sum += edge_scores.at(k).at(0);
          }
        }
        if (bi_directional_sum / 2 > w_reco_score) {
          w_reco_score = bi_directional_sum / 2;
          w_reco_indices = w_candidates.at(j);
        }
      }
    }
  }
}

/**
 * @brief Reconstruct a top quark with a decay compatible with a list of decay
 * modes specified by a list of indices. Tops already reconstructed are masked
 * out.
 */
void RecoTop(
    const HyPERGraph& hyperGraph, const scores& edge_scores,
    const scores&
        hyperedge_scores,  // This is the hyperedge output from the model.
    std::vector<int64_t>&
        hyperedge_masks,  // This is a vector of 0s and 1s, where 0 means the
                          // hyperedge is already in use.
    std::vector<int64_t>&
        top_reco_indices,  // This we want to fill with the indices of particles
                           // in the top quark.
    std::vector<int64_t>&
        w_reco_indices,  // This we want to fill with the indices of particles
                         // in the W boson.
    float& top_reco_score,  // This we want to fill with the score of the top
                            // quark.
    float& w_reco_score,
    std::vector<int64_t>& top_reco_ids,  // This will hold the particle IDs of
                                         // the top quark decays.
    const std::vector<std::vector<int64_t>>&
        allowed_decay_modes) {  // This is the list of decay modes we want to
                                // consider.
  if (hyperGraph.nHyperEdges() != static_cast<int>(hyperedge_scores.size()))
    throw std::runtime_error(
        "Number of defined hyperedges does not match with the ones from model "
        "output!");
  if (hyperGraph.nEdges() != static_cast<int>(edge_scores.size()))
    throw std::runtime_error(
        "Number of defined graph edges does not match with the ones from model "
        "output!");
  if (allowed_decay_modes.empty())
    throw std::runtime_error("No allowed decay modes available!");

  for (std::size_t i = 0; i < hyperedge_scores.size(); i++) {
    if (hyperedge_masks.at(i) == 0)
      continue;  // Skip the hyperedge if it is masked
    if (hyperedge_scores.at(i).at(0) > top_reco_score) {

      std::vector<int64_t> top_candidate_indices = {
          hyperGraph.getHyperEdgeIndices(i).at(0),
          hyperGraph.getHyperEdgeIndices(i).at(1),
          hyperGraph.getHyperEdgeIndices(i).at(2)};
      // Get particle IDs. The conversions here are float -> HyPERParticleID
      // (int) -> int64_t
      // The ID should be the element before the last in the node features. See
      // RunHyPERAlg::buildTtbarLJetsGraph
      std::size_t beforeLastPosition = hyperGraph.getNodeFeats(0).size() - 2;
      EventReco::HyPERParticleID id_1 = static_cast<EventReco::HyPERParticleID>(
          hyperGraph.getNodeFeats(top_candidate_indices.at(0))
              .at(beforeLastPosition));
      EventReco::HyPERParticleID id_2 = static_cast<EventReco::HyPERParticleID>(
          hyperGraph.getNodeFeats(top_candidate_indices.at(1))
              .at(beforeLastPosition));
      EventReco::HyPERParticleID id_3 = static_cast<EventReco::HyPERParticleID>(
          hyperGraph.getNodeFeats(top_candidate_indices.at(2))
              .at(beforeLastPosition));
      std::vector<int64_t> top_candidate_ids = {static_cast<int64_t>(id_1),
                                                static_cast<int64_t>(id_2),
                                                static_cast<int64_t>(id_3)};
      if (!isAllowedDecay(top_candidate_ids, allowed_decay_modes))
        continue;

      top_reco_score = hyperedge_scores.at(i).at(0);
      top_reco_ids = std::move(top_candidate_ids);
      top_reco_indices = std::move(top_candidate_indices);
      w_reco_score = 0.0;  // Reset the Ws if a new best top is found.
      w_reco_indices = {-1, -1};

      if (isHadTop(top_reco_ids)) {  // Do the same as the had top reco
        std::vector<std::vector<int64_t>> w_candidates =
            buildCombinations<int64_t>(top_reco_indices, 2);
        for (std::size_t j = 0; j < w_candidates.size(); j++) {
          // There are two directional edges connecting each pair of nodes
          float bi_directional_sum = 0;
          for (std::size_t k = 0; k < edge_scores.size(); k++) {
            if (w_candidates.at(j).at(0) ==
                    hyperGraph.getEdgeIndicesVector().at(k).first &&
                w_candidates.at(j).at(1) ==
                    hyperGraph.getEdgeIndicesVector().at(k).second) {
              bi_directional_sum += edge_scores.at(k).at(0);
            }
            if (w_candidates.at(j).at(1) ==
                    hyperGraph.getEdgeIndicesVector().at(k).first &&
                w_candidates.at(j).at(0) ==
                    hyperGraph.getEdgeIndicesVector().at(k).second) {
              bi_directional_sum += edge_scores.at(k).at(0);
            }
          }
          if (bi_directional_sum / 2 > w_reco_score) {
            w_reco_score = bi_directional_sum / 2;
            w_reco_indices = w_candidates.at(j);
          }
        }
      } else {  // Just look for the hadronic index and the rest is the W boson
        w_reco_indices = findWInTopLep(top_reco_indices, top_reco_ids);
        float bi_directional_sum = 0;
        for (std::size_t k = 0; k < edge_scores.size(); k++) {
          if (w_reco_indices.at(0) ==
                  hyperGraph.getEdgeIndicesVector().at(k).first &&
              w_reco_indices.at(1) ==
                  hyperGraph.getEdgeIndicesVector().at(k).second) {
            bi_directional_sum += edge_scores.at(k).at(0);
          }
          if (w_reco_indices.at(0) ==
                  hyperGraph.getEdgeIndicesVector().at(k).second &&
              w_reco_indices.at(1) ==
                  hyperGraph.getEdgeIndicesVector().at(k).first) {
            bi_directional_sum += edge_scores.at(k).at(0);
          }
        }
        w_reco_score = bi_directional_sum / 2;
      }
    }
  }
}

/**
 * @brief Reconstruct the ttbar all hadronic topology.
 */
void RecoTtbarAllHadronic(const HyPERGraph& hyperGraph,
                          const scores& edge_scores,
                          const scores& hyperedge_scores, indices& reco_indices,
                          std::vector<float>& reco_scores,
                          std::vector<std::string>& reco_labels) {

  std::vector<int64_t> top1_reco_indices(3, -1);
  std::vector<int64_t> top2_reco_indices(3, -1);
  std::vector<int64_t> w1_reco_indices(2, -1);
  std::vector<int64_t> w2_reco_indices(2, -1);

  float top1_reco_score = 0;
  float top2_reco_score = 0;
  float w1_reco_score = 0;
  float w2_reco_score = 0;

  // Reconstructing top1
  std::vector<int64_t> top1_masks(hyperedge_scores.size(), 1);
  RecoHadronicTop(hyperGraph, edge_scores, hyperedge_scores, top1_masks,
                  top1_reco_indices, w1_reco_indices, top1_reco_score,
                  w1_reco_score);

  // Check if reco top1 is successful
  if (top1_reco_indices == std::vector<int64_t>(3, -1)) {
    reco_labels.emplace_back("HyPER_Reco_Top1");
    reco_indices.push_back(std::move(top1_reco_indices));
    reco_scores.push_back(top1_reco_score);
    reco_labels.emplace_back("HyPER_Reco_Top2");
    reco_indices.push_back(std::move(top2_reco_indices));
    reco_scores.push_back(top2_reco_score);
    reco_labels.emplace_back("HyPER_Reco_W1");
    reco_indices.push_back(std::move(w1_reco_indices));
    reco_scores.push_back(w1_reco_score);
    reco_labels.emplace_back("HyPER_Reco_W2");
    reco_indices.push_back(std::move(w2_reco_indices));
    reco_scores.push_back(w2_reco_score);
    return;  // No point in continuing if top1 is not reconstructed.
  }

  // Masking out the hyperedges containing nodes in top1
  std::vector<int64_t> top2_masks;
  for (std::size_t i = 0;
       i < static_cast<std::size_t>(hyperGraph.nHyperEdges()); i++) {
    if (anyCommonElement(top1_reco_indices, hyperGraph.getHyperEdgeIndices(i)))
      top2_masks.push_back(0);
    else
      top2_masks.push_back(1);
  }

  // Reconstructing top2
  RecoHadronicTop(hyperGraph, edge_scores, hyperedge_scores, top2_masks,
                  top2_reco_indices, w2_reco_indices, top2_reco_score,
                  w2_reco_score);

  // Save results
  reco_labels.push_back("HyPER_Reco_Top1");
  reco_indices.push_back(std::move(top1_reco_indices));
  reco_scores.push_back(top1_reco_score);
  reco_labels.push_back("HyPER_Reco_Top2");
  reco_indices.push_back(std::move(top2_reco_indices));
  reco_scores.push_back(top2_reco_score);
  reco_labels.push_back("HyPER_Reco_W1");
  reco_indices.push_back(std::move(w1_reco_indices));
  reco_scores.push_back(w1_reco_score);
  reco_labels.push_back("HyPER_Reco_W2");
  reco_indices.push_back(std::move(w2_reco_indices));
  reco_scores.push_back(w2_reco_score);
}

/**
 * @brief Reconstruct the ttbar single lepton topology.
 */
void RecoTtbarLJets(const HyPERGraph& hyperGraph, const scores& edge_scores,
                    const scores& hyperedge_scores,
                    const scores& classification_score, indices& reco_indices,
                    std::vector<float>& reco_scores,
                    std::vector<std::string>& reco_labels,
                    std::vector<std::vector<int64_t>>& reco_ids,
                    float& reco_classification_score) {
  using namespace asg::msgUserCode;

  std::vector<int64_t> top1_reco_indices(3, -1);
  std::vector<int64_t> top2_reco_indices(3, -1);
  std::vector<int64_t> w1_reco_indices(2, -1);
  std::vector<int64_t> w2_reco_indices(2, -1);

  float top1_reco_score = 0;
  float top2_reco_score = 0;
  float w1_reco_score = 0;
  float w2_reco_score = 0;

  std::vector<int64_t> top1_reco_ids(3, -1);
  std::vector<int64_t> top2_reco_ids(3, -1);

  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    setMsgLevel(MSG::INFO);
    ANA_MSG_INFO("Reconstructing TtbarSingleLepton top quarks...");
    ANA_MSG_INFO("Working with these HE scores:");
    for (const auto& he_score : hyperedge_scores) {
      ANA_MSG_INFO("Element " << he_score.at(0));
    }
    ANA_MSG_INFO("Working with these edge scores:");
    for (const auto& e_score : edge_scores) {
      ANA_MSG_INFO("Element " << e_score.at(0));
    }
    ANA_MSG_INFO("Working with this classification score:");
    ANA_MSG_INFO(classification_score.at(0).at(0));
  }

  // Save the classification score
  reco_classification_score = classification_score.at(0).at(0);

  // Allowed decays
  std::vector<std::vector<int64_t>> allowed_decays = {
      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::jet,
       EventReco::HyPERParticleID::jet},
      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::e,
       EventReco::HyPERParticleID::met},
      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::mu,
       EventReco::HyPERParticleID::met}};

  // Reconstructing top1
  std::vector<int64_t> top1_masks(hyperedge_scores.size(), 1);
  RecoTop(hyperGraph, edge_scores, hyperedge_scores, top1_masks,
          top1_reco_indices, w1_reco_indices, top1_reco_score, w1_reco_score,
          top1_reco_ids, allowed_decays);
  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Top1 reco indices: " << top1_reco_indices.at(0) << " "
                                       << top1_reco_indices.at(1) << " "
                                       << top1_reco_indices.at(2));
    ANA_MSG_INFO("Top1 reco score: " << top1_reco_score);
    ANA_MSG_INFO("W1 reco indices: " << w1_reco_indices.at(0) << " "
                                     << w1_reco_indices.at(1));
    ANA_MSG_INFO("W1 reco score: " << w1_reco_score);
  }

  // Check if reco top1 is successful
  if (top1_reco_indices == std::vector<int64_t>(3, -1)) {
    // If not, return the default values for both tops.
    reco_labels.push_back("HyPER_Reco_TopHad");
    reco_indices.push_back(std::move(top1_reco_indices));
    reco_scores.push_back(top1_reco_score);
    reco_ids.push_back(std::move(top1_reco_ids));
    reco_labels.push_back("HyPER_Reco_TopLep");
    reco_indices.push_back(std::move(top2_reco_indices));
    reco_scores.push_back(top2_reco_score);
    reco_ids.push_back(std::move(top2_reco_ids));
    reco_labels.push_back("HyPER_Reco_WHad");
    reco_indices.push_back(std::move(w1_reco_indices));
    reco_scores.push_back(w1_reco_score);
    reco_labels.push_back("HyPER_Reco_WLep");
    reco_indices.push_back(std::move(w2_reco_indices));
    reco_scores.push_back(w2_reco_score);
    return;
  }

  // Masking out the hyperedges containing nodes in top1
  std::vector<int64_t> top2_masks;
  for (std::size_t i = 0;
       i < static_cast<std::size_t>(hyperGraph.nHyperEdges()); i++) {
    if (anyCommonElement(top1_reco_indices, hyperGraph.getHyperEdgeIndices(i)))
      top2_masks.push_back(0);
    else
      top2_masks.push_back(1);
  }

  // If the top1 is hadronic, redefine the allowed decays and conversely
  bool top1_is_hadronic = false;
  if (isHadTop(top1_reco_ids)) {
    if (EventReco::g_hyper_msg_level == MSG::VERBOSE)
      ANA_MSG_INFO("Top1 is hadronic!");
    top1_is_hadronic = true;
    allowed_decays = {{EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::e,
                       EventReco::HyPERParticleID::met},
                      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::mu,
                       EventReco::HyPERParticleID::met}};
  } else {
    if (EventReco::g_hyper_msg_level == MSG::VERBOSE)
      ANA_MSG_INFO("Top1 is Leptonic!");
    allowed_decays = {{EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::jet,
                       EventReco::HyPERParticleID::jet}};
  }

  // Reconstructing top2
  RecoTop(hyperGraph, edge_scores, hyperedge_scores, top2_masks,
          top2_reco_indices, w2_reco_indices, top2_reco_score, w2_reco_score,
          top2_reco_ids, allowed_decays);
  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Top2 reco indices: " << top2_reco_indices.at(0) << " "
                                       << top2_reco_indices.at(1) << " "
                                       << top2_reco_indices.at(2));
    ANA_MSG_INFO("Top2 reco score: " << top2_reco_score);
    ANA_MSG_INFO("W2 reco indices: " << w2_reco_indices.at(0) << " "
                                     << w2_reco_indices.at(1));
    ANA_MSG_INFO("W2 reco score: " << w2_reco_score);
  }

  // Check if reco top2 is successful
  bool both_tops_reco = true;
  if (top2_reco_indices == std::vector<int64_t>(3, -1))
    both_tops_reco = false;

  // Finally, we need to shift the indices in the leptonic top decay to reflect
  // the position in the particle-specific containers.
  if (top1_is_hadronic && both_tops_reco) {
    reescaleIndicesTtbarLJets(hyperGraph, top2_reco_indices, w2_reco_indices,
                              top2_reco_ids);
  }
  if (!top1_is_hadronic) {
    reescaleIndicesTtbarLJets(hyperGraph, top1_reco_indices, w1_reco_indices,
                              top1_reco_ids);
  }

  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Final reco indices:");
    ANA_MSG_INFO("Top1 reco indices: " << top1_reco_indices.at(0) << " "
                                       << top1_reco_indices.at(1) << " "
                                       << top1_reco_indices.at(2));
    ANA_MSG_INFO("Top2 reco indices: " << top2_reco_indices.at(0) << " "
                                       << top2_reco_indices.at(1) << " "
                                       << top2_reco_indices.at(2));
    ANA_MSG_INFO("W1 reco indices: " << w1_reco_indices.at(0) << " "
                                     << w1_reco_indices.at(1));
    ANA_MSG_INFO("W2 reco indices: " << w2_reco_indices.at(0) << " "
                                     << w2_reco_indices.at(1));
    ANA_MSG_INFO("Final reco scores:");
    ANA_MSG_INFO("Top1 reco score: " << top1_reco_score);
    ANA_MSG_INFO("Top2 reco score: " << top2_reco_score);
    ANA_MSG_INFO("W1 reco score: " << w1_reco_score);
    ANA_MSG_INFO("W2 reco score: " << w2_reco_score);
    ANA_MSG_INFO("Final reco IDs:");
    ANA_MSG_INFO("Top1 reco IDs: " << top1_reco_ids.at(0) << " "
                                   << top1_reco_ids.at(1) << " "
                                   << top1_reco_ids.at(2));
    ANA_MSG_INFO("Top2 reco IDs: " << top2_reco_ids.at(0) << " "
                                   << top2_reco_ids.at(1) << " "
                                   << top2_reco_ids.at(2));
  }

  // Save results
  if (top1_is_hadronic) {
    reco_labels.push_back("HyPER_Reco_TopHad");
    reco_indices.push_back(std::move(top1_reco_indices));
    reco_scores.push_back(top1_reco_score);
    reco_ids.push_back(std::move(top1_reco_ids));
    reco_labels.push_back("HyPER_Reco_TopLep");
    reco_indices.push_back(std::move(top2_reco_indices));
    reco_scores.push_back(top2_reco_score);
    reco_ids.push_back(std::move(top2_reco_ids));
    reco_labels.push_back("HyPER_Reco_WHad");
    reco_indices.push_back(std::move(w1_reco_indices));
    reco_scores.push_back(w1_reco_score);
    reco_labels.push_back("HyPER_Reco_WLep");
    reco_indices.push_back(std::move(w2_reco_indices));
    reco_scores.push_back(w2_reco_score);
  } else {
    reco_labels.push_back("HyPER_Reco_TopHad");
    reco_indices.push_back(std::move(top2_reco_indices));
    reco_scores.push_back(top2_reco_score);
    reco_ids.push_back(std::move(top2_reco_ids));
    reco_labels.push_back("HyPER_Reco_TopLep");
    reco_indices.push_back(std::move(top1_reco_indices));
    reco_scores.push_back(top1_reco_score);
    reco_ids.push_back(std::move(top1_reco_ids));
    reco_labels.push_back("HyPER_Reco_WHad");
    reco_indices.push_back(std::move(w2_reco_indices));
    reco_scores.push_back(w2_reco_score);
    reco_labels.push_back("HyPER_Reco_WLep");
    reco_indices.push_back(std::move(w1_reco_indices));
    reco_scores.push_back(w1_reco_score);
  }
}

/**
 * @brief Reconstruct the leptonic top quark (with the highest score) from
 * DiLepton topology. Tops already reconstructed are masked out. Only edges are
 * considered in this case.
 */
void RecoTopForDiLepton(
    const HyPERGraph& hyperGraph, const scores& edge_scores,
    // const scores&
    //    hyperedge_scores,  // This is not needed in this case.
    std::vector<int64_t>& edge_masks,  // This is a vector of 0s and 1s, where 0
                                       // means the edge is already used.
    std::vector<int64_t>& top_reco_indices,  // This we want to fill with the
                                             // indices of the top quark.
    float& top_reco_score,  // This we want to fill with the score of the top
                            // quark.
    std::vector<int64_t>& top_reco_ids,  // This will hold the particle IDs of
                                         // the top quark decays.
    const std::vector<std::vector<int64_t>>&
        allowed_decay_modes) {  // This is the list of decay modes we want to
                                // consider.

  using namespace asg::msgUserCode;
  // Sanity checks
  if (hyperGraph.nEdges() != static_cast<int>(edge_scores.size()))
    throw std::runtime_error(
        "Number of defined graph edges does not match with the ones from model "
        "output!");
  if (allowed_decay_modes.empty())
    throw std::runtime_error("No allowed decay modes available!");

  for (std::size_t i = 0; i < edge_scores.size(); i++) {
    if (edge_masks.at(i) == 0) {
      if (EventReco::g_hyper_msg_level == MSG::VERBOSE)
        ANA_MSG_INFO("Skipping due to masking.");
      continue;  // Skip the edge if it is masked
    }

    // Get particle IDs.
    std::vector<int64_t> top_candidate_indices = {
        hyperGraph.getEdgeIndicesVector().at(i).first,
        hyperGraph.getEdgeIndicesVector().at(i).second};
    // Get particle IDs. The conversions here are float -> HyPERParticleID
    // (int) -> int64_t
    // The ID should be the element before the last in the node features. We
    // have to MULTIPLY by 2 because of the feature scaling. See
    // RunHyPERAlg::buildTtbarDiLeptonGraph
    std::size_t beforeLastPosition = hyperGraph.getNodeFeats(0).size() - 2;
    EventReco::HyPERParticleID id_1 = static_cast<EventReco::HyPERParticleID>(
        2 * hyperGraph.getNodeFeats(top_candidate_indices.at(0))
                .at(beforeLastPosition));
    EventReco::HyPERParticleID id_2 = static_cast<EventReco::HyPERParticleID>(
        2 * hyperGraph.getNodeFeats(top_candidate_indices.at(1))
                .at(beforeLastPosition));
    std::vector<int64_t> top_candidate_ids = {static_cast<int64_t>(id_1),
                                              static_cast<int64_t>(id_2)};

    // Only allow allowed decay modes
    if (!isAllowedDecay(top_candidate_ids, allowed_decay_modes)) {
      if (EventReco::g_hyper_msg_level == MSG::VERBOSE)
        ANA_MSG_INFO("Skipping due to not allowed decay.");
      continue;
    }

    if (edge_scores.at(i).at(0) > top_reco_score) {
      top_reco_score = edge_scores.at(i).at(0);
      top_reco_indices = std::move(top_candidate_indices);
      top_reco_ids = std::move(top_candidate_ids);
    }
  }
}

/**
 * @brief Reconstruct the ttbar single lepton topology.
 */
void RecoTtbarDiLepton(const HyPERGraph& hyperGraph, const scores& edge_scores,
                       const scores& hyperedge_scores,
                       const scores& classification_score,
                       indices& reco_indices, std::vector<float>& reco_scores,
                       std::vector<std::string>& reco_labels,
                       std::vector<std::vector<int64_t>>& reco_ids,
                       float& reco_classification_score) {
  using namespace asg::msgUserCode;

  std::vector<int64_t> top1_reco_indices(2, -1);
  std::vector<int64_t> top2_reco_indices(2, -1);

  float top1_reco_score = 0;
  float top2_reco_score = 0;

  std::vector<int64_t> top1_reco_ids(2, -1);
  std::vector<int64_t> top2_reco_ids(2, -1);

  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    setMsgLevel(MSG::INFO);
    ANA_MSG_INFO("Reconstructing TtbarDiLepton top quarks...");
    ANA_MSG_INFO("Working with these HE scores:");
    for (const auto& he_score : hyperedge_scores) {
      ANA_MSG_INFO("Element " << he_score.at(0));
    }
    ANA_MSG_INFO("Working with these edge scores:");
    for (const auto& e_score : edge_scores) {
      ANA_MSG_INFO("Element " << e_score.at(0));
    }
    ANA_MSG_INFO("Working with this classification score:");
    ANA_MSG_INFO(classification_score.at(0).at(0));
  }

  // Save the classification score
  reco_classification_score = classification_score.at(0).at(0);

  // Allowed decays
  std::vector<std::vector<int64_t>> allowed_decays = {
      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::e},
      {EventReco::HyPERParticleID::jet, EventReco::HyPERParticleID::mu}};

  // Reconstructing top1
  std::vector<int64_t> top1_masks(edge_scores.size(), 1);
  RecoTopForDiLepton(hyperGraph, edge_scores, top1_masks, top1_reco_indices,
                     top1_reco_score, top1_reco_ids, allowed_decays);
  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Top1 reco score: " << top1_reco_score);
    ANA_MSG_INFO("Top1 reco indices before scaling: "
                 << top1_reco_indices.at(0) << " " << top1_reco_indices.at(1));
    ANA_MSG_INFO("Top1 reco IDs: " << top1_reco_ids.at(0) << " "
                                   << top1_reco_ids.at(1));
  }

  // If top1 reco failed, return default values for both tops
  if (top1_reco_indices == std::vector<int64_t>(2, -1)) {
    ANA_MSG_INFO("Top1 reco failed.");
    reco_labels.push_back("HyPER_Reco_Top1");
    reco_indices.push_back(std::move(top1_reco_indices));
    reco_scores.push_back(top1_reco_score);
    reco_ids.push_back(std::move(top1_reco_ids));
    reco_labels.push_back("HyPER_Reco_Top2");
    reco_indices.push_back(std::move(top2_reco_indices));
    reco_scores.push_back(top2_reco_score);
    reco_ids.push_back(std::move(top2_reco_ids));
    reco_labels.push_back("HyPER_Reco_HE");
    reco_scores.push_back(-1.0);
    return;
  }

  // Now, reconstruct top2
  // Calculate top2 masks
  std::vector<int64_t> top2_masks(edge_scores.size(), 1);
  for (std::size_t i = 0; i < static_cast<std::size_t>(edge_scores.size());
       i++) {
    if (anyCommonElement(top1_reco_indices,
                         {hyperGraph.getEdgeIndicesVector().at(i).first,
                          hyperGraph.getEdgeIndicesVector().at(i).second}))
      top2_masks.at(i) = 0;
  }
  // Now, reconstruct top2
  RecoTopForDiLepton(hyperGraph, edge_scores, top2_masks, top2_reco_indices,
                     top2_reco_score, top2_reco_ids, allowed_decays);
  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Top2 reco score: " << top2_reco_score);
    ANA_MSG_INFO("Top2 reco indices before scaling: "
                 << top2_reco_indices.at(0) << " " << top2_reco_indices.at(1));
    ANA_MSG_INFO("Top2 reco IDs: " << top2_reco_ids.at(0) << " "
                                   << top2_reco_ids.at(1));
  }

  // Check if reco top2 is successful
  bool both_tops_reco = true;  // At this point, top1 is already reconstructed
  if (top2_reco_indices == std::vector<int64_t>(2, -1))
    both_tops_reco = false;

  // Let's find the hyperedge score corresponding to the two tops if both tops
  // are reconstructed.
  float HE_reco_score = -1.0;
  if (both_tops_reco) {
    for (std::size_t i = 0;
         i < static_cast<std::size_t>(hyperGraph.nHyperEdges()); i++) {
      std::vector<int64_t> he_indices = hyperGraph.getHyperEdgeIndices(i);
      // Check all top indices are in the HE indices.
      bool top1_in_he =
          std::find(he_indices.begin(), he_indices.end(),
                    top1_reco_indices.at(0)) != he_indices.end() &&
          std::find(he_indices.begin(), he_indices.end(),
                    top1_reco_indices.at(1)) != he_indices.end();
      bool top2_in_he =
          std::find(he_indices.begin(), he_indices.end(),
                    top2_reco_indices.at(0)) != he_indices.end() &&
          std::find(he_indices.begin(), he_indices.end(),
                    top2_reco_indices.at(1)) != he_indices.end();
      if (top1_in_he && top2_in_he) {
        HE_reco_score = hyperedge_scores.at(i).at(0);
        break;
      }
    }
  }

  // Finally, we need to shift the indices in the tops to reflect
  // the position in the particle-specific containers.
  if (both_tops_reco) {
    reescaleIndicesTtbarDiLepton(hyperGraph, top1_reco_indices, top1_reco_ids);
    reescaleIndicesTtbarDiLepton(hyperGraph, top2_reco_indices, top2_reco_ids);
  } else {  // Only shift top1
    reescaleIndicesTtbarDiLepton(hyperGraph, top1_reco_indices, top1_reco_ids);
  }

  if (EventReco::g_hyper_msg_level == MSG::VERBOSE) {
    ANA_MSG_INFO("Top1 reco indices after scaling: "
                 << top1_reco_indices.at(0) << " " << top1_reco_indices.at(1));
    ANA_MSG_INFO("Top2 reco indices after scaling: "
                 << top2_reco_indices.at(0) << " " << top2_reco_indices.at(1));
  }

  // Save results
  reco_labels.push_back("HyPER_Reco_Top1");
  reco_indices.push_back(std::move(top1_reco_indices));
  reco_scores.push_back(top1_reco_score);
  reco_ids.push_back(std::move(top1_reco_ids));
  reco_labels.push_back("HyPER_Reco_Top2");
  reco_indices.push_back(std::move(top2_reco_indices));
  reco_scores.push_back(top2_reco_score);
  reco_ids.push_back(std::move(top2_reco_ids));
  reco_labels.push_back("HyPER_Reco_HE");
  reco_scores.push_back(HE_reco_score);
}

}  // namespace EventReco
