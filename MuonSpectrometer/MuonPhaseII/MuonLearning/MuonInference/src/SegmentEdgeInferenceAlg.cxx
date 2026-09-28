/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentEdgeInferenceAlg.h"
#include "InferenceUtils.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODMuonViews/ContainerDecorator.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "Acts/Utilities/Helpers.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <unordered_set>
#include <utility>

namespace MuonML {

namespace {

std::uint64_t 
undirectedPairKey(std::size_t first, std::size_t second) {
  if (first > second) std::swap(first, second);
  const auto first32 = static_cast<std::uint32_t>(first);
  const auto second32 = static_cast<std::uint32_t>(second);

  return (static_cast<std::uint64_t>(first32) << 32) | static_cast<std::uint64_t>(second32);
}

std::pair<std::size_t, std::size_t>
unpackPairKey(std::uint64_t key) {
  const auto first = static_cast<std::uint32_t>(key >> 32);
  const auto second = static_cast<std::uint32_t>(key);
  return {first, second};
}

class DisjointSet {
public:
  explicit DisjointSet(std::size_t size) : m_parent(size), m_rank(size, 0) {
    std::iota(m_parent.begin(), m_parent.end(), 0);
  }

  std::size_t find(std::size_t node) {
    if (m_parent[node] != node) m_parent[node] = find(m_parent[node]);
    return m_parent[node];
  }

  void unite(std::size_t first, std::size_t second) {
    first = find(first);
    second = find(second);
    if (first == second) return;
    if (m_rank[first] < m_rank[second]) std::swap(first, second);
    m_parent[second] = first;
    if (m_rank[first] == m_rank[second]) ++m_rank[first];
  }

private:
  std::vector<std::size_t> m_parent;
  std::vector<unsigned char> m_rank;
};

// Explicit radial ranking (0 = closest to the IP), independent of the
// LayerIndex enum's underlying integer values. BarrelExtended is the
// barrel's BI-equivalent chamber, so it ranks alongside Inner.
int layerRadialRank(Muon::MuonStationIndex::LayerIndex layer) {
  using enum Muon::MuonStationIndex::LayerIndex;
  switch (layer) {
    case Inner:
    case BarrelExtended:
      return 0;
    case Middle:
      return 1;
    case Outer:
      return 2;
    case Extended:
      return 3;
    default:
      return 4;
  }
}

// Truth diagnostics only. Why a node that passed the threshold was not kept.
constexpr unsigned char kReasonNotSelected = 0;        // no selected pair (top-K / mutual gate)
constexpr unsigned char kReasonChamberDedup = 1;       // KeepBestSegmentPerChamber
constexpr unsigned char kReasonComponentRejected = 2;  // MinSegmentsPerComponent / no anchor

// Loss categories; the order matches the table printed by finalize().
enum LossCategory : std::uint8_t{
  kNoPartnerSingleton = 0,
  kNoPartnerSameChamber,
  kBucketSelf,
  kBucketPartners,
  kSectorWindow,
  kAngleWindow,
  kEdgeCaps,
  kBelowThreshold,
  kDedupChamber,
  kComponentRejected,
  kNotSelected,
  kNumLossCategories
};

enum class TruthRegion : std::uint8_t { kBarrel = 0, kTransition = 1, kEndcap = 2 };

TruthRegion regionOf(double absEta) {
  return absEta < 1.0 ? TruthRegion::kBarrel
                      : (absEta < 1.3 ? TruthRegion::kTransition : TruthRegion::kEndcap);
}

}  // namespace

StatusCode SegmentEdgeInferenceAlg::initialize() {
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_pairGateDecorKey.initialize());
  ATH_CHECK(m_filteredSegmentKey.initialize(!m_filteredSegmentKey.empty()));
  ATH_CHECK(m_edgeClassifier.retrieve());
  m_truthDiagEnabled = m_edgeClassifier->enableTruthDiagnostics();
  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeInferenceAlg::execute(const EventContext& ctx) const {
  const xAOD::MuonSegmentContainer* segments{};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
  // Evaluated once per event; short-circuits without touching the message
  // service unless the property was explicitly enabled.
  const bool truthDiag = m_truthDiagEnabled && msgLvl(MSG::DEBUG);
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": input segments in '" << m_segmentKey.key()
                << "' = " << segments->size());

  if (truthDiag) {
    std::size_t truthSegs = 0, bkgSegs = 0;
    for (const xAOD::MuonSegment* seg : *segments) {
      MuonR4::getTruthMatchedParticle(*seg) ? ++truthSegs : ++bkgSegs;
    }
    m_sumInputTruthSegments += truthSegs;
    m_sumInputBkgSegments += bkgSegs;
  }

