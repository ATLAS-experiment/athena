#include "SegmentEdgeClassifierTool.h"
#include "InferenceUtils.h"
#include "MuonInferenceInterfaces/GraphData.h"
#include "xAODMuon/MuonSegment.h"
#include "MuonPatternEvent/Segment.h"
#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "Acts/Utilities/Helpers.hpp"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <map>
#include <optional>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace {
/// Segments with the same detector bucket contribute to one occupancy value.
using SegmentBucketKey =
    std::tuple<int, int, int>;  // sector, chamberIndex, etaIndex

SegmentBucketKey segmentBucketKey(const xAOD::MuonSegment& seg) {
  return {seg.sector(), static_cast<int>(seg.chamberIndex()), seg.etaIndex()};
}

/// Number of unique layers among the space points of a bucket: the quantity
/// SegmentDumperAlg::countLayersInBucket writes to the bucket_layers branch
/// the edge models are trained on.
int layersInBucket(const MuonR4::SpacePointBucket& bucket) {
  MuonR4::SpacePointPerLayerSorter sorter{};
  std::vector<unsigned int> uniqueLayers;
  uniqueLayers.reserve(bucket.size());
  for (const MuonR4::SpacePointBucket::value_type& sp : bucket) {
    const unsigned int layNum = sorter.sectorLayerNum(*sp);
    if (!Acts::rangeContainsValue(uniqueLayers, layNum)) {
      uniqueLayers.push_back(layNum);
    }
  }
  return static_cast<int>(uniqueLayers.size());
}

/// Compute the minimum angular distance between sectors in a circular modulo space.
/// @param a First sector
/// @param b Second sector
/// @param mod Modulo value (e.g., 16); if <= 0, ordinary Euclidean distance is used
/// @return Minimum distance considering wrap-around
inline int sectorDistance(int a, int b, int mod) {
  int d = std::abs(a - b);
  return mod > 0 ? std::min(d, mod - d) : d;
}

std::optional<MuonML::SegmentNodeFeatureId> nodeFeatureIdFromName(const std::string& name) {
  using FeatureId = MuonML::SegmentNodeFeatureId;
  if (name == "segmentPositionX_m") return FeatureId::SegmentPositionX;
  if (name == "segmentPositionY_m") return FeatureId::SegmentPositionY;
  if (name == "segmentPositionZ_m") return FeatureId::SegmentPositionZ;
  if (name == "segmentDirectionX")  return FeatureId::SegmentDirectionX;
  if (name == "segmentDirectionY")  return FeatureId::SegmentDirectionY;
  if (name == "segmentDirectionZ")  return FeatureId::SegmentDirectionZ;
  if (name == "bucket_chamberIndex") return FeatureId::BucketChamberIndex;
  if (name == "bucket_layers")      return FeatureId::BucketLayers;
  if (name == "bucket_sector")      return FeatureId::BucketSector;
  if (name == "bucket_segments")    return FeatureId::BucketSegments;
  return std::nullopt;
}

float nodeFeatureValue(MuonML::SegmentNodeFeatureId feature,
                       const Amg::Vector3D& pos,
                       const Amg::Vector3D& dir,
                       const MuonML::BucketSegmentFeatures& bucket) {
  using FeatureId = MuonML::SegmentNodeFeatureId;
  switch (feature) {
    case FeatureId::SegmentPositionX: return static_cast<float>(pos.x());
    case FeatureId::SegmentPositionY: return static_cast<float>(pos.y());
    case FeatureId::SegmentPositionZ: return static_cast<float>(pos.z());
    case FeatureId::SegmentDirectionX: return static_cast<float>(dir.x());
    case FeatureId::SegmentDirectionY: return static_cast<float>(dir.y());
    case FeatureId::SegmentDirectionZ: return static_cast<float>(dir.z());
    case FeatureId::BucketChamberIndex: return static_cast<float>(bucket.chamberIndex);
    case FeatureId::BucketLayers: return static_cast<float>(bucket.layers);
    case FeatureId::BucketSector: return static_cast<float>(bucket.sector);
    case FeatureId::BucketSegments: return static_cast<float>(bucket.nSegments);
  }
  return 0.f;
}
}

