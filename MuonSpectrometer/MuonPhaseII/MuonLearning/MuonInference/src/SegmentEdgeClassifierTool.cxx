#include "SegmentEdgeClassifierTool.h"
#include "InferenceUtils.h"
#include "MuonInferenceInterfaces/GraphData.h"
#include "xAODMuon/MuonSegment.h"
#include "MuonPatternEvent/Segment.h"
#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "Acts/Utilities/Helpers.hpp"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <mutex>
#include <map>
#include <optional>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace {
using SegmentGroupKey = std::tuple<int, int, int>; // sector, chamberIndex, etaIndex

SegmentGroupKey segmentGroupKey(const xAOD::MuonSegment& seg) {
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

StatusCode SegmentEdgeClassifierTool::buildGraph(const EventContext&, const xAOD::MuonSegmentContainer& segments, SegmentEdgeGraph& graph) const {
  graph = SegmentEdgeGraph{};
  graph.nNodes = segments.size();
  graph.segments.reserve(graph.nNodes);
  graph.nodeFeatures.reserve(graph.nNodes * kNodeFeatureCount);

  std::vector<Amg::Vector3D> pos, dir;
  std::vector<BucketSegmentFeatures> bucket;
  pos.reserve(graph.nNodes); dir.reserve(graph.nNodes); bucket.reserve(graph.nNodes);

  std::map<SegmentGroupKey, int> segmentMultiplicity{};
  for (const xAOD::MuonSegment* seg : segments) {
    if (!seg) continue;
    ++segmentMultiplicity[segmentGroupKey(*seg)];
  }

  for (const xAOD::MuonSegment* seg : segments) {
    if (!seg) continue;
    const Amg::Vector3D p = seg->position();
    Amg::Vector3D d = seg->direction();

    const int chamberIdx = static_cast<int>(seg->chamberIndex());
    const int layers = layersInBucket(*MuonR4::detailedSegment(*seg)->parent()->parentBucket());
    const int sec = seg->sector();
    const auto multIt = segmentMultiplicity.find(segmentGroupKey(*seg));
    const int nSeg = (multIt != segmentMultiplicity.end()) ? multIt->second : 1;

    graph.segments.push_back(seg);
    pos.emplace_back(p.x() / Gaudi::Units::m,
                     p.y() / Gaudi::Units::m,
                     p.z() / Gaudi::Units::m);
    dir.emplace_back(d.x(), d.y(), d.z());
    bucket.emplace_back(BucketSegmentFeatures{chamberIdx, layers, sec, nSeg});
    for (const SegmentNodeFeatureId featureId : m_nodeFeatureIds) {
      graph.nodeFeatures.push_back(nodeFeatureValue(featureId, pos.back(), dir.back(), bucket.back()));
    }
  }
  graph.nNodes = graph.segments.size();

  // Consistency check: all vectors must have same size
  if (pos.size() != graph.nNodes || dir.size() != graph.nNodes || bucket.size() != graph.nNodes) {
    ATH_MSG_ERROR("Inconsistent vector sizes during graph building: nodes=" << graph.nNodes
                  << ", pos=" << pos.size() << ", dir=" << dir.size() << ", bucket=" << bucket.size());
    return StatusCode::FAILURE;
  }

  if (graph.nNodes < 2) {
    graph.nEdges = 0;
    return StatusCode::SUCCESS;
  }

  auto normalizeSector = [&](int s) {
    // m_sectorModulo > 0: wrap sector to [0, modulo); <=0: disable wrapping
    if (m_sectorModulo.value() > 0) {
      s %= m_sectorModulo.value();
      if (s < 0) s += m_sectorModulo.value();
    }
    return s;
  };

  // The lookup key must use the same wrapping as the target sectors below:
  // ATLAS sectors are 1-based (1..16), so a raw key of 16 can never match a
  // wrapped target of 0, which silently dropped every edge into sector 16.
  // The per-pair sectorDistance check below enforces the true circular
  // distance on the raw sector numbers.
  std::unordered_map<int, std::vector<std::size_t>> nodesBySector;
  nodesBySector.reserve(graph.nNodes);
  for (std::size_t i = 0; i < graph.nNodes; ++i) {
    nodesBySector[normalizeSector(bucket[i].sector)].push_back(i);
  }

  const std::size_t maxEdges = graph.nNodes * (graph.nNodes - 1);
  graph.edgeIndex.reserve(2 * maxEdges);
  graph.edgeFeatures.reserve(kEdgeFeatureCount * maxEdges);

  for (std::size_t i = 0; i < graph.nNodes; ++i) {
    std::unordered_set<int> targetSectors;
    targetSectors.reserve(2 * m_maxDeltaSector.value() + 1);
    for (int delta = -m_maxDeltaSector.value(); delta <= m_maxDeltaSector.value(); ++delta) {
      targetSectors.insert(normalizeSector(bucket[i].sector + delta));
    }

    for (const int sec : targetSectors) {
      auto it = nodesBySector.find(sec);
      if (it == nodesBySector.end()) continue;
      for (const std::size_t j : it->second) {
        if (i == j) continue;
        if (sectorDistance(bucket[i].sector, bucket[j].sector, m_sectorModulo.value()) > m_maxDeltaSector.value()) continue;
        const float cosang = static_cast<float>(dir[i].dot(dir[j]));
        if (cosang < m_cosMin) continue;

        graph.edgeIndex.push_back(static_cast<int64_t>(i));
        graph.edgeIndex.push_back(static_cast<int64_t>(j));

        const Amg::Vector3D delta = pos[j] - pos[i];
        const float dx = static_cast<float>(delta.x());
        const float dy = static_cast<float>(delta.y());
        const float dz = static_cast<float>(delta.z());
        const float dist = static_cast<float>(delta.mag());
        graph.edgeFeatures.insert(graph.edgeFeatures.end(), {dx,dy,dz,dist,cosang, float(bucket[i].chamberIndex==bucket[j].chamberIndex), float(bucket[i].sector==bucket[j].sector)});
      }
    }
  }
  graph.nEdges = graph.edgeIndex.size() / 2;
  ATH_MSG_DEBUG("buildGraph: input segments=" << segments.size()
                << ", kept nodes=" << graph.nNodes
                << ", built edges=" << graph.nEdges);
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
  raw.featureLeaves = graph.nodeFeatures;
  raw.edgeIndexPacked.reserve(2 * graph.nEdges);
  raw.srcEdges.reserve(graph.nEdges);
  raw.desEdges.reserve(graph.nEdges);
  for (std::size_t e = 0; e < graph.nEdges; ++e) {
    raw.srcEdges.push_back(graph.edgeIndex[2 * e]);
    raw.desEdges.push_back(graph.edgeIndex[2 * e + 1]);
  }
  raw.edgeIndexPacked.insert(raw.edgeIndexPacked.end(), raw.srcEdges.begin(), raw.srcEdges.end());
  raw.edgeIndexPacked.insert(raw.edgeIndexPacked.end(), raw.desEdges.begin(), raw.desEdges.end());

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  const std::vector<int64_t> nodeShape{static_cast<int64_t>(graph.nNodes), static_cast<int64_t>(kNodeFeatureCount)};
  raw.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      raw.featureLeaves.data(),
                                      raw.featureLeaves.size(),
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