  SegmentEdgeGraph graph{};
  std::vector<SegmentEdgeScore> scores{};
  ATH_CHECK(m_edgeClassifier->buildGraph(ctx, *segments, graph));
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": built graph with nodes=" << graph.nNodes
                << ", edges=" << graph.nEdges);

  // Per-node truth-particle grouping key (-1 = unlabeled), used for the 
  // segment-level funnel and for the edge-level true/background split. 
  std::vector<int32_t> nodeTruthId;
  if (truthDiag) {
    nodeTruthId.assign(graph.nNodes, -1);
    std::size_t truthSegs = 0, bkgSegs = 0;
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      const xAOD::TruthParticle* truthPart =
          MuonR4::getTruthMatchedParticle(*graph.segments[node]);
      if (truthPart) {
        nodeTruthId[node] = static_cast<int32_t>(truthPart->index());
        ++truthSegs;
      } else {
        ++bkgSegs;
      }
    }
    m_sumOnnxTruthSegments += truthSegs;
    m_sumOnnxBkgSegments += bkgSegs;
  }

  ATH_CHECK(m_edgeClassifier->classifyEdges(ctx, graph, scores));
  if (!scores.empty()) {
    float minProb = std::numeric_limits<float>::max();
    float maxProb = std::numeric_limits<float>::lowest();
    for (const SegmentEdgeScore& score : scores) {
      minProb = std::min(minProb, score.probability);
      maxProb = std::max(maxProb, score.probability);
    }
    ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                  << ": edge scores=" << scores.size()
                  << ", prob range=[" << minProb << ", " << maxProb << "]");
  } else {
    ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                  << ": no edge scores produced");
  }

  // Symmetrise directed model outputs once.  Everything downstream uses the
  // score of an undirected segment association.
  std::unordered_map<std::uint64_t, float> pairProbability;
  pairProbability.reserve(scores.size());
  for (const SegmentEdgeScore& score : scores) {
    if (score.src >= graph.nNodes || score.dst >= graph.nNodes ||
        score.src == score.dst) {
      continue;
    }
    const std::uint64_t key = undirectedPairKey(score.src, score.dst);
    auto [it, inserted] = pairProbability.emplace(key, score.probability);
    if (!inserted) it->second = std::max(it->second, score.probability);
  }

  // Edge-level recall/false-positive check: candidate pairs that reached ONNX 
  // true pairs reach PairGateThreshold, versus how many background pairs do. 
  if (truthDiag) {
    std::size_t trueTotal = 0, truePassed = 0, bkgTotal = 0, bkgPassed = 0;
    for (const auto& [key, probability] : pairProbability) {
      const auto [first, second] = unpackPairKey(key);
      if (first >= graph.nNodes || second >= graph.nNodes) continue;
      const bool isTrueEdge =
          nodeTruthId[first] >= 0 && nodeTruthId[first] == nodeTruthId[second];
      const bool passed = probability >= m_pairGateThreshold.value();
      if (isTrueEdge) {
        ++trueTotal;
        truePassed += passed;
      } else {
        ++bkgTotal;
        bkgPassed += passed;
      }
    }
    m_sumTrueEdgesTotal += trueTotal;
    m_sumTrueEdgesPassed += truePassed;
    m_sumBkgEdgesTotal += bkgTotal;
    m_sumBkgEdgesPassed += bkgPassed;
  }

  // Form a sparse, score-ranked topology. Mutual top-K associations retain
  // locally consistent paths while suppressing one-sided bridges.
  using WeightedEdge = std::pair<std::uint64_t, float>;
  const auto betterWeightedEdge = [](const WeightedEdge& first,
                                      const WeightedEdge& second) {
    const int probabilityOrder = InferenceUtils::compareFloatDescending(
        first.second, second.second);
    if (probabilityOrder != 0) {
      return probabilityOrder < 0;
    }
    return first.first < second.first;
  };
  std::vector<std::vector<WeightedEdge>> edgesByNode(graph.nNodes);
  std::size_t thresholdPairs = 0;
  for (const auto& [key, probability] : pairProbability) {
    if (probability < m_pairGateThreshold.value()) continue;
    const auto [first, second] = unpackPairKey(key);
    if (first >= graph.nNodes || second >= graph.nNodes) continue;
    edgesByNode[first].emplace_back(key, probability);
    edgesByNode[second].emplace_back(key, probability);
    ++thresholdPairs;
  }

  std::size_t thresholdedNodes = 0;
  for (const std::vector<WeightedEdge>& nodeEdges : edgesByNode) {
    thresholdedNodes += !nodeEdges.empty();
  }

  std::unordered_map<std::uint64_t, unsigned char> nominations;
  nominations.reserve(thresholdPairs);
  for (std::vector<WeightedEdge>& nodeEdges : edgesByNode) {
    std::sort(nodeEdges.begin(), nodeEdges.end(), betterWeightedEdge);
    if (m_maxEdgesPerNode.value() != 0 &&
        nodeEdges.size() > m_maxEdgesPerNode.value()) {
      nodeEdges.resize(m_maxEdgesPerNode.value());
    }
    for (const WeightedEdge& edge : nodeEdges) {
      ++nominations[edge.first];
    }
  }

  std::size_t mutualTopKPairs = 0;
  std::size_t oneSidedTopKPairs = 0;
  for (const auto& [_, count] : nominations) {
    if (count == 2) {
      ++mutualTopKPairs;
    } else {
      ++oneSidedTopKPairs;
    }
  }

  std::unordered_set<std::uint64_t> selectedPairKeys;
  selectedPairKeys.reserve(thresholdPairs);
  if (m_useDegreeCappedComponents.value()) {
    // Apply a score-ordered global degree cap when explicitly requested.
    std::vector<WeightedEdge> acceptedPairs;
    acceptedPairs.reserve(thresholdPairs);
    for (const auto& [key, probability] : pairProbability) {
      if (probability < m_pairGateThreshold.value()) continue;
      acceptedPairs.emplace_back(key, probability);
    }
    std::sort(acceptedPairs.begin(), acceptedPairs.end(), betterWeightedEdge);

    const unsigned int maxDegree = m_maxEdgesPerNode.value();
    std::vector<unsigned int> degree(graph.nNodes, 0);
    for (const WeightedEdge& edge : acceptedPairs) {
      const auto [first, second] = unpackPairKey(edge.first);
      if (maxDegree != 0 &&
          (degree[first] >= maxDegree || degree[second] >= maxDegree)) {
        continue;
      }
      selectedPairKeys.insert(edge.first);
      ++degree[first];
      ++degree[second];
    }
  } else {
    for (const auto& [key, count] : nominations) {
      if (m_requireMutualTopKEdges.value() && count != 2) continue;
      selectedPairKeys.insert(key);
    }
  }

  // A mutual top-K selection can leave an endpoint without an association.
  // Add its best thresholded edge, at most once per orphaned endpoint.
  std::size_t orphanRecoveryPairs = 0;
  if (!m_useDegreeCappedComponents.value() &&
      m_requireMutualTopKEdges.value() &&
      m_recoverOrphanNodes.value()) {
    std::vector<unsigned char> selectedNode(graph.nNodes, 0);
    for (const std::uint64_t key : selectedPairKeys) {
      const auto [first, second] = unpackPairKey(key);
      if (first < graph.nNodes) selectedNode[first] = 1;
      if (second < graph.nNodes) selectedNode[second] = 1;
    }
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      if (selectedNode[node] || edgesByNode[node].empty()) continue;
      const std::uint64_t key = edgesByNode[node].front().first;
      const auto [first, second] = unpackPairKey(key);
      if (first >= graph.nNodes || second >= graph.nNodes) continue;
      if (selectedPairKeys.insert(key).second) ++orphanRecoveryPairs;
      selectedNode[first] = 1;
      selectedNode[second] = 1;
    }
  }

  std::vector<std::uint64_t> selectedPairs{selectedPairKeys.begin(),
                                           selectedPairKeys.end()};
  std::sort(selectedPairs.begin(), selectedPairs.end());

  DisjointSet components{graph.nNodes};
  std::vector<bool> activeNode(graph.nNodes, false);
  for (const std::uint64_t key : selectedPairs) {
    const auto [first, second] = unpackPairKey(key);
    components.unite(first, second);
    activeNode[first] = true;
    activeNode[second] = true;
  }

  std::unordered_map<std::size_t, std::vector<std::size_t>> byRoot;
  byRoot.reserve(graph.nNodes);
  for (std::size_t node = 0; node < graph.nNodes; ++node) {
    if (activeNode[node]) byRoot[components.find(node)].push_back(node);
  }

  // Deterministic component IDs make debugging and validation reproducible.
  std::vector<std::vector<std::size_t>> componentNodes;
  componentNodes.reserve(byRoot.size());
  for (auto& [_, nodes] : byRoot) {
    std::sort(nodes.begin(), nodes.end());
    componentNodes.push_back(std::move(nodes));
  }
  std::ranges::sort(componentNodes,
            [](const auto& first, const auto& second) {
              return first.front() < second.front();
            });

  xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, std::vector<unsigned>>
      decor{m_pairGateDecorKey, ctx};

  std::vector<std::uint8_t> keptNode(graph.nNodes, false);
  std::vector<std::uint8_t> nodeDropReason;
  if (truthDiag) {
     nodeDropReason.assign(graph.nNodes, kReasonNotSelected);
  }

  std::size_t topologyNodes = 0;
  std::size_t retainedNodes = 0;
  std::size_t chamberSuppressedNodes = 0;
  std::size_t rejectedComponents = 0;
  std::size_t componentsKept = 0;
  std::size_t anchors = 0;
  std::size_t nodesRejectedByMinComponent = 0;
  unsigned nextComponentId = 1;
  const InferenceUtils::SegmentQualityOrder betterSegment{};

  const auto isBetterNode = [&](std::size_t candidate, std::size_t incumbent) {
    if (betterSegment(graph.segments[candidate], graph.segments[incumbent])) {
      return true;
    }
    if (betterSegment(graph.segments[incumbent], graph.segments[candidate])) {
      return false;
    }
    return candidate < incumbent;
  };

  for (const std::vector<std::size_t>& rawNodes : componentNodes) {
    topologyNodes += rawNodes.size();
    std::vector<std::size_t> retained = rawNodes;

    // The seeder resolves same-chamber alternatives while building a seed.
    // Retaining only the highest-ranked representative is therefore optional.
    if (m_keepBestSegmentPerChamber.value()) {
      std::unordered_map<int, std::size_t> bestByChamber;
      bestByChamber.reserve(rawNodes.size());
      for (const std::size_t node : rawNodes) {
        const int chamber =
            static_cast<int>(graph.segments[node]->chamberIndex());
        const auto found = bestByChamber.find(chamber);
        if (found == bestByChamber.end() || isBetterNode(node, found->second)) {
          bestByChamber[chamber] = node;
        }
      }
      retained.clear();
      retained.reserve(bestByChamber.size());
      for (const auto& [_, node] : bestByChamber) retained.push_back(node);
      std::sort(retained.begin(), retained.end());
      chamberSuppressedNodes += rawNodes.size() - retained.size();
    }

    // Provisional reasons; nodes that end up kept are identified by keptNode.
    if (truthDiag) {
      for (const std::size_t node : rawNodes) {
          nodeDropReason[node] = kReasonChamberDedup;
      }
      for (const std::size_t node : retained) {
          nodeDropReason[node] = kReasonComponentRejected;
      }
    }

    if (retained.size() < m_minSegmentsPerComponent.value()) {
      nodesRejectedByMinComponent += retained.size();
      ++rejectedComponents;
      continue;
    }

    // Only ranked component members launch seeds. The edge score therefore
    // reduces seed attempts directly rather than serving only as a label.
    std::vector<std::size_t> rankedNodes{retained};
    if (m_anchorInnermostLayer.value()) {
      // Anchor only on the inner segment(s)
      int bestRank = std::numeric_limits<int>::max();
      for (const std::size_t node : rankedNodes) {
        bestRank = std::min(
            bestRank,
            layerRadialRank(Muon::MuonStationIndex::toLayerIndex(
                graph.segments[node]->chamberIndex())));
      }
      std::erase_if(rankedNodes, [&](std::size_t node) {
        return layerRadialRank(Muon::MuonStationIndex::toLayerIndex(
                   graph.segments[node]->chamberIndex())) != bestRank;
      });
    } else {
      std::ranges::sort(rankedNodes, isBetterNode);
      const std::size_t nAnchors = m_seedAnchorsPerComponent.value() == 0
          ? rankedNodes.size()
          : std::min<std::size_t>(m_seedAnchorsPerComponent.value(),
                                  rankedNodes.size());
      rankedNodes.resize(nAnchors);
    }
    if (rankedNodes.empty()) {
      ++rejectedComponents;
      continue;
    }
    std::ranges::sort(rankedNodes);

    const unsigned componentId = nextComponentId++;
    for (const std::size_t node : retained) {
      const bool isAnchor = Acts::rangeContainsValue(rankedNodes, node);
      decor(*graph.segments[node]) = {
          componentId, static_cast<unsigned int>(isAnchor)};
      keptNode[node] = true;
    }
    retainedNodes += retained.size();
    anchors += rankedNodes.size();
    ++componentsKept;
  }

  if (truthDiag) {
    std::size_t truthSegs = 0, bkgSegs = 0;
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      if (!keptNode[node]) {
         continue;
      }
      nodeTruthId[node] >= 0 ? ++truthSegs : ++bkgSegs;
    }
    m_sumRetainedTruthSegments += truthSegs;
    m_sumRetainedBkgSegments += bkgSegs;

    std::vector<unsigned char> thresholded(graph.nNodes, 0);
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      thresholded[node] = !edgesByNode[node].empty();
    }
    classifyLostTruthSegments(*segments, graph, pairProbability, thresholded,
                              keptNode, nodeDropReason);
  }

  if (!m_filteredSegmentKey.empty()) {
    auto connectedSegments =
        std::make_unique<ConstDataVector<xAOD::MuonSegmentContainer>>(
            SG::VIEW_ELEMENTS);
    connectedSegments->reserve(graph.nNodes);
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      if (!keptNode[node] || !graph.segments[node]) continue;
      connectedSegments->push_back(graph.segments[node]);
    }

    const std::size_t nConnectedSegments = connectedSegments->size();
    SG::WriteHandle<ConstDataVector<xAOD::MuonSegmentContainer>> connectedHandle{
        m_filteredSegmentKey, ctx};
    ATH_CHECK(connectedHandle.record(std::move(connectedSegments)));
    ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                  << ": wrote " << nConnectedSegments
                  << " ML-connected segment(s) to '"
                  << m_filteredSegmentKey.key() << "'");
  }

  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": ML components graphNodes=" << graph.nNodes
                << ", thresholdedNodes=" << thresholdedNodes
                << ", thresholdPairs=" << thresholdPairs
                << ", mutualTopKPairs=" << mutualTopKPairs
                << ", oneSidedTopKPairs=" << oneSidedTopKPairs
                << ", selectedPairs=" << selectedPairKeys.size()
                << ", components=" << componentsKept
                << ", topologyNodes=" << topologyNodes
                << ", retainedNodes=" << retainedNodes
                << ", chamberSuppressedNodes=" << chamberSuppressedNodes
                << ", nodesRejectedByMinComponent=" << nodesRejectedByMinComponent
                << ", keepBestSegmentPerChamber=" << m_keepBestSegmentPerChamber.value()
                << ", seedAnchors=" << anchors
                << ", rejectedComponents=" << rejectedComponents
                << ", threshold=" << m_pairGateThreshold.value()
                << ", maxEdgesPerNode=" << m_maxEdgesPerNode.value()
                << ", orphanRecoveryPairs=" << orphanRecoveryPairs
                << ", mutualTopK=" << m_requireMutualTopKEdges.value()
                << ", degreeCapped=" << m_useDegreeCappedComponents.value());

  // Job-summed diagnostics
  if (msgLvl(MSG::DEBUG)) {
    m_sumGraphNodes += graph.nNodes;
    m_sumThresholdedNodes += thresholdedNodes;
    m_sumMutualTopKPairs += mutualTopKPairs;
    m_sumOneSidedTopKPairs += oneSidedTopKPairs;
    m_sumOrphanRecoveryPairs += orphanRecoveryPairs;
    m_sumTopologyNodes += topologyNodes;
    m_sumRetainedNodes += retainedNodes;
    m_sumChamberSuppressedNodes += chamberSuppressedNodes;
    m_sumNodesRejectedByMinComponent += nodesRejectedByMinComponent;
    m_sumRejectedComponents += rejectedComponents;
    m_sumComponentsKept += componentsKept;
  }
  return StatusCode::SUCCESS;
}

