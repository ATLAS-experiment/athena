/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DVInferenceToolBase.h"

#include "InferenceUtils.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/ReadHandle.h"

#include "CaloEvent/CaloTower.h"
#include "CxxUtils/phihelper.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "xAODMuon/MuonSegment.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

using namespace MuonML;

namespace {

enum class NodeKind : uint8_t { Muon, Calo };

struct DVNodeAux {
  NodeKind kind{NodeKind::Muon};
  std::array<float, 7> features{};
  float eta{0.f};
  float phi{0.f};
  float energyLike{0.f};
  Amg::Vector3D direction{0., 0., 1.};
  int sector{-1};
};

using SegmentList = std::vector<const xAOD::MuonSegment*>;

void appendUniqueSegment(SegmentList& segments, const xAOD::MuonSegment* seg) {
  if (seg && !Acts::rangeContainsValue(segments, seg)) segments.push_back(seg);
}

uint16_t countLayersInBucket(const MuonR4::SpacePointBucket& bucket) {
  MuonR4::SpacePointPerLayerSorter sorter{};
  std::set<unsigned int> uniqueLayers{};
  for (const MuonR4::SpacePointBucket::value_type& sp : bucket) {
    uniqueLayers.insert(sorter.sectorLayerNum(*sp));
  }
  return static_cast<uint16_t>(uniqueLayers.size());
}

std::string bucketSignatureKey(const MuonR4::SpacePointBucket& bucket) {

  std::ostringstream sig;
  sig << static_cast<const void*>(bucket.msSector()) << '|'
      << bucket.msSector()->sector() << '|'
      << bucket.msSector()->side() << '|'
      << std::fixed << std::setprecision(3)
      << bucket.coveredMin() << '|' << bucket.coveredMax();
  return sig.str();
}

void appendMuonSegmentNode(const xAOD::MuonSegment& seg, int bucketSector, std::vector<DVNodeAux>& nodes) {
  const Amg::Vector3D posMm = seg.position();
  const Amg::Vector3D dir = seg.direction();
  const Amg::Vector3D posM = posMm / Gaudi::Units::m;

  DVNodeAux node{};
  node.kind = NodeKind::Muon;
  node.features[0] = static_cast<float>(posM.mag());
  node.features[1] = static_cast<float>(posM.theta());
  node.features[2] = static_cast<float>(posM.phi());
  node.features[3] = static_cast<float>(dir.theta());
  node.features[4] = static_cast<float>(dir.phi());
  node.features[5] = 0.f;
  node.features[6] = static_cast<float>(seg.numberDoF());
  node.eta = static_cast<float>(posMm.eta());
  node.phi = static_cast<float>(posMm.phi());
  node.energyLike = 0.f;
  node.direction = dir;
  node.sector = bucketSector;
  nodes.push_back(node);
}

bool hasName(const std::vector<std::string>& names, const std::string& needle) {
  return Acts::rangeContainsValue(names, needle);
}

std::string joinNames(const std::vector<std::string>& names) {
  std::ostringstream ostr;
  for (std::size_t i = 0; i < names.size(); ++i) {
    if (i != 0u) ostr << ", ";
    ostr << names[i];
  }
  return ostr.str();
}

std::optional<Amg::Vector3D> firstIntersectionWithEnvelope(const Amg::Vector3D& direction,
                                                           float rMaxMm,
                                                           float zMaxMm) {
  const float ux = static_cast<float>(direction.x());
  const float uy = static_cast<float>(direction.y());
  const float uz = static_cast<float>(direction.z());
  std::optional<float> bestT{};
  const float ur = static_cast<float>(direction.perp());
  if (rMaxMm > 0.f && ur > 0.f) {
    const float tBarrel = rMaxMm / ur;
    const float zBarrel = tBarrel * uz;
    if (std::abs(zBarrel) <= zMaxMm) bestT = tBarrel;
  }

  if (zMaxMm > 0.f && std::abs(uz) > 0.f) {
    const float tEndcap = zMaxMm / std::abs(uz);
    const float xEnd = tEndcap * ux;
    const float yEnd = tEndcap * uy;
    if (std::hypot(xEnd, yEnd) <= rMaxMm && (!bestT || tEndcap < *bestT)) {
      bestT = tEndcap;
    }
  }

  if (!bestT) return std::nullopt;
  return (*bestT) * direction;
}

}  // namespace