namespace MuonML {

StatusCode SegmentEdgeClassifierTool::initialize() {
  if (m_sectorModulo.value() > 0 &&
      2ULL * static_cast<unsigned long long>(m_maxDeltaSector.value()) + 1ULL >
          static_cast<unsigned long long>(m_sectorModulo.value())) {
    ATH_MSG_ERROR("MaxDeltaSector=" << m_maxDeltaSector.value()
                  << " spans duplicate sectors for SectorModulo="
                  << m_sectorModulo.value());
    return StatusCode::FAILURE;
  }
  ATH_CHECK(setupModel());

  // Resolve node feature names from model metadata, matching the ONNX exporter.
  {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata meta = model().GetModelMetadata();
    auto keys = meta.GetCustomMetadataMapKeysAllocated(allocator);
    std::vector<std::string> keyList;
    keyList.reserve(keys.size());
    for (const auto& k : keys) keyList.emplace_back(k.get());

    constexpr std::array<std::string_view, 4> candidates{
        "x_feature_names", "node_feature_names", "feature_names", "input_feature_names"};
    std::string usedKey;
    std::vector<std::string> names;
    for (std::string_view key : candidates) {
      const std::string keyStr{key};
      if (std::find(keyList.begin(), keyList.end(), keyStr) == keyList.end()) continue;
      names = parseFeatureNames(meta.LookupCustomMetadataMapAllocated(keyStr.c_str(), allocator).get());
      if (!names.empty()) {
        usedKey = keyStr;
        break;
      }
    }

    if (names.empty()) {
      m_nodeFeatureNames.assign(kDefaultNodeFeatureNames.begin(), kDefaultNodeFeatureNames.end());
      ATH_MSG_WARNING("Model metadata has no usable node feature name key"
                      " (tried x_feature_names/node_feature_names/feature_names/input_feature_names)."
                      " Falling back to default training order.");
    } else {
      if (names.size() != kNodeFeatureCount) {
        ATH_MSG_ERROR("Model metadata key '" << usedKey << "' has " << names.size()
                      << " features, expected " << kNodeFeatureCount);
        return StatusCode::FAILURE;
      }
      for (const std::string& n : names) {
        if (!nodeFeatureIdFromName(n).has_value()) {
          ATH_MSG_ERROR("Unsupported node feature name in model metadata ('" << usedKey
                        << "'): '" << n << "'."
                        " Add mapping in SegmentEdgeClassifierTool::nodeFeatureValue().");
          return StatusCode::FAILURE;
        }
      }
      m_nodeFeatureNames = std::move(names);
      ATH_MSG_DEBUG("Using node feature names from model metadata key '" << usedKey << "'.");
    }

    m_nodeFeatureIds.reserve(m_nodeFeatureNames.size());
    for (const std::string& n : m_nodeFeatureNames) {
      const auto id = nodeFeatureIdFromName(n);
      if (!id.has_value()) {
        ATH_MSG_ERROR("Internal feature-id resolution failed for node feature name '" << n << "'.");
        return StatusCode::FAILURE;
      }
      m_nodeFeatureIds.push_back(*id);
    }

    std::ostringstream order;
    order << "Node feature order:";
    for (std::size_t i = 0; i < m_nodeFeatureNames.size(); ++i) {
      order << " f" << i << "=" << m_nodeFeatureNames[i];
      if (i + 1 < m_nodeFeatureNames.size()) order << ",";
    }
    ATH_MSG_DEBUG(order.str());
  }

  if (m_nodeFeatureNames.size() != kNodeFeatureCount) {
    ATH_MSG_ERROR("Internal node feature setup has " << m_nodeFeatureNames.size()
                  << " entries, expected " << kNodeFeatureCount);
    return StatusCode::FAILURE;
  }
  if (m_nodeFeatureIds.size() != kNodeFeatureCount) {
    ATH_MSG_ERROR("Internal node feature id setup has " << m_nodeFeatureIds.size()
                  << " entries, expected " << kNodeFeatureCount);
    return StatusCode::FAILURE;
  }

  m_cosMin = std::cos(m_maxDeltaThetaDeg.value() * Gaudi::Units::deg);

  if (!m_debugDumpFile.value().empty()) {
    std::ofstream out{m_debugDumpFile.value(), std::ios::out | std::ios::trunc};
    if (!out) {
      ATH_MSG_ERROR("Could not create segment-edge debug dump file: "
                    << m_debugDumpFile.value());
      return StatusCode::FAILURE;
    }

    nlohmann::ordered_json metadata;
    metadata["record_type"] = "metadata";
    metadata["format_version"] = 1;
    metadata["tool"] = "SegmentEdgeClassifierTool";
    metadata["input_names"] = {m_inputNodeName.value(),
                               m_inputEdgeIndexName.value(),
                               m_inputEdgeAttrName.value()};
    metadata["output_name"] = m_outputName.value();
    metadata["x_feature_names"] = m_nodeFeatureNames;
    metadata["edge_attr_feature_names"] = {
        "deltaPositionX_m", "deltaPositionY_m", "deltaPositionZ_m",
        "distance_m", "cos_opening_angle", "same_chamber", "same_sector"};
    metadata["edge_index_layout"] = "row_major_2_by_E";
    metadata["edge_order"] = "directed src_to_dst; row 0 then row 1";
    metadata["max_delta_theta_deg"] = m_maxDeltaThetaDeg.value();
    metadata["max_delta_sector"] = m_maxDeltaSector.value();
    metadata["sector_modulo"] = m_sectorModulo.value();
    metadata["debug_dump_max_events"] = m_debugDumpMaxEvents.value();
    out << metadata.dump() << '\n';

    ATH_MSG_INFO("Writing segment-edge ONNX debug dump to "
                 << m_debugDumpFile.value()
                 << " (DebugDumpMaxEvents="
                 << m_debugDumpMaxEvents.value() << ")");
  }

  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeClassifierTool::runGraphInference(const EventContext&, GraphRawData&) const {
  ATH_MSG_ERROR("runGraphInference is not supported by SegmentEdgeClassifierTool. Use SegmentEdgeInferenceAlg + ISegmentEdgeClassifierTool methods.");
  return StatusCode::FAILURE;
}

StatusCode SegmentEdgeClassifierTool::buildGraph(
    const EventContext&, const xAOD::MuonSegmentContainer& segments,
    SegmentEdgeGraph& graph) const {
  graph = SegmentEdgeGraph{};
  graph.segments.reserve(segments.size());
  graph.nodeFeatures.reserve(segments.size() * kNodeFeatureCount);
  // Evaluated once per event; short-circuits without touching the message
  // service unless the property was explicitly enabled.
  const bool truthDiag = m_enableTruthDiagnostics.value() && msgLvl(MSG::DEBUG);

  /*
   * Keep the original bucket multiplicity in the node feature even when the
   * speed configuration retains only the best representatives of a bucket.
   * This preserves the model's occupancy input while removing duplicate node
   * and edge work before tensor construction.
   */
  std::map<SegmentBucketKey, std::vector<const xAOD::MuonSegment*>>
      segmentsByBucket;
  for (const xAOD::MuonSegment* segment : segments) {
    segmentsByBucket[segmentBucketKey(*segment)].push_back(segment);
  }

  const InferenceUtils::SegmentQualityOrder betterSegment{};

  std::unordered_set<const xAOD::MuonSegment*> retainedSegments;
  retainedSegments.reserve(segments.size());
  for (auto& [_, bucketSegments] : segmentsByBucket) {
    std::ranges::sort(bucketSegments, betterSegment);
    const std::size_t nKeep = m_maxSegmentsPerBucket.value() == 0
                                  ? bucketSegments.size()
                                  : std::min<std::size_t>(
                                        bucketSegments.size(),
                                        m_maxSegmentsPerBucket.value());
    retainedSegments.insert(bucketSegments.begin(),
                            bucketSegments.begin() + nKeep);
  }

  std::vector<Amg::Vector3D> pos;
  std::vector<Amg::Vector3D> dir;
  std::vector<BucketSegmentFeatures> bucket;
  pos.reserve(retainedSegments.size());
  dir.reserve(retainedSegments.size());
  bucket.reserve(retainedSegments.size());

  for (const xAOD::MuonSegment* segment : segments) {
    if (!retainedSegments.contains(segment)) continue;

    const Amg::Vector3D position = segment->position();
    const Amg::Vector3D direction = segment->direction();
    const SegmentBucketKey key = segmentBucketKey(*segment);
    const auto bucketIt = segmentsByBucket.find(key);
    const int multiplicity =
        bucketIt == segmentsByBucket.end()
            ? 1
            : static_cast<int>(bucketIt->second.size());

    const int chamberIndex = static_cast<int>(segment->chamberIndex());
    const int layers = layersInBucket(*MuonR4::detailedSegment(*segment)->parent()->parentBucket());
    const int sector = segment->sector();

    graph.segments.push_back(segment);
    pos.emplace_back(position / Gaudi::Units::m);
    dir.emplace_back(direction);
    bucket.emplace_back(BucketSegmentFeatures{
        chamberIndex, layers, sector, multiplicity});
    for (const SegmentNodeFeatureId featureId : m_nodeFeatureIds) {
      graph.nodeFeatures.push_back(
          nodeFeatureValue(featureId, pos.back(), dir.back(), bucket.back()));
    }
  }
  graph.nNodes = graph.segments.size();

  if (pos.size() != graph.nNodes || dir.size() != graph.nNodes ||
      bucket.size() != graph.nNodes) {
    ATH_MSG_ERROR("Inconsistent vector sizes during graph building: nodes="
                  << graph.nNodes << ", pos=" << pos.size()
                  << ", dir=" << dir.size() << ", bucket=" << bucket.size());
    return StatusCode::FAILURE;
  }

  if (graph.nNodes < 2) {
    graph.nEdges = 0;
    if (truthDiag) fillTruthDiagnostics(segments, retainedSegments, graph);
    return StatusCode::SUCCESS;
  }

  const auto wrapRegularSector = [&](int sector) {
    // MuonSegment::sector() is a regular sector number, not an ExpandedSector
    // coordinate. Wrap it to [0, modulo); <= 0 disables wrapping.
    if (m_sectorModulo.value() > 0) {
      sector %= m_sectorModulo.value();
      if (sector < 0) sector += m_sectorModulo.value();
    }
    return sector;
  };

  // The lookup key must use the same wrapping as the target sectors below:
  // ATLAS sectors are 1-based (1..16), so a raw key of 16 can never match a
  // wrapped target of 0, which silently dropped every edge into sector 16.
  // The per-pair sectorDistance check below enforces the true circular
  // distance on the raw sector numbers.
  std::unordered_map<int, std::vector<std::size_t>> nodesBySector;
  nodesBySector.reserve(graph.nNodes);
  for (std::size_t node = 0; node < graph.nNodes; ++node) {
    nodesBySector[wrapRegularSector(bucket[node].sector)].push_back(node);
  }

  std::unordered_map<int, std::vector<int>> targetSectorsBySourceSector;
  targetSectorsBySourceSector.reserve(nodesBySector.size());
  std::size_t sectorLocalEdgeUpperBound = 0;
  for (const auto& [sourceSector, sourceNodes] : nodesBySector) {
    std::vector<int> targetSectors;
    targetSectors.reserve(2 * m_maxDeltaSector.value() + 1);
    for (int delta = -m_maxDeltaSector.value();
         delta <= m_maxDeltaSector.value(); ++delta) {
      targetSectors.push_back(wrapRegularSector(sourceSector + delta));
    }
    for (const int targetSector : targetSectors) {
      const auto found = nodesBySector.find(targetSector);
      if (found == nodesBySector.end()) continue;
      sectorLocalEdgeUpperBound += sourceNodes.size() * found->second.size();
      if (targetSector == sourceSector) {
        sectorLocalEdgeUpperBound -= sourceNodes.size();
      }
    }
    targetSectorsBySourceSector.emplace(sourceSector,
                                        std::move(targetSectors));
  }

  /*
   * The model receives a directed graph, but the geometric candidate relation
   * is undirected.  Build each pair once, then emit both directions.  With a
   * non-zero input cap, each endpoint nominates its best candidates and the
   * union is made bidirectional before inference; this preserves the message
   * passing symmetry expected by the GNN.
   */
  struct UndirectedEdge {
    std::size_t first{0};
    std::size_t second{0};
    float dx{0.f};
    float dy{0.f};
    float dz{0.f};
    float distance{0.f};
    float cosAngle{0.f};
  };
  const auto betterEdge = [](const UndirectedEdge& first,
                             const UndirectedEdge& second) {
    const int cosOrder = InferenceUtils::compareFloatDescending(
        first.cosAngle, second.cosAngle);
    if (cosOrder != 0) {
      return cosOrder < 0;
    }

    const int distanceOrder =
        InferenceUtils::compareFloat(first.distance, second.distance);
    if (distanceOrder != 0) {
      return distanceOrder < 0;
    }
    if (first.first != second.first) return first.first < second.first;
    return first.second < second.second;
  };
  const auto edgeKey = [](const UndirectedEdge& edge) {
    return (static_cast<std::uint64_t>(edge.first) << 32) |
           static_cast<std::uint64_t>(edge.second);
  };

  const unsigned int maxEdgesPerNode =
      m_maxEdgesPerNodeBeforeInference.value();
  const unsigned int maxEdgesPerTargetChamber =
      m_maxEdgesPerTargetChamberBeforeInference.value();
  const bool usePreInferenceSelection =
      maxEdgesPerNode != 0 || maxEdgesPerTargetChamber != 0;
  std::vector<std::vector<UndirectedEdge>> bestEdgesByNode;
  if (usePreInferenceSelection) {
    bestEdgesByNode.resize(graph.nNodes);
    const unsigned int reservePerNode =
        maxEdgesPerNode != 0 ? maxEdgesPerNode : maxEdgesPerTargetChamber;
    for (std::vector<UndirectedEdge>& edges : bestEdgesByNode) {
      edges.reserve(reservePerNode);
    }
  } else {
    graph.edgeIndex.reserve(2 * sectorLocalEdgeUpperBound);
    graph.edgeFeatures.reserve(kEdgeFeatureCount * sectorLocalEdgeUpperBound);
  }
  const auto appendDirectedPair = [&](const UndirectedEdge& edge) {
    graph.edgeIndex.push_back(static_cast<int64_t>(edge.first));
    graph.edgeIndex.push_back(static_cast<int64_t>(edge.second));
    graph.edgeFeatures.insert(
        graph.edgeFeatures.end(),
        {edge.dx, edge.dy, edge.dz, edge.distance, edge.cosAngle,
         float(bucket[edge.first].chamberIndex ==
               bucket[edge.second].chamberIndex),
         float(bucket[edge.first].sector == bucket[edge.second].sector)});

    graph.edgeIndex.push_back(static_cast<int64_t>(edge.second));
    graph.edgeIndex.push_back(static_cast<int64_t>(edge.first));
    graph.edgeFeatures.insert(
        graph.edgeFeatures.end(),
        {-edge.dx, -edge.dy, -edge.dz, edge.distance, edge.cosAngle,
         float(bucket[edge.first].chamberIndex ==
               bucket[edge.second].chamberIndex),
         float(bucket[edge.first].sector == bucket[edge.second].sector)});
  };

  const auto retainForNode = [&](std::size_t node,
                                 const UndirectedEdge& candidate) {
    std::vector<UndirectedEdge>& retained = bestEdgesByNode[node];
    const std::size_t other = candidate.first == node ? candidate.second
                                                       : candidate.first;
    const int targetChamber = bucket[other].chamberIndex;

    if (maxEdgesPerTargetChamber != 0) {
      unsigned int sameChamberCount = 0;
      auto worstSameChamber = retained.end();
      for (auto it = retained.begin(); it != retained.end(); ++it) {
        const std::size_t retainedOther =
            it->first == node ? it->second : it->first;
        if (bucket[retainedOther].chamberIndex != targetChamber) continue;
        ++sameChamberCount;
        if (worstSameChamber == retained.end() ||
            betterEdge(*worstSameChamber, *it)) {
          worstSameChamber = it;
        }
      }
      if (sameChamberCount >= maxEdgesPerTargetChamber) {
        if (!betterEdge(candidate, *worstSameChamber)) return;
        *worstSameChamber = candidate;
      } else {
        retained.push_back(candidate);
      }
    } else {
      retained.push_back(candidate);
    }

    if (maxEdgesPerNode != 0 && retained.size() > maxEdgesPerNode) {
      auto worst = retained.begin();
      for (auto it = std::next(retained.begin()); it != retained.end(); ++it) {
        if (betterEdge(*worst, *it)) worst = it;
      }
      retained.erase(worst);
    }
  };

  std::size_t candidatePairs = 0;
  for (std::size_t first = 0; first < graph.nNodes; ++first) {
    const auto sectorsIt =
        targetSectorsBySourceSector.find(wrapRegularSector(bucket[first].sector));
    if (sectorsIt == targetSectorsBySourceSector.end()) continue;
    for (const int sector : sectorsIt->second) {
      const auto targetIt = nodesBySector.find(sector);
      if (targetIt == nodesBySector.end()) continue;

      for (const std::size_t second : targetIt->second) {
        // Every valid pair will be visited from the lower-index endpoint.
        if (second <= first) continue;
        if (sectorDistance(bucket[first].sector, bucket[second].sector,
                           m_sectorModulo.value()) >
            m_maxDeltaSector.value()) {
          continue;
        }
        if (m_dropSameChamberEdgesBeforeInference.value() &&
            bucket[first].chamberIndex == bucket[second].chamberIndex) {
          continue;
        }
        const float cosAngle = static_cast<float>(dir[first].dot(dir[second]));
        if (cosAngle < m_cosMin) continue;

        const Amg::Vector3D delta = pos[second] - pos[first];
        const UndirectedEdge candidate{
            first,
            second,
            static_cast<float>(delta.x()),
            static_cast<float>(delta.y()),
            static_cast<float>(delta.z()),
            static_cast<float>(delta.mag()),
            cosAngle};
        ++candidatePairs;

        if (!usePreInferenceSelection) {
          appendDirectedPair(candidate);
        } else {
          retainForNode(first, candidate);
          retainForNode(second, candidate);
        }
      }
    }
  }
  std::size_t retainedPairs = candidatePairs;
  if (usePreInferenceSelection) {
    const unsigned int selectedReservePerNode =
        maxEdgesPerNode != 0 ? maxEdgesPerNode : maxEdgesPerTargetChamber;
    std::unordered_set<std::uint64_t> selectedKeys;
    selectedKeys.reserve(graph.nNodes * selectedReservePerNode);
    std::vector<UndirectedEdge> selectedEdges;
    selectedEdges.reserve(graph.nNodes * selectedReservePerNode);

    for (const std::vector<UndirectedEdge>& nodeEdges : bestEdgesByNode) {
      for (const UndirectedEdge& edge : nodeEdges) {
        if (selectedKeys.insert(edgeKey(edge)).second) {
          selectedEdges.push_back(edge);
        }
      }
    }
    std::sort(selectedEdges.begin(), selectedEdges.end(),
              [](const UndirectedEdge& first,
                 const UndirectedEdge& second) {
                if (first.first != second.first) {
                  return first.first < second.first;
                }
                return first.second < second.second;
              });

    retainedPairs = selectedEdges.size();
    graph.edgeIndex.reserve(4 * retainedPairs);
    graph.edgeFeatures.reserve(2 * kEdgeFeatureCount * retainedPairs);
    for (const UndirectedEdge& edge : selectedEdges) {
      appendDirectedPair(edge);
    }
  }
  graph.nEdges = graph.edgeIndex.size() / 2;
  const std::size_t nodesBeforeIsolatedNodeDrop = graph.nNodes;
  if (m_dropIsolatedNodesBeforeInference.value() && graph.nEdges != 0) {
    std::vector<unsigned char> active(graph.nNodes, 0);
    for (const int64_t index : graph.edgeIndex) {
      active[static_cast<std::size_t>(index)] = 1;
    }
    const std::size_t activeNodes =
        std::count(active.begin(), active.end(), static_cast<unsigned char>(1));
    if (activeNodes != graph.nNodes) {
      std::vector<std::size_t> oldToNew(graph.nNodes, graph.nNodes);
      std::vector<const xAOD::MuonSegment*> compactedSegments;
      std::vector<float> compactedNodeFeatures;
      compactedSegments.reserve(activeNodes);
      compactedNodeFeatures.reserve(activeNodes * kNodeFeatureCount);
      for (std::size_t oldNode = 0; oldNode < graph.nNodes; ++oldNode) {
        if (!active[oldNode]) continue;
        oldToNew[oldNode] = compactedSegments.size();
        compactedSegments.push_back(graph.segments[oldNode]);
        const auto featureBegin = graph.nodeFeatures.begin() +
            oldNode * kNodeFeatureCount;
        compactedNodeFeatures.insert(compactedNodeFeatures.end(),
                                     featureBegin,
                                     featureBegin + kNodeFeatureCount);
      }
      for (int64_t& index : graph.edgeIndex) {
        index = static_cast<int64_t>(oldToNew[static_cast<std::size_t>(index)]);
      }
      graph.segments = std::move(compactedSegments);
      graph.nodeFeatures = std::move(compactedNodeFeatures);
      graph.nNodes = activeNodes;
    }
  }
  if (truthDiag) fillTruthDiagnostics(segments, retainedSegments, graph);
  ATH_MSG_DEBUG("buildGraph: input segments=" << segments.size()
                << ", kept nodes=" << graph.nNodes
                << ", nodes before isolated-node drop=" << nodesBeforeIsolatedNodeDrop
                << ", bucket cap=" << m_maxSegmentsPerBucket.value()
                << ", candidate pairs=" << candidatePairs
                << ", retained pairs=" << retainedPairs
                << ", built directed edges=" << graph.nEdges
                << ", pre-inference node cap=" << m_maxEdgesPerNodeBeforeInference.value()
                << ", per-target-chamber cap=" << maxEdgesPerTargetChamber
                << ", drop same chamber=" << m_dropSameChamberEdgesBeforeInference.value()
                << ", drop isolated nodes=" << m_dropIsolatedNodesBeforeInference.value()
                << ", sector-local reserve=" << sectorLocalEdgeUpperBound);

  // Job-summed diagnostics 
  if (msgLvl(MSG::DEBUG)) {
    m_sumInputSegments += segments.size();
    m_sumCandidatePairs += candidatePairs;
    m_sumRetainedPairs += retainedPairs;
    m_sumNodesBeforeIsolatedDrop += nodesBeforeIsolatedNodeDrop;
    m_sumNodesAfterIsolatedDrop += graph.nNodes;
  }
  return StatusCode::SUCCESS;
}

void SegmentEdgeClassifierTool::fillTruthDiagnostics(
    const xAOD::MuonSegmentContainer& segments,
    const std::unordered_set<const xAOD::MuonSegment*>& bucketRetained,
    SegmentEdgeGraph& graph) const {
  std::unordered_map<const xAOD::MuonSegment*, std::int32_t> nodeOf;
  nodeOf.reserve(graph.segments.size());
  for (std::size_t node = 0; node < graph.segments.size(); ++node) {
    nodeOf.emplace(graph.segments[node], static_cast<std::int32_t>(node));
  }

  graph.inputNodeIndex.assign(segments.size(), kDroppedAsIsolated);
  std::unordered_map<std::int32_t, std::vector<std::uint32_t>> byTruth;
  std::uint32_t inputIndex = 0;
  for (const xAOD::MuonSegment* segment : segments) {
    const auto found = nodeOf.find(segment);
    if (found != nodeOf.end()) {
      graph.inputNodeIndex[inputIndex] = found->second;
    } else if (!bucketRetained.contains(segment)) {
      graph.inputNodeIndex[inputIndex] = kDroppedByBucketCap;
    }
    if (const xAOD::TruthParticle* truthPart =
            MuonR4::getTruthMatchedParticle(*segment)) {
      byTruth[static_cast<std::int32_t>(truthPart->index())].push_back(inputIndex);
    }
    ++inputIndex;
  }

  const auto pairKey = [](std::uint64_t first, std::uint64_t second) {
    return first < second ? (first << 32) | second : (second << 32) | first;
  };
  std::unordered_set<std::uint64_t> scoredPairs;
  scoredPairs.reserve(graph.nEdges);
  for (std::size_t edge = 0; edge < graph.nEdges; ++edge) {
    scoredPairs.insert(
        pairKey(static_cast<std::uint64_t>(graph.edgeIndex[2 * edge]),
                static_cast<std::uint64_t>(graph.edgeIndex[2 * edge + 1])));
  }

  // Same order as the pair loop in buildGraph(): bucket cap (node level),
  // sector window, same-chamber drop, angle window; a pair that clears all of
  // them but was not scored can only have been evicted by the edge caps.
  for (const auto& entry : byTruth) {
    const std::vector<std::uint32_t>& members = entry.second;
    for (std::size_t x = 0; x < members.size(); ++x) {
      for (std::size_t y = x + 1; y < members.size(); ++y) {
        const std::uint32_t i = members[x];
        const std::uint32_t j = members[y];
        const xAOD::MuonSegment* first = segments[i];
        const xAOD::MuonSegment* second = segments[j];
        PairFate fate{i, j, PairGate::Scored, 0};
        const int sectorDelta = sectorDistance(first->sector(), second->sector(),
                                               m_sectorModulo.value());
        fate.sectorDelta = static_cast<std::uint8_t>(std::min(sectorDelta, 255));
        if (!bucketRetained.contains(first) || !bucketRetained.contains(second)) {
          fate.gate = PairGate::BucketCap;
        } else if (sectorDelta > m_maxDeltaSector.value()) {
          fate.gate = PairGate::SectorWindow;
        } else if (m_dropSameChamberEdgesBeforeInference.value() &&
                   first->chamberIndex() == second->chamberIndex()) {
          fate.gate = PairGate::SameChamber;
        } else if (static_cast<float>(first->direction().dot(second->direction())) <
                   m_cosMin) {
          fate.gate = PairGate::AngleWindow;
        } else {
          const std::int32_t firstNode = graph.inputNodeIndex[i];
          const std::int32_t secondNode = graph.inputNodeIndex[j];
          const bool scored =
              firstNode >= 0 && secondNode >= 0 &&
              scoredPairs.contains(pairKey(static_cast<std::uint64_t>(firstNode),
                                           static_cast<std::uint64_t>(secondNode)));
          if (!scored) fate.gate = PairGate::EdgeCaps;
        }
        graph.truthPairFates.push_back(fate);
      }
    }
  }
}

StatusCode SegmentEdgeClassifierTool::finalize() {
  ATH_MSG_DEBUG(
      "SegmentEdgeClassifierTool pre-ONNX pruning summary (job-summed, "
      "independent of PairGateThreshold): "
      << "inputSegments=" << m_sumInputSegments
      << ", candidatePairs=" << m_sumCandidatePairs
      << " (geometric pairs before MaxEdgesPerNodeBeforeInference/"
         "MaxEdgesPerTargetChamberBeforeInference caps)"
      << ", retainedPairs=" << m_sumRetainedPairs
      << " (pairs actually sent to ONNX; MaxEdgesPerNodeBeforeInference="
      << m_maxEdgesPerNodeBeforeInference.value()
      << ", MaxEdgesPerTargetChamberBeforeInference="
      << m_maxEdgesPerTargetChamberBeforeInference.value() << ")"
      << ", nodesBeforeIsolatedDrop=" << m_sumNodesBeforeIsolatedDrop
      << ", nodesAfterIsolatedDrop=" << m_sumNodesAfterIsolatedDrop
      << " (DropIsolatedNodesBeforeInference="
      << m_dropIsolatedNodesBeforeInference.value()
      << "; nodes dropped here never reached ONNX or the pair-gate threshold)");
  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeClassifierTool::classifyEdges(const EventContext& ctx,
                                                     const SegmentEdgeGraph& graph,
                                                     std::vector<SegmentEdgeScore>& scores) const {
  scores.clear();
  if (!graph.nNodes) return StatusCode::SUCCESS;
  if (!graph.nEdges) {
    ATH_CHECK(dumpDebugEvent(ctx, graph, scores));
    return StatusCode::SUCCESS;
  }

  if (graph.nodeFeatures.size() != graph.nNodes * kNodeFeatureCount) {
    ATH_MSG_ERROR("Unexpected node feature size " << graph.nodeFeatures.size()
                  << "; expected " << (graph.nNodes * kNodeFeatureCount));
    return StatusCode::FAILURE;
  }
  if (graph.edgeIndex.size() != 2 * graph.nEdges) {
    ATH_MSG_ERROR("Unexpected edge index size " << graph.edgeIndex.size()
                  << "; expected " << (2 * graph.nEdges));
    return StatusCode::FAILURE;
  }
  if (graph.edgeFeatures.size() != graph.nEdges * kEdgeFeatureCount) {
    ATH_MSG_ERROR("Unexpected edge feature size " << graph.edgeFeatures.size()
                  << "; expected " << (graph.nEdges * kEdgeFeatureCount));
    return StatusCode::FAILURE;
  }

  GraphRawData raw{};
  raw.graph = std::make_unique<InferenceGraph>();
  raw.edgeIndexPacked.resize(2 * graph.nEdges);
  for (std::size_t e = 0; e < graph.nEdges; ++e) {
    raw.edgeIndexPacked[e] = graph.edgeIndex[2 * e];
    raw.edgeIndexPacked[graph.nEdges + e] = graph.edgeIndex[2 * e + 1];
  }

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  const std::vector<int64_t> nodeShape{static_cast<int64_t>(graph.nNodes), static_cast<int64_t>(kNodeFeatureCount)};
  // The graph outlives the synchronous ONNX call below.  Use its node
  // buffer directly instead of allocating and copying featureLeaves per event.
  ATLAS_THREAD_SAFE float* nodeFeaturesData =
      const_cast<float*>(graph.nodeFeatures.data());
  raw.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      nodeFeaturesData,
                                      graph.nodeFeatures.size(),
                                      nodeShape.data(),
                                      nodeShape.size()));

  const std::vector<int64_t> edgeIndexShape{2, static_cast<int64_t>(graph.nEdges)};
  raw.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<int64_t>(memInfo,
                                        raw.edgeIndexPacked.data(),
                                        raw.edgeIndexPacked.size(),
                                        edgeIndexShape.data(),
                                        edgeIndexShape.size()));