void SegmentEdgeInferenceAlg::classifyLostTruthSegments(
    const xAOD::MuonSegmentContainer& segments, const SegmentEdgeGraph& graph,
    const std::unordered_map<std::uint64_t, float>& pairProbability,
    const std::vector<unsigned char>& thresholded,
    const std::vector<std::uint8_t>& keptNode,
    const std::vector<unsigned char>& nodeDropReason) const {
  static_assert(kNumLossCategories == TruthLossCounters::kCategories);
  constexpr std::size_t kColumns = TruthLossCounters::kColumns;
  const std::size_t nInput = segments.size();

  if (graph.inputNodeIndex.size() != nInput) {
    if (!m_warnedMissingToolDiagnostics.exchange(true)) {
      ATH_MSG_WARNING("Truth diagnostics requested, but the edge classifier tool "
                      "did not provide its pre-ONNX fates; set "
                      "EnableTruthDiagnostics on the tool as well. Skipping the "
                      "lost-segment classification.");
    }
    return;
  }

  // Truth particle of every input segment and the segments grouped per particle.
  std::vector<const xAOD::TruthParticle*> truthPart(nInput, nullptr);
  std::vector<std::int32_t> truthId(nInput, -1);
  std::unordered_map<std::int32_t, std::vector<std::uint32_t>> byTruth;
  {
    std::uint32_t inputIndex = 0;
    for (const xAOD::MuonSegment* segment : segments) {
      if (const xAOD::TruthParticle* part = MuonR4::getTruthMatchedParticle(*segment)) {
        truthPart[inputIndex] = part;
        truthId[inputIndex] = static_cast<std::int32_t>(part->index());
        byTruth[truthId[inputIndex]].push_back(inputIndex);
      }
      ++inputIndex;
    }
  }
  const auto isRetained = [&](std::size_t input) {
    const std::int32_t node = graph.inputNodeIndex[input];
    return node >= 0 && keptNode[static_cast<std::size_t>(node)];
  };

  // Per segment: how far its best cross-chamber true pair got (1 = bucket cap,
  // 2 = sector window, 3 = angle window, 4 = edge caps, 5 = scored, 0 = only
  // same-chamber partners) and the score / index of its best scored pair.
  const auto rankOf = [](PairGate gate) -> unsigned char {
    switch (gate) {
      case PairGate::BucketCap: return 1;
      case PairGate::SectorWindow: return 2;
      case PairGate::AngleWindow: return 3;
      case PairGate::EdgeCaps: return 4;
      case PairGate::Scored: return 5;
      case PairGate::SameChamber: break;
    }
    return 0;
  };
  std::vector<unsigned char> bestRank(nInput, 0);
  std::vector<float> bestScore(nInput, -1.f);
  std::vector<std::int32_t> bestScoredPair(nInput, -1);
  for (std::size_t k = 0; k < graph.truthPairFates.size(); ++k) {
    const PairFate& fate = graph.truthPairFates[k];
    const unsigned char rank = rankOf(fate.gate);
    if (rank == 0) continue;
    const std::array<std::uint32_t, 2> ends{fate.first, fate.second};
    for (const std::uint32_t end : ends) bestRank[end] = std::max(bestRank[end], rank);
    if (fate.gate != PairGate::Scored) continue;
    const std::int32_t firstNode = graph.inputNodeIndex[fate.first];
    const std::int32_t secondNode = graph.inputNodeIndex[fate.second];
    if (firstNode < 0 || secondNode < 0) continue;
    const auto found = pairProbability.find(undirectedPairKey(
        static_cast<std::size_t>(firstNode), static_cast<std::size_t>(secondNode)));
    const float score = found != pairProbability.end() ? found->second : 0.f;
    for (const std::uint32_t end : ends) {
      if (score > bestScore[end]) {
        bestScore[end] = score;
        bestScoredPair[end] = static_cast<std::int32_t>(k);
      }
    }
  }

  // Seedability per truth muon: at least 2 distinct chambers in the input,
  // fewer than 2 among the retained segments.
  std::unordered_map<std::int32_t, bool> seedLostByTruth;
  std::array<std::size_t, 4> muonsSeedable{};
  std::array<std::size_t, 4> muonsSeedLost{};
  for (const auto& entry : byTruth) {
    std::unordered_set<int> inputChambers;
    std::unordered_set<int> retainedChambers;
    for (const std::uint32_t input : entry.second) {
      const int chamber = static_cast<int>(segments[input]->chamberIndex());
      inputChambers.insert(chamber);
      if (isRetained(input)) retainedChambers.insert(chamber);
    }
    const bool seedable = inputChambers.size() >= 2;
    const bool lost = seedable && retainedChambers.size() < 2;
    seedLostByTruth[entry.first] = lost;
    if (!seedable) continue;
    const std::size_t region = 1 + static_cast<std::size_t>(
        regionOf(std::abs(truthPart[entry.second.front()]->eta())));
    ++muonsSeedable[0];
    ++muonsSeedable[region];
    if (lost) {
      ++muonsSeedLost[0];
      ++muonsSeedLost[region];
    }
  }

  std::array<std::size_t, kNumLossCategories * kColumns> category{};
  std::array<std::size_t, 5> scoreBin{};
  std::array<std::size_t, 2> sectorDelta{};
  std::array<std::size_t, 4> layerPair{};
  std::array<std::size_t, 4> muonSegments{};
  std::array<std::size_t, 3> precisionHits{};
  for (std::size_t i = 0; i < nInput; ++i) {
    if (truthId[i] < 0 || isRetained(i)) continue;
    const std::vector<std::uint32_t>& members = byTruth[truthId[i]];
    const std::int32_t node = graph.inputNodeIndex[i];

    std::size_t cat{};
    if (members.size() == 1) {
      cat = kNoPartnerSingleton;
    } else if (bestRank[i] == 0) {
      cat = kNoPartnerSameChamber;
    } else if (node == kDroppedByBucketCap) {
      cat = kBucketSelf;
    } else if (node >= 0 && thresholded[static_cast<std::size_t>(node)]) {
      switch (nodeDropReason[static_cast<std::size_t>(node)]) {
        case kReasonChamberDedup: cat = kDedupChamber; break;
        case kReasonComponentRejected: cat = kComponentRejected; break;
        default: cat = kNotSelected; break;
      }
    } else {
      switch (bestRank[i]) {
        case 5: cat = kBelowThreshold; break;
        case 4: cat = kEdgeCaps; break;
        case 3: cat = kAngleWindow; break;
        case 2: cat = kSectorWindow; break;
        default: cat = kBucketPartners; break;
      }
    }

    const std::size_t region = 1 + static_cast<std::size_t>(
        regionOf(std::abs(truthPart[i]->eta())));
    ++category[cat * kColumns];
    ++category[cat * kColumns + region];
    if (seedLostByTruth[truthId[i]]) ++category[cat * kColumns + 4];

    if (cat != kBelowThreshold) continue;
    if (bestScoredPair[i] >= 0) {
      const PairFate& fate = graph.truthPairFates[static_cast<std::size_t>(bestScoredPair[i])];
      const float score = bestScore[i];
      ++scoreBin[score < 1e-3f ? 0 : score < 1e-2f ? 1 : score < 2e-2f ? 2
                                                         : score < 5e-2f ? 3 : 4];
      ++sectorDelta[fate.sectorDelta == 0 ? 0 : 1];
      const std::uint32_t other = fate.first == i ? fate.second : fate.first;
      int rankA = layerRadialRank(Muon::MuonStationIndex::toLayerIndex(
          segments[i]->chamberIndex()));
      int rankB = layerRadialRank(Muon::MuonStationIndex::toLayerIndex(
          segments[other]->chamberIndex()));
      if (rankA > rankB) std::swap(rankA, rankB);
      ++layerPair[(rankA == 0 && rankB == 1) ? 0
                  : (rankA == 1 && rankB == 2) ? 1
                  : (rankA == 0 && rankB == 2) ? 2 : 3];
    }
    ++muonSegments[std::min<std::size_t>(members.size(), 5) - 2];
    const unsigned int hits = segments[i]->nPrecisionHits();
    ++precisionHits[hits <= 4 ? 0 : (hits <= 6 ? 1 : 2)];
  }

  for (std::size_t c = 0; c < category.size(); ++c) m_truthLoss.category[c] += category[c];
  for (std::size_t c = 0; c < scoreBin.size(); ++c) m_truthLoss.scoreBin[c] += scoreBin[c];
  for (std::size_t c = 0; c < sectorDelta.size(); ++c) m_truthLoss.sectorDelta[c] += sectorDelta[c];
  for (std::size_t c = 0; c < layerPair.size(); ++c) m_truthLoss.layerPair[c] += layerPair[c];
  for (std::size_t c = 0; c < muonSegments.size(); ++c) m_truthLoss.muonSegments[c] += muonSegments[c];
  for (std::size_t c = 0; c < precisionHits.size(); ++c) m_truthLoss.precisionHits[c] += precisionHits[c];
  for (std::size_t c = 0; c < muonsSeedable.size(); ++c) {
    m_truthLoss.muonsSeedable[c] += muonsSeedable[c];
    m_truthLoss.muonsSeedLost[c] += muonsSeedLost[c];
  }
}