StatusCode DVInferenceToolBase::initialize() {
  ATH_CHECK(setupModel());
  if (m_singleOutputMode.value() != "auto" &&
      m_singleOutputMode.value() != "logit" &&
      m_singleOutputMode.value() != "prob") {
    ATH_MSG_ERROR("SingleOutputMode must be one of auto, logit, prob; got " << m_singleOutputMode.value());
    return StatusCode::FAILURE;
  }

  if (m_useBucketSegmentSelection.value() && m_spacePointKeys.size() > 1u) {
    ATH_MSG_WARNING("DV SpacePointKeys has " << m_spacePointKeys.size()
                    << " entries. The MuonBucketDump training samples use the "
                    << "default SegmentKey array with segments attached only to "
                    << "the first SpacePointKeys entry. For parity with training, "
                    << "configure SpacePointKeys=['MuonSpacePoints'] unless the "
                    << "training dump was produced with matching segment keys for all entries.");
  }

  ATH_MSG_INFO("Initialized DVInferenceToolBase with SegmentKey=" << m_segmentKey.key()
               << ", SpacePointKeys=" << m_spacePointKeys.size()
               << ", " << m_useBucketSegmentSelection
               << ", TowerContainerKey="
               << (m_towerKey.empty() ? std::string("<disabled>") : m_towerKey.key())
               << ", " << m_minTowerEnergyMeV
               << ", " << m_maxTowerSegmentDR
               << ", " << m_caloRMaxMm
               << ", " << m_caloZMaxMm
               << ", " << m_fallbackToAllSegments
               << ", " << m_singleOutputMode);
  return StatusCode::SUCCESS;
}

Ort::Session& DVInferenceToolBase::model() const {
  return m_onnxSessionTool->session();
}

std::vector<std::string> DVInferenceToolBase::modelInputNames() const {
  std::vector<std::string> names{};
  Ort::AllocatorWithDefaultOptions allocator;
  const std::size_t nInputs = model().GetInputCount();
  names.reserve(nInputs);
  for (std::size_t i = 0; i < nInputs; ++i) {
    auto name = model().GetInputNameAllocated(i, allocator);
    if (name) names.emplace_back(name.get());
  }
  return names;
}

std::vector<std::string> DVInferenceToolBase::modelOutputNames() const {
  std::vector<std::string> names{};
  Ort::AllocatorWithDefaultOptions allocator;
  const std::size_t nOutputs = model().GetOutputCount();
  names.reserve(nOutputs);
  for (std::size_t i = 0; i < nOutputs; ++i) {
    auto name = model().GetOutputNameAllocated(i, allocator);
    if (name) names.emplace_back(name.get());
  }
  return names;
}

StatusCode DVInferenceToolBase::setupModel() {
  ATH_CHECK(m_onnxSessionTool.retrieve());
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_spacePointKeys.initialize());
  ATH_CHECK(m_towerKey.initialize(SG::AllowEmpty));

  const InferenceUtils::SessionBackend backend = InferenceUtils::sessionBackend(m_onnxSessionTool);
  m_isCuda = backend.isCuda;
  m_cudaDeviceId = backend.cudaDeviceId;
  if (m_isCuda) {
    ATH_MSG_INFO("ONNX session is running on CUDA device " << m_cudaDeviceId
                 << ". I/O binding will be used.");
  } else {
    ATH_MSG_INFO("ONNX session is running on CPU.");
  }
  return StatusCode::SUCCESS;
}

StatusCode DVInferenceToolBase::runGraphInference(const EventContext& ctx,
                                                   GraphRawData& graphData) const {
  ATH_CHECK(buildGraph(ctx, graphData));
  if (!graphData.graph || graphData.graph->dataTensor.empty()) {
    ATH_MSG_DEBUG("DV graph has no input tensors; skip inference for this event.");
    return StatusCode::SUCCESS;
  }
  return runInference(graphData);
}