  // ONNX Runtime's CreateTensor API takes a non-const pointer, but it does not
  // mutate input buffers during inference. Avoid copying edge_attr every event.
  ATLAS_THREAD_SAFE float* edgeFeaturesData = const_cast<float*>(graph.edgeFeatures.data());
  const std::vector<int64_t> edgeAttrShape{static_cast<int64_t>(graph.nEdges), static_cast<int64_t>(kEdgeFeatureCount)};
  raw.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      edgeFeaturesData,
                                      graph.edgeFeatures.size(),
                                      edgeAttrShape.data(),
                                      edgeAttrShape.size()));

  const std::vector<const char*> inputNames{
      m_inputNodeName.value().c_str(),
      m_inputEdgeIndexName.value().c_str(),
      m_inputEdgeAttrName.value().c_str()};
  const std::vector<const char*> outputNames{m_outputName.value().c_str()};
  ATH_MSG_DEBUG("classifyEdges: ONNX inputs shapes x=[" << nodeShape[0] << "," << nodeShape[1]
                << "], edge_index=[" << edgeIndexShape[0] << "," << edgeIndexShape[1]
                << "], edge_attr=[" << edgeAttrShape[0] << "," << edgeAttrShape[1] << "]");
  ATH_CHECK(runNamedInference(raw, inputNames, outputNames));

  if (raw.graph->dataTensor.size() <= inputNames.size()) {
    ATH_MSG_ERROR("Missing ONNX output tensor for segment edge inference");
    return StatusCode::FAILURE;
  }

  const Ort::Value& outTensor = raw.graph->dataTensor[inputNames.size()];
  const auto outInfo = outTensor.GetTensorTypeAndShapeInfo();
  const std::vector<int64_t> outShape = outInfo.GetShape();
  const size_t outSize = outInfo.GetElementCount();
  if (!outShape.empty()) {
    ATH_MSG_DEBUG("classifyEdges: ONNX output rank=" << outShape.size()
                  << ", first dim=" << outShape.front()
                  << ", elements=" << outSize);
  } else {
    ATH_MSG_DEBUG("classifyEdges: ONNX scalar output, elements=" << outSize);
  }
  if (outSize < graph.nEdges) {
    ATH_MSG_ERROR("ONNX logits tensor has " << outSize << " entries for " << graph.nEdges << " edges");
    return StatusCode::FAILURE;
  }

  const float* logits = outTensor.GetTensorData<float>();
  scores.reserve(graph.nEdges);
  for (std::size_t e=0; e<graph.nEdges; ++e) {
    const float l = logits[e];
    scores.push_back({std::size_t(graph.edgeIndex[2 * e]),
                      std::size_t(graph.edgeIndex[2 * e + 1]),
                      l,
                      InferenceUtils::sigmoid(l)});
  }

  ATH_CHECK(dumpDebugEvent(ctx, graph, scores));
  return StatusCode::SUCCESS;
}