StatusCode SegmentEdgeInferenceAlg::finalize() {
  ATH_MSG_DEBUG(
      "SegmentEdgeInferenceAlg post-ONNX selection summary (job-summed): "
      << "graphNodes=" << m_sumGraphNodes
      << ", thresholdedNodes=" << m_sumThresholdedNodes
      << " (>=PairGateThreshold=" << m_pairGateThreshold.value() << ")"
      << ", mutualTopKPairs=" << m_sumMutualTopKPairs
      << ", oneSidedTopKPairs(rejected by RequireMutualTopKEdges="
      << m_requireMutualTopKEdges.value() << ")=" << m_sumOneSidedTopKPairs
      << " (MaxEdgesPerNode=" << m_maxEdgesPerNode.value()
      << "; a thresholded edge is dropped here purely by rank, "
         "independent of PairGateThreshold)"
      << ", orphanRecoveryPairs=" << m_sumOrphanRecoveryPairs
      << ", chamberSuppressedNodes=" << m_sumChamberSuppressedNodes
      << " (KeepBestSegmentPerChamber=" << m_keepBestSegmentPerChamber.value() << ")"
      << ", nodesRejectedByMinComponent=" << m_sumNodesRejectedByMinComponent
      << " (MinSegmentsPerComponent=" << m_minSegmentsPerComponent.value() << ")"
      << ", rejectedComponents=" << m_sumRejectedComponents
      << ", componentsKept=" << m_sumComponentsKept
      << ", topologyNodes=" << m_sumTopologyNodes
      << ", retainedNodes=" << m_sumRetainedNodes);

  if (m_truthDiagEnabled) {
    ATH_MSG_DEBUG(
        "SegmentEdgeInferenceAlg truth-vs-background funnel (job-summed; "
        "truth segment = getTruthMatchedParticle(seg) != nullptr, matching "
        "SegmentDumperAlg::m_segmentHasTruth, no isMuon() filter, no G4 "
        "pseudo-label fallback): "
        << "input: truth=" << m_sumInputTruthSegments
        << " bkg=" << m_sumInputBkgSegments
        << "; reachesONNX: truth=" << m_sumOnnxTruthSegments
        << " bkg=" << m_sumOnnxBkgSegments
        << "; retained: truth=" << m_sumRetainedTruthSegments
        << " bkg=" << m_sumRetainedBkgSegments
        << "; edge-level (same truth-particle index on both endpoints) at "
           "PairGateThreshold=" << m_pairGateThreshold.value()
        << ": trueEdges passed/total=" << m_sumTrueEdgesPassed << "/"
        << m_sumTrueEdgesTotal
        << ", bkgEdges passed/total=" << m_sumBkgEdgesPassed << "/"
        << m_sumBkgEdgesTotal);

    constexpr std::array<const char*, TruthLossCounters::kCategories> names{
        "A1 no possible partner: singleton muon segment",
        "A2 no possible partner: same-chamber partners only",
        "B0 never reached ONNX: segment dropped by MaxSegmentsPerBucket",
        "B1 never reached ONNX: partners dropped by MaxSegmentsPerBucket",
        "B2 never reached ONNX: outside sector window (MaxDeltaSector)",
        "B3 never reached ONNX: outside angle window (MaxDeltaThetaDeg)",
        "B4 never reached ONNX: pre-ONNX edge caps",
        "C  true pair(s) scored below PairGateThreshold",
        "D1 passed threshold, removed: KeepBestSegmentPerChamber",
        "D2 passed threshold, removed: component rejected (MinSegmentsPerComponent/anchors)",
        "D3 passed threshold, removed: not selected by top-K/mutual gate"};
    constexpr std::size_t kColumns = TruthLossCounters::kColumns;
    ATH_MSG_DEBUG("Lost truth segments by stage (job-summed; truth segment = "
                  "input segment with a truth particle, lost = not in the filtered "
                  "container). Columns: all | barrel | transition | endcap | "
                  "belonging to a muon that lost seedability");
    std::size_t lostTotal = 0;
    for (std::size_t c = 0; c < names.size(); ++c) {
      const auto value = [&](std::size_t col) { return m_truthLoss.category[c * kColumns + col].load(); };
      lostTotal += value(0);
      ATH_MSG_DEBUG("  " << names[c] << ": " << value(0) << " | " << value(1)
                    << " | " << value(2) << " | " << value(3) << " | " << value(4));
    }
    ATH_MSG_DEBUG("  check: sum of categories=" << lostTotal
                  << ", input truth - retained truth="
                  << (m_sumInputTruthSegments.load() - m_sumRetainedTruthSegments.load()));
    ATH_MSG_DEBUG("  truth muons with >=2 input chambers (all|barrel|transition|endcap): "
                  << m_truthLoss.muonsSeedable[0].load() << "|" << m_truthLoss.muonsSeedable[1].load()
                  << "|" << m_truthLoss.muonsSeedable[2].load() << "|" << m_truthLoss.muonsSeedable[3].load()
                  << "; of which <2 chambers retained: "
                  << m_truthLoss.muonsSeedLost[0].load() << "|" << m_truthLoss.muonsSeedLost[1].load()
                  << "|" << m_truthLoss.muonsSeedLost[2].load() << "|" << m_truthLoss.muonsSeedLost[3].load());
    ATH_MSG_DEBUG("  category C descriptors: best true-pair score <1e-3/1e-3..1e-2/1e-2..2e-2/2e-2..5e-2/>=5e-2: "
                  << m_truthLoss.scoreBin[0].load() << "/" << m_truthLoss.scoreBin[1].load() << "/"
                  << m_truthLoss.scoreBin[2].load() << "/" << m_truthLoss.scoreBin[3].load() << "/"
                  << m_truthLoss.scoreBin[4].load()
                  << "; pair sector delta same/adjacent: " << m_truthLoss.sectorDelta[0].load() << "/"
                  << m_truthLoss.sectorDelta[1].load()
                  << "; layer pair Inner-Middle/Middle-Outer/Inner-Outer/other: "
                  << m_truthLoss.layerPair[0].load() << "/" << m_truthLoss.layerPair[1].load() << "/"
                  << m_truthLoss.layerPair[2].load() << "/" << m_truthLoss.layerPair[3].load()
                  << "; muon input segments 2/3/4/5+: " << m_truthLoss.muonSegments[0].load() << "/"
                  << m_truthLoss.muonSegments[1].load() << "/" << m_truthLoss.muonSegments[2].load() << "/"
                  << m_truthLoss.muonSegments[3].load()
                  << "; nPrecisionHits <=4/5-6/>=7: " << m_truthLoss.precisionHits[0].load() << "/"
                  << m_truthLoss.precisionHits[1].load() << "/" << m_truthLoss.precisionHits[2].load());
  }
  return StatusCode::SUCCESS;
}

} // namespace MuonML
