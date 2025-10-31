/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#include "BucketInferenceToolBase.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandle.h"

#include "BucketGraphUtils.h"
#include "MuonSpacePoint/SpacePointContainer.h"

#include <algorithm>
#include <limits>
#include <span>

using namespace MuonML;

Ort::Session& BucketInferenceToolBase::model() const {
  return m_onnxSessionTool->session();
}

StatusCode BucketInferenceToolBase::setupModel() {
  ATH_CHECK(m_onnxSessionTool.retrieve());
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_geoCtxKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::buildFeaturesOnly(const EventContext& ctx,
                                                      GraphRawData& graphData) const {
  graphData.graph = std::make_unique<InferenceGraph>();
  graphData.srcEdges.clear();
  graphData.desEdges.clear();
  graphData.featureLeaves.clear();
  graphData.spacePointsInBucket.clear();

  const MuonR4::SpacePointContainer* buckets{nullptr};
  ATH_CHECK(SG::get(buckets, m_readKey, ctx));

  const ActsTrk::GeometryContext* gctx = nullptr;
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  std::vector<BucketGraphUtils::NodeAux> nodes;
  BucketGraphUtils::buildNodesAndFeatures(*buckets, *gctx, nodes,
                                          graphData.featureLeaves,
                                          graphData.spacePointsInBucket); // now int64_t-compatible

  const int64_t numNodes = static_cast<int64_t>(nodes.size());
  ATH_MSG_DEBUG("Total buckets: " << buckets->size()
                 << " -> nodes (size>0): " << numNodes
                 << " | features.size()=" << graphData.featureLeaves.size());

  if (numNodes == 0) {
    ATH_MSG_WARNING("No valid buckets found (all have size 0.0). Skipping inference.");
    return StatusCode::SUCCESS;
  }

  const int64_t nFeatPerNode = 6;
  if (numNodes * nFeatPerNode != static_cast<int64_t>(graphData.featureLeaves.size())) {
    ATH_MSG_ERROR( "Feature size mismatch: expected " << (numNodes * nFeatPerNode)
                   << " got " << graphData.featureLeaves.size());
    return StatusCode::FAILURE;
  }

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  std::vector<int64_t> featShape{numNodes, nFeatPerNode};
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      graphData.featureLeaves.data(),
                                      graphData.featureLeaves.size(),
                                      featShape.data(),
                                      featShape.size()));
  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::buildTransformerInputs(const EventContext& ctx,
                                                           GraphRawData& graphData) const {
  // Start from (N,6)
  ATH_CHECK(buildFeaturesOnly(ctx, graphData));

  // Copy features flat buffer for lifetime management
  std::vector<float> featuresFlat = graphData.featureLeaves;
  const int64_t S = static_cast<int64_t>(featuresFlat.size() / 6);

  if (S == 0) {
    ATH_MSG_WARNING("No valid features for transformer input. Skipping inference.");
    return StatusCode::SUCCESS;
  }

  if (msgLvl(MSG::DEBUG)) {
    // DEBUG: Print transformer input features for first 10 nodes
    ATH_MSG_DEBUG("=== DEBUGGING: Transformer input features for first 10 nodes ===");
    const int64_t debugNodes = std::min(S, static_cast<int64_t>(10));
    for (int64_t nodeIdx = 0; nodeIdx < debugNodes; ++nodeIdx) {
      const int64_t baseIdx = nodeIdx * 6;
      ATH_MSG_DEBUG("TransformerNode[" << nodeIdx << "]: "
                   << "x=" << featuresFlat[baseIdx + 0] << ", "
                   << "y=" << featuresFlat[baseIdx + 1] << ", "
                   << "z=" << featuresFlat[baseIdx + 2] << ", "
                   << "layers=" << featuresFlat[baseIdx + 3] << ", "
                   << "nSp=" << featuresFlat[baseIdx + 4] << ", "
                   << "bucketSize=" << featuresFlat[baseIdx + 5]);
    }
    ATH_MSG_DEBUG("=== END DEBUG TRANSFORMER FEATURES ===");
  }

  // Rebuild graph with exactly 2 inputs: features [1,S,6], pad_mask [1,S]
  graphData.graph = std::make_unique<InferenceGraph>();

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // features: [1,S,6] (backed by graphData.featureLeaves to keep alive)
  std::vector<int64_t> fShape{1, S, 6};
  graphData.featureLeaves.swap(featuresFlat);
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<float>(memInfo,
                                      graphData.featureLeaves.data(),
                                      graphData.featureLeaves.size(),
                                      fShape.data(),
                                      fShape.size()));

  // pad_mask: [1,S] (bool). Create ORT-owned tensor and fill with False (=valid).
  Ort::AllocatorWithDefaultOptions allocator;
  std::vector<int64_t> mShape{1, S};
  Ort::Value padVal = Ort::Value::CreateTensor(allocator,
                                               mShape.data(),
                                               mShape.size(),
                                               ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL);
  bool* maskPtr = padVal.GetTensorMutableData<bool>();
  for (int64_t i = 0; i < S; ++i) maskPtr[i] = false;
  graphData.graph->dataTensor.emplace_back(std::move(padVal));

  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::buildGraph(const EventContext& ctx,
                                               GraphRawData& graphData) const {
  ATH_CHECK(buildFeaturesOnly(ctx, graphData));

  const MuonR4::SpacePointContainer* buckets{nullptr};
  ATH_CHECK(SG::get(buckets, m_readKey, ctx));

  const ActsTrk::GeometryContext* gctx = nullptr;
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  std::vector<BucketGraphUtils::NodeAux> nodes;
  std::vector<float> throwawayFeatures;
  std::vector<int64_t> throwawaySp;  // int64_t
  BucketGraphUtils::buildNodesAndFeatures(*buckets, *gctx, nodes, throwawayFeatures, throwawaySp);

  const int64_t numNodes = static_cast<int64_t>(nodes.size());
  if (numNodes == 0) {
    ATH_MSG_WARNING("No valid buckets found (all have size 0.0). Skipping graph building.");
    return StatusCode::SUCCESS;
  }

  std::vector<int64_t> srcEdges, dstEdges;
  BucketGraphUtils::buildSparseEdges(nodes,
                                     m_minLayers,
                                     m_maxChamberDelta,
                                     m_maxSectorDelta,
                                     m_maxDistXY,
                                     m_maxAbsDz,
                                     srcEdges, dstEdges);
  if (m_validateEdges) {
    size_t bad = 0;
    std::vector<int64_t> newSrc;
    std::vector<int64_t> newDst;
    newSrc.reserve(srcEdges.size());
    newDst.reserve(dstEdges.size());
    for (size_t k = 0; k < srcEdges.size(); ++k) {
      const int64_t u = srcEdges[k];
      const int64_t v = dstEdges[k];
      const bool okU = (u >= 0 && u < numNodes);
      const bool okV = (v >= 0 && v < numNodes);
      if (okU && okV) {
        newSrc.push_back(u);
        newDst.push_back(v);
      } else {
        ++bad;
        ATH_MSG_DEBUG( "Drop invalid edge " << k << ": (" << u << "->" << v
                        << "), valid node range [0," << (numNodes-1) << "]");
      }
    }
    if (bad) {
      ATH_MSG_WARNING( "Removed " << bad << " invalid edges out of "
                        << srcEdges.size());
      srcEdges.swap(newSrc);
      dstEdges.swap(newDst);
    }
  }

  const size_t E = srcEdges.size();

  if (msgLvl(MSG::DEBUG)) {
    // DEBUG: Count connections per node
    ATH_MSG_DEBUG("Edges built: " << E);
    const unsigned int dumpE = std::min<unsigned int>(m_debugDumpFirstNEdges, E);
    for (unsigned int k = 0; k < dumpE; ++k) {
      ATH_MSG_DEBUG("EDGE[" << k << "]: " << srcEdges[k] << " -> " << dstEdges[k]);
    }
    std::vector<int> nodeConnections(numNodes, 0);
    for (size_t k = 0; k < srcEdges.size(); ++k) {
      const int64_t u = srcEdges[k];
      const int64_t v = dstEdges[k];
      if (u >= 0 && u < numNodes) nodeConnections[u]++;
      if (v >= 0 && v < numNodes) nodeConnections[v]++;
    }

    ATH_MSG_INFO("=== DEBUGGING: Node Connections (first 10 nodes) ===");
    const int64_t debugNodeCount = std::min(numNodes, static_cast<int64_t>(10));
    for (int64_t i = 0; i < debugNodeCount; ++i) {
      ATH_MSG_DEBUG("Node[" << i << "] connections: " << nodeConnections[i]);
    }
    ATH_MSG_DEBUG("=== END DEBUG NODE CONNECTIONS ===");

    // DEBUG: Show detailed edge connections for first 10 nodes
    ATH_MSG_DEBUG("=== DEBUGGING: Detailed Edge Connections (first 10 nodes) ===");
    for (int64_t nodeIdx = 0; nodeIdx < debugNodeCount; ++nodeIdx) {
      std::stringstream connections;
      connections << "Node[" << nodeIdx << "] connected to: ";
      bool foundAny = false;

      for (size_t k = 0; k < srcEdges.size(); ++k) {
        const int64_t u = srcEdges[k];
        const int64_t v = dstEdges[k];

        if (u == nodeIdx) {
          if (foundAny) connections << ", ";
          connections << v;
          foundAny = true;
        } else if (v == nodeIdx) {
          if (foundAny) connections << ", ";
          connections << u;
          foundAny = true;
        }
      }

      if (!foundAny) connections << "none";
      ATH_MSG_DEBUG(connections.str());
    }
    ATH_MSG_DEBUG("=== END DEBUG DETAILED CONNECTIONS ===");
  }

  graphData.edgeIndexPacked.clear();
  const size_t Efinal = BucketGraphUtils::packEdgeIndex(srcEdges, dstEdges, graphData.edgeIndexPacked);

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  std::vector<int64_t> edgeShape{2, static_cast<int64_t>(Efinal)};
  graphData.graph->dataTensor.emplace_back(
      Ort::Value::CreateTensor<int64_t>(memInfo,
                                        graphData.edgeIndexPacked.data(),
                                        graphData.edgeIndexPacked.size(),
                                        edgeShape.data(),
                                        edgeShape.size()));

  ATH_MSG_DEBUG("Built sparse bucket graph: N=" << numNodes << ", E=" << Efinal);
  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::runNamedInference(
    GraphRawData& graphData,
    const std::vector<const char*>& inputNames,
    const std::vector<const char*>& outputNames) const
{
  if (!graphData.graph) {
    ATH_MSG_ERROR("Graph data is not built.");
    return StatusCode::FAILURE;
  }
  if (graphData.graph->dataTensor.empty()) {
    ATH_MSG_ERROR("No input tensors prepared for inference.");
    return StatusCode::FAILURE;
  }

  if (msgLvl(MSG::DEBUG)) {
    // DEBUG: Print actual input tensor data for features tensor

    ATH_MSG_DEBUG("=== DEBUGGING: ONNX Input tensor data ===");
    if (!graphData.graph->dataTensor.empty()) {
      const auto& featureTensor = graphData.graph->dataTensor[0];
      auto featShape = featureTensor.GetTensorTypeAndShapeInfo().GetShape();
      ATH_MSG_DEBUG("Features tensor shape: [" << featShape[0] 
                   << (featShape.size()>1 ? ("," + std::to_string(featShape[1])) : "")
                   << (featShape.size()>2 ? ("," + std::to_string(featShape[2])) : "") << "]");
      
      float* featData = const_cast<Ort::Value&>(featureTensor).GetTensorMutableData<float>();
      const size_t totalElements = featureTensor.GetTensorTypeAndShapeInfo().GetElementCount();
      ATH_MSG_DEBUG("Features tensor total elements: " << totalElements);
      
      // Print first 10 nodes (60 values, 6 per node)
      const size_t debugElements = std::min(totalElements, static_cast<size_t>(60));
      for (size_t i = 0; i < debugElements; i += 6) {
        if (i + 5 < totalElements) {
          ATH_MSG_DEBUG("ONNXNode[" << (i/6) << "]: "
                       << "x=" << featData[i+0] << ", "
                       << "y=" << featData[i+1] << ", "
                       << "z=" << featData[i+2] << ", "
                       << "layers=" << featData[i+3] << ", "
                       << "nSp=" << featData[i+4] << ", "
                       << "bucketSize=" << featData[i+5]);
        }
      }
    }
    ATH_MSG_DEBUG("=== END DEBUG ONNX INPUT ===");
  }

  Ort::RunOptions run_options;
  run_options.SetRunLogSeverityLevel(ORT_LOGGING_LEVEL_WARNING);

  std::vector<Ort::Value> outputs =
      model().Run(run_options,
                  inputNames.data(),
                  graphData.graph->dataTensor.data(),
                  graphData.graph->dataTensor.size(),
                  outputNames.data(),
                  outputNames.size());

  if (outputs.empty()) {
    ATH_MSG_ERROR("Inference returned empty output.");
    return StatusCode::FAILURE;
  }

  float* outData = outputs[0].GetTensorMutableData<float>();
  const size_t outSize = outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
  ATH_MSG_DEBUG("ONNX raw output elementCount = " << outSize);

  std::span<float> preds(outData, outData + outSize);
  for (size_t i = 0; i < outSize; ++i) {
    if (!std::isfinite(preds[i])) {
      ATH_MSG_WARNING("Non-finite prediction detected at " << i << " -> set to -100.");
      preds[i] = -100.0f;
    }
  }

  for (auto& v : outputs) {
    graphData.graph->dataTensor.emplace_back(std::move(v));
  }
  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::runInference(GraphRawData& graphData) const {
  std::vector<const char*> inputNames  = {"features", "edge_index"};
  std::vector<const char*> outputNames = {"output"};
  return runNamedInference(graphData, inputNames, outputNames);
}