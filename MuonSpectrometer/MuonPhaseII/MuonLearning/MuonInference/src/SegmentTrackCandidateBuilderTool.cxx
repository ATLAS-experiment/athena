#include "SegmentTrackCandidateBuilderTool.h"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <set>
#include <unordered_map>

namespace MuonML {
namespace {

// Node index container reused by candidate and component builders.
using NodeVec = std::vector<std::size_t>;

// Disjoint-set union for connected-component clustering.
struct DSU {
  NodeVec p;
  explicit DSU(std::size_t n) : p(n) { std::iota(p.begin(), p.end(), 0); }
  // Terminates because roots satisfy p[root] == root; path compression shortens chains.
  std::size_t find(std::size_t x) { return p[x] == x ? x : p[x] = find(p[x]); }
  void unite(std::size_t a, std::size_t b) {
    a = find(a);
    b = find(b);
    if (a != b) p[b] = a;
  }
};

void addUnique(std::vector<unsigned>& ids, unsigned id) {
  if (std::find(ids.begin(), ids.end(), id) == ids.end()) ids.push_back(id);
}

std::uint64_t pairKey(std::size_t a, std::size_t b) {
  if (a > b) std::swap(a, b);
  return (static_cast<std::uint64_t>(a) << 32) | static_cast<std::uint64_t>(b);
}

std::pair<std::size_t, std::size_t> unpackPairKey(std::uint64_t key) {
  return {static_cast<std::size_t>(key >> 32),
          static_cast<std::size_t>(key & 0xffffffffu)};
}

std::unordered_map<std::uint64_t, float>
makeUndirectedScores(const SegmentEdgeGraph& graph,
                     const std::vector<SegmentEdgeScore>& scores,
                     const bool symmetrize) {
  std::unordered_map<std::uint64_t, float> pairScore;
  pairScore.reserve(scores.size());
  for (const SegmentEdgeScore& s : scores) {
    if (s.src >= graph.nNodes || s.dst >= graph.nNodes || s.src == s.dst) continue;
    const std::uint64_t key = symmetrize ? pairKey(s.src, s.dst)
                                         : ((static_cast<std::uint64_t>(s.src) << 32) |
                                            static_cast<std::uint64_t>(s.dst));
    auto [it, inserted] = pairScore.emplace(key, s.probability);
    if (!inserted) it->second = std::max(it->second, s.probability);
  }
  return pairScore;
}

std::vector<NodeVec> componentsAboveThreshold(const SegmentEdgeGraph& graph,
                                              const std::unordered_map<std::uint64_t, float>& pairScore,
                                              const float threshold,
                                              const bool keepIsolated) {
  DSU dsu(graph.nNodes);
  std::vector<char> touched(graph.nNodes, false);

  for (const auto& [key, prob] : pairScore) {
    if (prob < threshold) continue;
    const auto [a, b] = unpackPairKey(key);
    if (a >= graph.nNodes || b >= graph.nNodes) continue;
    dsu.unite(a, b);
    touched[a] = true;
    touched[b] = true;
  }

  std::unordered_map<std::size_t, NodeVec> byRoot;
  byRoot.reserve(graph.nNodes);
  for (std::size_t i = 0; i < graph.nNodes; ++i) {
    if (!touched[i] && !keepIsolated) continue;
    byRoot[dsu.find(i)].push_back(i);
  }

  std::vector<NodeVec> out;
  out.reserve(byRoot.size());
  for (auto& [_, nodes] : byRoot) {
    std::ranges::sort(nodes);
    out.push_back(std::move(nodes));
  }
  return out;
}
}

StatusCode SegmentTrackCandidateBuilderTool::buildCandidates(
    const EventContext&,
    const SegmentEdgeGraph& graph,
    const std::vector<SegmentEdgeScore>& scores,
    std::vector<std::vector<unsigned>>& candidateIdsPerSegment) const {

  candidateIdsPerSegment.assign(graph.nNodes, {});

  if (graph.nNodes == 0) return StatusCode::SUCCESS;

  const auto pairScore = makeUndirectedScores(graph, scores, m_symmetrizeDirectedEdges.value());

  // 1. High-purity cores.
  std::vector<NodeVec> candidates =
      componentsAboveThreshold(graph, pairScore,
                               m_overlapThreshold.value(),
                               m_keepIsolatedSegments.value());

  // 2. Recall recovery components.
  //
  // This is the important no-loss improvement. A true trajectory may contain
  // only 0.3--0.7 edge probabilities, especially for difficult segments.
  // The previous implementation never created a candidate unless some edge
  // crossed OverlapThreshold. Here we add an extra low-threshold connected
  // component candidate.
  if (m_useRecoveryComponents.value()) {
    std::vector<NodeVec> recovery =
        componentsAboveThreshold(graph, pairScore,
                                 m_edgeThreshold.value(),
                                 m_keepIsolatedSegments.value());
    candidates.insert(candidates.end(),
                      std::make_move_iterator(recovery.begin()),
                      std::make_move_iterator(recovery.end()));
  }

  // 3. Optional validation/debug safety net.
  //
  // This should be disabled for production timing studies, but it is useful
  // when proving that remaining losses are due to the model/graph pruning
  // rather than the seeding wrapper.
  if (m_addAllSegmentsRecoveryCandidate.value() &&
      graph.nNodes >= m_minCandidateSize.value()) {
    NodeVec all(graph.nNodes);
    std::iota(all.begin(), all.end(), 0);
    candidates.push_back(std::move(all));
  }

  // 4. Remove exact duplicate candidate node sets and assign IDs.
  std::set<NodeVec> seen;
  unsigned next = 0;
  for (NodeVec& nodes : candidates) {
    std::sort(nodes.begin(), nodes.end());
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    if (nodes.size() < m_minCandidateSize.value()) continue;
    if (!seen.insert(nodes).second) continue;

    for (std::size_t n : nodes) {
      addUnique(candidateIdsPerSegment[n], next);
    }
    ++next;
  }

  for (auto& ids : candidateIdsPerSegment) {
    std::sort(ids.begin(), ids.end());
  }

  std::unordered_map<unsigned, unsigned> candSegCount;
  candSegCount.reserve(next);
  for (const auto& ids : candidateIdsPerSegment) {
    for (unsigned id : ids) ++candSegCount[id];
  }

  std::size_t assignedNodes = 0;
  std::size_t multiAssignedNodes = 0;
  for (const auto& ids : candidateIdsPerSegment) {
    assignedNodes += !ids.empty();
    multiAssignedNodes += ids.size() > 1;
  }

  ATH_MSG_DEBUG("buildCandidates: " << candSegCount.size()
                << " candidate(s) from " << graph.nNodes
                << " segment(s); assignedNodes=" << assignedNodes
                << ", multiAssignedNodes=" << multiAssignedNodes
                << ", edgeThreshold=" << m_edgeThreshold.value()
                << ", overlapThreshold=" << m_overlapThreshold.value()
                << ", recovery=" << m_useRecoveryComponents.value()
                << ", symmetrize=" << m_symmetrizeDirectedEdges.value());
  if (msgLvl(MSG::VERBOSE)) {
    for (const auto& [id, nSegs] : candSegCount) {
      ATH_MSG_VERBOSE("  candidate " << id << ": " << nSegs << " segment(s)");
    }
  }

  return StatusCode::SUCCESS;
}
} // namespace MuonML