StatusCode DVInferenceToolBase::inferEvent(const EventContext& ctx,
                                            DVInferenceResult& result) const {
  result = DVInferenceResult{};
  GraphRawData graphData{};
  ATH_CHECK(buildGraph(ctx, graphData));
  if (!graphData.graph || graphData.graph->dataTensor.size() < kInputTensorCount) {
    ATH_MSG_WARNING("DV graph is empty; no event-classifier output will be produced.");
    return StatusCode::SUCCESS;
  }

  const auto xShape = graphData.graph->dataTensor[0].GetTensorTypeAndShapeInfo().GetShape();
  const auto edgeShape = graphData.graph->dataTensor[1].GetTensorTypeAndShapeInfo().GetShape();
  result.nNodes = !xShape.empty() && xShape[0] > 0 ? static_cast<std::size_t>(xShape[0]) : 0u;
  result.nEdges = edgeShape.size() > 1 && edgeShape[1] > 0 ? static_cast<std::size_t>(edgeShape[1]) : 0u;
  if (graphData.spacePointsInBucket.size() >= 2) {
    result.nMuonNodes = static_cast<std::size_t>(std::max<int64_t>(graphData.spacePointsInBucket[0], 0));
    result.nCaloNodes = static_cast<std::size_t>(std::max<int64_t>(graphData.spacePointsInBucket[1], 0));
  }

  ATH_CHECK(runInference(graphData));
  if (graphData.graph->dataTensor.size() <= kInputTensorCount) {
    ATH_MSG_ERROR("DV inference finished without an output tensor.");
    return StatusCode::FAILURE;
  }

  result.probability = probabilityFromOutput(graphData.graph->dataTensor.back(), result.rawOutput);
  result.valid = std::isfinite(result.probability);
  ATH_MSG_DEBUG("DV event classifier: N=" << result.nNodes
                << " (muon=" << result.nMuonNodes << ", calo=" << result.nCaloNodes
                << "), E=" << result.nEdges
                << ", raw=" << result.rawOutput
                << ", probability=" << result.probability);
  return StatusCode::SUCCESS;
}