StatusCode SegmentEdgeClassifierTool::dumpDebugEvent(
    const EventContext& ctx,
    const SegmentEdgeGraph& graph,
    const std::vector<SegmentEdgeScore>& scores) const {
  if (m_debugDumpFile.value().empty()) return StatusCode::SUCCESS;

  std::lock_guard<std::mutex> lock{m_debugDumpMutex};
  if (m_debugDumpMaxEvents.value() != 0 &&
      m_debugDumpEvents.load(std::memory_order_relaxed) >=
          m_debugDumpMaxEvents.value()) {
    return StatusCode::SUCCESS;
  }

  if (graph.nodeFeatures.size() != graph.nNodes * kNodeFeatureCount ||
      graph.edgeIndex.size() != graph.nEdges * 2 ||
      graph.edgeFeatures.size() != graph.nEdges * kEdgeFeatureCount ||
      scores.size() != graph.nEdges) {
    ATH_MSG_ERROR("Cannot write segment-edge debug dump: inconsistent graph/output sizes"
                  << " nodes=" << graph.nNodes
                  << " nodeFeatures=" << graph.nodeFeatures.size()
                  << " edges=" << graph.nEdges
                  << " edgeIndex=" << graph.edgeIndex.size()
                  << " edgeFeatures=" << graph.edgeFeatures.size()
                  << " scores=" << scores.size());
    return StatusCode::FAILURE;
  }

  nlohmann::json x = nlohmann::json::array();
  x.get_ref<nlohmann::json::array_t&>().reserve(graph.nodeFeatures.size());
  for (const float value : graph.nodeFeatures) {
    x.push_back(std::isfinite(value) ? nlohmann::json(value)
                                     : nlohmann::json(nullptr));
  }

  nlohmann::json edgeIndex = nlohmann::json::array();
  edgeIndex.get_ref<nlohmann::json::array_t&>().reserve(graph.nEdges * 2);
  // This is the actual ONNX [2,E] row-major buffer: all sources then all destinations.
  for (std::size_t edge = 0; edge < graph.nEdges; ++edge) {
    edgeIndex.push_back(graph.edgeIndex[2 * edge]);
  }
  for (std::size_t edge = 0; edge < graph.nEdges; ++edge) {
    edgeIndex.push_back(graph.edgeIndex[2 * edge + 1]);
  }

  nlohmann::json edgeAttr = nlohmann::json::array();
  edgeAttr.get_ref<nlohmann::json::array_t&>().reserve(graph.edgeFeatures.size());
  for (const float value : graph.edgeFeatures) {
    edgeAttr.push_back(std::isfinite(value) ? nlohmann::json(value)
                                            : nlohmann::json(nullptr));
  }

  nlohmann::json logits = nlohmann::json::array();
  nlohmann::json probabilities = nlohmann::json::array();
  nlohmann::json edgeSrc = nlohmann::json::array();
  nlohmann::json edgeDst = nlohmann::json::array();
  logits.get_ref<nlohmann::json::array_t&>().reserve(scores.size());
  probabilities.get_ref<nlohmann::json::array_t&>().reserve(scores.size());
  edgeSrc.get_ref<nlohmann::json::array_t&>().reserve(scores.size());
  edgeDst.get_ref<nlohmann::json::array_t&>().reserve(scores.size());
  for (const SegmentEdgeScore& score : scores) {
    edgeSrc.push_back(score.src);
    edgeDst.push_back(score.dst);
    logits.push_back(std::isfinite(score.logit) ? nlohmann::json(score.logit)
                                                : nlohmann::json(nullptr));
    probabilities.push_back(std::isfinite(score.probability)
                                ? nlohmann::json(score.probability)
                                : nlohmann::json(nullptr));
  }

  std::ofstream out{m_debugDumpFile.value(), std::ios::out | std::ios::app};
  if (!out) {
    ATH_MSG_ERROR("Could not append to segment-edge debug dump file: "
                  << m_debugDumpFile.value());
    return StatusCode::FAILURE;
  }

  const unsigned int dumpIndex =
      m_debugDumpEvents.fetch_add(1, std::memory_order_relaxed);
  nlohmann::ordered_json event;
  event["record_type"] = "event";
  event["format_version"] = 1;
  event["dump_index"] = dumpIndex;
  event["run_number"] = ctx.eventID().run_number();
  event["lumi_block"] = ctx.eventID().lumi_block();
  event["event_number"] = ctx.eventID().event_number();
  event["slot"] = ctx.slot();
  event["n_nodes"] = graph.nNodes;
  event["n_edges"] = graph.nEdges;
  event["x_shape"] = {graph.nNodes, kNodeFeatureCount};
  event["edge_index_shape"] = {2, graph.nEdges};
  event["edge_attr_shape"] = {graph.nEdges, kEdgeFeatureCount};
  event["logits_shape"] = {graph.nEdges};
  event["x"] = std::move(x);
  event["edge_index"] = std::move(edgeIndex);
  event["edge_attr"] = std::move(edgeAttr);
  event["edge_src"] = std::move(edgeSrc);
  event["edge_dst"] = std::move(edgeDst);
  event["logits"] = std::move(logits);
  event["probabilities"] = std::move(probabilities);
  out << event.dump() << '\n';

  ATH_MSG_DEBUG("Wrote segment-edge debug event " << dumpIndex
                << " to " << m_debugDumpFile.value());

  return StatusCode::SUCCESS;
}

} // namespace MuonML
