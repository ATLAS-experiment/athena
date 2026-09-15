#include "SegmentEdgeInferenceAlg.h"
#include "InferenceUtils.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODMuonViews/ContainerDecorator.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "Acts/Utilities/Helpers.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace MuonML {

namespace {

std::uint64_t undirectedPairKey(std::size_t first, std::size_t second) {
  if (first > second) std::swap(first, second);
  return (static_cast<std::uint64_t>(first) << 32) |
         static_cast<std::uint64_t>(second);
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

}  // namespace

StatusCode SegmentEdgeInferenceAlg::initialize() {
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_pairGateDecorKey.initialize());
  ATH_CHECK(m_filteredSegmentKey.initialize(!m_filteredSegmentKey.empty()));
  ATH_CHECK(m_edgeClassifier.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeInferenceAlg::execute(const EventContext& ctx) const {
  const xAOD::MuonSegmentContainer* segments{};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": input segments in '" << m_segmentKey.key()
                << "' = " << segments->size());

  SegmentEdgeGraph graph{};
  std::vector<SegmentEdgeScore> scores{};
  ATH_CHECK(m_edgeClassifier->buildGraph(ctx, *segments, graph));
  ATH_MSG_DEBUG("Event " << ctx.eventID().event_number()
                << ": built graph with nodes=" << graph.nNodes
                << ", edges=" << graph.nEdges);

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
    const std::size_t first = static_cast<std::size_t>(key >> 32);
    const std::size_t second = static_cast<std::size_t>(key & 0xffffffffu);
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
      const std::size_t first = static_cast<std::size_t>(edge.first >> 32);
      const std::size_t second = static_cast<std::size_t>(edge.first & 0xffffffffu);
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
      const std::size_t first = static_cast<std::size_t>(key >> 32);
      const std::size_t second =
          static_cast<std::size_t>(key & 0xffffffffu);
      if (first < graph.nNodes) selectedNode[first] = 1;
      if (second < graph.nNodes) selectedNode[second] = 1;
    }
    for (std::size_t node = 0; node < graph.nNodes; ++node) {
      if (selectedNode[node] || edgesByNode[node].empty()) continue;
      const std::uint64_t key = edgesByNode[node].front().first;
      const std::size_t first = static_cast<std::size_t>(key >> 32);
      const std::size_t second =
          static_cast<std::size_t>(key & 0xffffffffu);
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
    const std::size_t first = static_cast<std::size_t>(key >> 32);
    const std::size_t second = static_cast<std::size_t>(key & 0xffffffffu);
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

  std::vector<bool> keptNode(graph.nNodes, false);

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
  return StatusCode::SUCCESS;
}

} // namespace MuonML