StatusCode DVInferenceToolBase::buildGraph(const EventContext& ctx,
                                            GraphRawData& graphData) const {
  graphData.graph.reset();
  graphData.featureLeaves.clear();
  graphData.srcEdges.clear();
  graphData.desEdges.clear();
  graphData.edgeIndexPacked.clear();
  graphData.spacePointsInBucket.clear();
  graphData.graph = std::make_unique<InferenceGraph>();
  graphData.graph->dataTensor.reserve(kInputTensorCount);

  std::vector<DVNodeAux> nodes;

  const xAOD::MuonSegmentContainer* segments{nullptr};
  ATH_CHECK(SG::get(segments, m_segmentKey, ctx));

  nodes.reserve(segments ? segments->size() : 0u);

  if (segments && m_useBucketSegmentSelection.value() && !m_spacePointKeys.empty()) {
    using SegmentsPerBucket_t =
        std::unordered_map<const MuonR4::SpacePointBucket*, SegmentList>;

    using SegmentsPerBucketSignature_t =
        std::unordered_map<std::string, SegmentList>;

    SegmentsPerBucket_t segmentsPerBucket{};
    SegmentsPerBucketSignature_t segmentsPerBucketSignature{};
    for (const xAOD::MuonSegment* seg : *segments) {
      const auto* detailed = MuonR4::detailedSegment(*seg);
      const MuonR4::SpacePointBucket* parentBucket = detailed->parent()->parentBucket();
      appendUniqueSegment(segmentsPerBucket[parentBucket], seg);
      const std::string parentSig = bucketSignatureKey(*parentBucket);
      if (!parentSig.empty()) appendUniqueSegment(segmentsPerBucketSignature[parentSig], seg);
    }
    std::size_t nSignatureMatchedBuckets = 0u;
    for (const SG::ReadHandleKey<MuonR4::SpacePointContainer>& spKey : m_spacePointKeys) {
      const MuonR4::SpacePointContainer* spContainer{nullptr};
      ATH_CHECK(SG::get(spContainer, spKey, ctx));

      for (const MuonR4::SpacePointBucket* bucket : *spContainer) {
        const auto it = segmentsPerBucket.find(bucket);
        const SegmentList* matchedSegments{nullptr};
        if (it != segmentsPerBucket.end() && !it->second.empty()) {
          matchedSegments = &it->second;
        } else {
          const std::string sig = bucketSignatureKey(*bucket);
          const auto sigIt = sig.empty() ? segmentsPerBucketSignature.end()
                                         : segmentsPerBucketSignature.find(sig);
          if (sigIt != segmentsPerBucketSignature.end() && !sigIt->second.empty()) {
            matchedSegments = &sigIt->second;
            ++nSignatureMatchedBuckets;
          }
        }
        if (!matchedSegments) continue;

        const int bucketSector = bucket->msSector() ? static_cast<int>(bucket->msSector()->sector()) : -1;
        const uint16_t bucketLayers = countLayersInBucket(*bucket);
        ATH_MSG_VERBOSE("DV bucket segment node source: key=" << spKey.key()
                        << " sector=" << bucketSector
                        << " layers=" << bucketLayers
                        << " segments=" << matchedSegments->size());

        for (const xAOD::MuonSegment* seg : *matchedSegments) {
          appendMuonSegmentNode(*seg, bucketSector, nodes);
        }
      }
    }

    ATH_MSG_DEBUG("DV graph built " << nodes.size()
                  << " muon nodes from BucketDumper-style SpacePointBucket-associated segments"
                  << " (signature-matched filtered buckets=" << nSignatureMatchedBuckets << ")");
  }

  if (segments && nodes.empty() &&
      (!m_useBucketSegmentSelection.value() || m_fallbackToAllSegments.value())) {
    if (m_useBucketSegmentSelection.value()) {
      ATH_MSG_WARNING("No bucket-associated segments were found for DV graph building; "
                      "falling back to all segments from " << m_segmentKey.key()
                      << ". This does not match the training converter exactly.");
    }
    for (const xAOD::MuonSegment* seg : *segments) {
      appendMuonSegmentNode(*seg, static_cast<int>(seg->sector()), nodes);
    }
  }

  if (segments && nodes.empty() && m_useBucketSegmentSelection.value() &&
      !m_fallbackToAllSegments.value()) {
    ATH_MSG_WARNING("No bucket-associated segments were found for DV graph building. "
                    "Not falling back to all segments because that does not match the training converter.");
  }

  const std::size_t nMuonNodes = nodes.size();

  if (!m_towerKey.empty() && nMuonNodes > 0u) {
    const CaloTowerContainer* towers{nullptr};
    ATH_CHECK(SG::get(towers, m_towerKey, ctx));

    nodes.reserve(nodes.size() + towers->size());
    for (const CaloTower* tower : *towers) {
      const float energyMeV = static_cast<float>(tower->energy());
      if (energyMeV < m_minTowerEnergyMeV) continue;

      const float eta = static_cast<float>(tower->eta());
      const float phi = static_cast<float>(tower->phi());
      float minDR = std::numeric_limits<float>::max();
      for (std::size_t i = 0; i < nMuonNodes; ++i) {
        minDR = std::min(
            minDR,
            static_cast<float>(xAOD::P4Helpers::deltaR(eta, phi, nodes[i].eta, nodes[i].phi)));
      }

      if (minDR >= m_maxTowerSegmentDR) continue;
      const Amg::Vector3D direction = Acts::makeDirectionFromPhiEta(
          static_cast<double>(phi), static_cast<double>(eta));
      const std::optional<Amg::Vector3D> posMm =
          firstIntersectionWithEnvelope(direction, m_caloRMaxMm.value(), m_caloZMaxMm.value());
      if (!posMm) continue;

      const Amg::Vector3D posM = (*posMm) / Gaudi::Units::m;

      DVNodeAux node{};
      node.kind = NodeKind::Calo;
      node.features[0] = static_cast<float>(posM.mag());
      node.features[1] = static_cast<float>(posM.theta());
      node.features[2] = static_cast<float>(posM.phi());
      node.features[3] = static_cast<float>(direction.theta());
      node.features[4] = static_cast<float>(direction.phi());
      node.features[5] = energyMeV;
      node.features[6] = static_cast<float>(tower->size());
      node.eta = eta;
      node.phi = phi;
      node.energyLike = energyMeV;
      node.direction = direction;
      node.sector = static_cast<int>(
          MuonR4::ExpandedSector{CxxUtils::wrapToPi(static_cast<double>(phi))}.msSector());
      nodes.push_back(node);
    }
  }

  const std::size_t nCaloNodes = nodes.size() - nMuonNodes;
  const std::size_t nNodes = nodes.size();
  graphData.spacePointsInBucket.push_back(static_cast<int64_t>(nMuonNodes));
  graphData.spacePointsInBucket.push_back(static_cast<int64_t>(nCaloNodes));

  if (nNodes == 0u) {
    ATH_MSG_WARNING("No muon segment or calo tower nodes found. Skipping DV inference.");
    return StatusCode::SUCCESS;
  }

  graphData.featureLeaves.reserve(nNodes * kNodeFeatureCount);
  for (const DVNodeAux& node : nodes) {
    graphData.featureLeaves.insert(graphData.featureLeaves.end(),
                                   node.features.begin(), node.features.end());
  }

  const int maxEdges = m_maxEdges.value();
  std::vector<float> edgeAttr;
  edgeAttr.reserve(2u * nMuonNodes * std::max<std::size_t>(nCaloNodes, 1u) * kEdgeFeatureCount);

  auto addEdge = [&graphData, &edgeAttr, maxEdges](std::size_t src,
						   std::size_t dst,
						   const DVNodeAux& a,
						   const DVNodeAux& b) -> bool {
    if (maxEdges >= 0 && static_cast<int>(graphData.srcEdges.size()) >= maxEdges) return false;
    const float dPhi = CxxUtils::deltaPhi(b.phi, a.phi);
    const float dEta = b.eta - a.eta;
    const float cosAng = std::clamp(static_cast<float>(a.direction.dot(b.direction)), -1.f, 1.f);
    std::array<float, kEdgeFeatureCount> attr{
        b.energyLike - a.energyLike,
        dPhi,
        dEta,
        cosAng,
        (a.sector == b.sector) ? 1.f : 0.f};

    graphData.srcEdges.push_back(static_cast<int64_t>(src));
    graphData.desEdges.push_back(static_cast<int64_t>(dst));
    edgeAttr.insert(edgeAttr.end(), attr.begin(), attr.end());
    return true;
  };

  bool edgeCapReached = false;
  for (std::size_t im = 0; im < nMuonNodes && !edgeCapReached; ++im) {
    for (std::size_t ic = nMuonNodes; ic < nNodes; ++ic) {
      if (xAOD::P4Helpers::deltaR(nodes[im].eta, nodes[im].phi,
                                    nodes[ic].eta, nodes[ic].phi) >= m_maxTowerSegmentDR.value()) continue;
      if (!addEdge(im, ic, nodes[im], nodes[ic])) {
        edgeCapReached = true;
        break;
      }
      if (!addEdge(ic, im, nodes[ic], nodes[im])) {
        edgeCapReached = true;
        break;
      }
    }
  }

  const std::size_t nEdges = graphData.srcEdges.size();
  if (m_requireEdges.value() && nEdges == 0u) {
    ATH_MSG_DEBUG("DV graph has no segment-tower edges and RequireEdges=True; skip inference.");
    graphData.graph.reset();
    return StatusCode::SUCCESS;
  }

  if (edgeAttr.size() != nEdges * kEdgeFeatureCount) {
    ATH_MSG_ERROR("DV edge attribute size mismatch: E=" << nEdges
                  << " edge_attr.size=" << edgeAttr.size());
    return StatusCode::FAILURE;
  }

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  std::vector<int64_t> nodeShape{static_cast<int64_t>(nNodes), static_cast<int64_t>(kNodeFeatureCount)};
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      graphData.featureLeaves.data(),
                                      graphData.featureLeaves.size(),
                                      nodeShape.data(),
                                      nodeShape.size()));

  graphData.edgeIndexPacked.clear();
  graphData.edgeIndexPacked.reserve(2u * nEdges);
  graphData.edgeIndexPacked.insert(graphData.edgeIndexPacked.end(), graphData.srcEdges.begin(), graphData.srcEdges.end());
  graphData.edgeIndexPacked.insert(graphData.edgeIndexPacked.end(), graphData.desEdges.begin(), graphData.desEdges.end());

  std::vector<int64_t> edgeIndexShape{2, static_cast<int64_t>(nEdges)};
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<int64_t>(memInfo,
                                        graphData.edgeIndexPacked.data(),
                                        graphData.edgeIndexPacked.size(),
                                        edgeIndexShape.data(),
                                        edgeIndexShape.size()));

  Ort::AllocatorWithDefaultOptions allocator;
  std::vector<int64_t> edgeAttrShape{static_cast<int64_t>(nEdges), static_cast<int64_t>(kEdgeFeatureCount)};
  Ort::Value edgeAttrTensor = Ort::Value::CreateTensor(allocator,
                                                       edgeAttrShape.data(),
                                                       edgeAttrShape.size(),
                                                       ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT);
  if (!edgeAttr.empty()) {
    float* edgeAttrData = edgeAttrTensor.GetTensorMutableData<float>();
    std::copy(edgeAttr.begin(), edgeAttr.end(), edgeAttrData);
  }
  graphData.graph->dataTensor.emplace_back(std::move(edgeAttrTensor));

  std::vector<int64_t> nMuonShape{1};
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<int64_t>(memInfo,
                                        graphData.spacePointsInBucket.data(),
                                        1,
                                        nMuonShape.data(),
                                        nMuonShape.size()));

  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG("Built DV graph: N=" << nNodes << " (muon=" << nMuonNodes
                  << ", calo=" << nCaloNodes << "), E=" << nEdges
                  << ", n_muon_nodes=" << graphData.spacePointsInBucket[0]);
    const std::size_t dumpNodes = std::min<std::size_t>(m_debugDumpFirstNNodes.value(), nNodes);
    for (std::size_t i = 0; i < dumpNodes; ++i) {
      std::ostringstream row;
      row << "DVNode[" << i << "] kind=" << (nodes[i].kind == NodeKind::Muon ? "muon" : "calo") << ":";
      for (std::size_t f = 0; f < kNodeFeatureCount; ++f) {
        row << " f" << f << "=" << graphData.featureLeaves[i * kNodeFeatureCount + f];
      }
      ATH_MSG_DEBUG(row.str());
    }
    const std::size_t dumpEdges = std::min<std::size_t>(m_debugDumpFirstNEdges.value(), nEdges);
    for (std::size_t e = 0; e < dumpEdges; ++e) {
      ATH_MSG_DEBUG("DVEdge[" << e << "]: " << graphData.srcEdges[e]
                    << " -> " << graphData.desEdges[e]
                    << " edge_attr=["
                    << edgeAttr[e * kEdgeFeatureCount + 0] << ", "
                    << edgeAttr[e * kEdgeFeatureCount + 1] << ", "
                    << edgeAttr[e * kEdgeFeatureCount + 2] << ", "
                    << edgeAttr[e * kEdgeFeatureCount + 3] << ", "
                    << edgeAttr[e * kEdgeFeatureCount + 4] << "]");
    }
  }

  graphData.srcEdges.clear();
  graphData.desEdges.clear();
  return StatusCode::SUCCESS;
}

StatusCode DVInferenceToolBase::runNamedInference(
    GraphRawData& graphData,
    const std::vector<InputTensorSpec>& inputSpecs,
    const std::vector<std::string>& outputNames) const {
  if (!graphData.graph) {
    ATH_MSG_ERROR("Graph data is not built.");
    return StatusCode::FAILURE;
  }
  if (inputSpecs.empty()) {
    ATH_MSG_ERROR("No DV ONNX inputs were selected for inference.");
    return StatusCode::FAILURE;
  }

  for (const InputTensorSpec& spec : inputSpecs) {
    if (spec.tensorIndex >= graphData.graph->dataTensor.size()) {
      ATH_MSG_ERROR("Input " << spec.name << " requests tensor index " << spec.tensorIndex
                    << " but only " << graphData.graph->dataTensor.size()
                    << " tensors were prepared.");
      return StatusCode::FAILURE;
    }
  }

  std::vector<const char*> inputNamePtrs{};
  inputNamePtrs.reserve(inputSpecs.size());
  for (const InputTensorSpec& spec : inputSpecs) {
    inputNamePtrs.push_back(spec.name.c_str());
  }

  std::vector<const char*> outputNamePtrs{};
  outputNamePtrs.reserve(outputNames.size());
  for (const std::string& name : outputNames) {
    outputNamePtrs.push_back(name.c_str());
  }

  graphData.graph->dataTensor.reserve(graphData.graph->dataTensor.size() + outputNamePtrs.size()); 

  Ort::RunOptions runOptions;
  runOptions.SetRunLogSeverityLevel(ORT_LOGGING_LEVEL_ERROR);

  if (m_isCuda) {
    Ort::IoBinding binding(model());
    for (const InputTensorSpec& spec : inputSpecs) {
      binding.BindInput(spec.name.c_str(), graphData.graph->dataTensor[spec.tensorIndex]);
    }
    Ort::MemoryInfo cpuOut = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    for (const char* outName : outputNamePtrs) {
      binding.BindOutput(outName, cpuOut);
    }

    model().Run(runOptions, binding);
    binding.SynchronizeOutputs();

    std::vector<Ort::Value> outputs = binding.GetOutputValues();
    if (outputs.empty()) {
      ATH_MSG_ERROR("IoBinding inference returned empty output.");
      return StatusCode::FAILURE;
    }

    if (m_sanitizeNonFinitePredictions.value()) {
      float* outData = outputs[0].GetTensorMutableData<float>();
      const std::size_t outSize = outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
      for (std::size_t i = 0; i < outSize; ++i) {
        if (!std::isfinite(outData[i])) {
          ATH_MSG_WARNING("Non-finite DV prediction detected at " << i << " -> set to -100.");
          outData[i] = -100.f;
        }
      }
    }

    for (auto& v : outputs) {
      graphData.graph->dataTensor.emplace_back(std::move(v));
    }
    return StatusCode::SUCCESS;
  }

  std::vector<Ort::Value> orderedInputs{};
  orderedInputs.reserve(inputSpecs.size());
  for (const InputTensorSpec& spec : inputSpecs) {
    orderedInputs.emplace_back(std::move(graphData.graph->dataTensor[spec.tensorIndex]));
  }

  std::vector<Ort::Value> outputs =
      model().Run(runOptions,
                  inputNamePtrs.data(),
                  orderedInputs.data(),
                  inputNamePtrs.size(),
                  outputNamePtrs.data(),
                  outputNamePtrs.size());

  if (outputs.empty()) {
    ATH_MSG_ERROR("Inference returned empty output.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("DV ONNX raw output elementCount = "
                << outputs[0].GetTensorTypeAndShapeInfo().GetElementCount());

  if (m_sanitizeNonFinitePredictions.value()) {
    float* outData = outputs[0].GetTensorMutableData<float>();
    const std::size_t outSize = outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
    for (std::size_t i = 0; i < outSize; ++i) {
      if (!std::isfinite(outData[i])) {
        ATH_MSG_WARNING("Non-finite DV prediction detected at " << i << " -> set to -100.");
        outData[i] = -100.f;
      }
    }
  }

  for (auto& v : outputs) {
    graphData.graph->dataTensor.emplace_back(std::move(v));
  }
  return StatusCode::SUCCESS;
}

StatusCode DVInferenceToolBase::runInference(GraphRawData& graphData) const {
  const std::vector<std::string> availableInputs = modelInputNames();
  if (availableInputs.empty()) {
    ATH_MSG_ERROR("DV ONNX model has no inputs.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("DV ONNX model inputs: " << joinNames(availableInputs));

  const std::string nodeName = m_inputNodeName.value();
  const std::string edgeIndexName = m_inputEdgeIndexName.value();
  const std::string edgeAttrName = m_inputEdgeAttrName.value();
  const std::string nMuonNodesName = m_inputNMuonNodesName.value();

  std::vector<InputTensorSpec> inputSpecs{};
  inputSpecs.reserve(kInputTensorCount);

  auto addIfPresent = [&availableInputs, &inputSpecs](const std::string& name, std::size_t tensorIndex) {
    if (hasName(availableInputs, name)) {
      inputSpecs.push_back(InputTensorSpec{name, tensorIndex});
      return true;
    }
    return false;
  };

  const bool hasNodeInput = addIfPresent(nodeName, 0u);
  const bool hasEdgeIndexInput = addIfPresent(edgeIndexName, 1u);
  const bool hasEdgeAttrInput = addIfPresent(edgeAttrName, 2u);
  const bool hasNMuonNodesInput = addIfPresent(nMuonNodesName, 3u);

  if (!hasNodeInput || !hasEdgeIndexInput) {
    ATH_MSG_ERROR("DV ONNX model is missing required inputs. Expected at least "
                  << nodeName << " and " << edgeIndexName
                  << "; model inputs are: " << joinNames(availableInputs));
    return StatusCode::FAILURE;
  }

  if (!hasEdgeAttrInput) {
    ATH_MSG_DEBUG("DV ONNX model has no input named " << edgeAttrName
                 << "; not binding edge_attr. This is expected for exports where "
                 << "the architecture does not consume edge attributes and ONNX pruned the input.");
  }
  if (!hasNMuonNodesInput) {
    ATH_MSG_DEBUG("DV ONNX model has no input named " << nMuonNodesName
                 << "; not binding n_muon_nodes. This is expected only if the exported "
                 << "model does not need model-side muon/calo normalization.");
  }

  for (const std::string& inputName : availableInputs) {
    if (inputName != nodeName && inputName != edgeIndexName &&
        inputName != edgeAttrName && inputName != nMuonNodesName) {
      ATH_MSG_ERROR("DV ONNX model has unsupported input " << inputName
                    << ". Configure the input-name properties or update the tool mapping.");
      return StatusCode::FAILURE;
    }
  }

  std::vector<std::string> outputNames{};
  const std::vector<std::string> availableOutputs = modelOutputNames();
  if (hasName(availableOutputs, m_outputName.value())) {
    outputNames.push_back(m_outputName.value());
  } else if (!availableOutputs.empty()) {
    ATH_MSG_WARNING("DV ONNX model has no output named " << m_outputName.value()
                    << "; using first model output " << availableOutputs.front() << ".");
    outputNames.push_back(availableOutputs.front());
  } else {
    ATH_MSG_ERROR("DV ONNX model has no outputs.");
    return StatusCode::FAILURE;
  }

  return runNamedInference(graphData, inputSpecs, outputNames);
}

float DVInferenceToolBase::probabilityFromOutput(const Ort::Value& output, float& rawOutput) const {
  rawOutput = 0.f;
  const float* data = output.GetTensorData<float>();
  const auto shapeInfo = output.GetTensorTypeAndShapeInfo();
  const std::size_t nElem = shapeInfo.GetElementCount();
  if (nElem == 0 || data == nullptr) return std::numeric_limits<float>::quiet_NaN();

  if (nElem == 1) {
    rawOutput = data[0];
    if (m_singleOutputMode.value() == "prob") return rawOutput;
    if (m_singleOutputMode.value() == "logit" || m_singleOutputMode.value() == "auto") {
      return InferenceUtils::sigmoid(rawOutput);
    }
    return InferenceUtils::sigmoid(rawOutput);
  }

  if (nElem == 2) {
    rawOutput = data[1];
    const float z0 = data[0] - std::max(data[0], data[1]);
    const float z1 = data[1] - std::max(data[0], data[1]);
    const float e0 = std::exp(z0);
    const float e1 = std::exp(z1);
    return e1 / (e0 + e1);
  }

  ATH_MSG_WARNING("DV output tensor has " << nElem
                  << " elements; using element 0 with SingleOutputMode=" << m_singleOutputMode.value());
  rawOutput = data[0];
  if (m_singleOutputMode.value() == "prob") return rawOutput;
  if (m_singleOutputMode.value() == "logit" || m_singleOutputMode.value() == "auto") {
    return InferenceUtils::sigmoid(rawOutput);
  }
  return InferenceUtils::sigmoid(rawOutput);
}
