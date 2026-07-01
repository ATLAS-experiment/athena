/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#include "BucketInferenceToolBase.h"

#include "InferenceUtils.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadCondHandle.h"

#include "BucketGraphUtils.h"
#include "MuonSpacePoint/SpacePointContainer.h"

#include <array>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <span>

using namespace MuonML;

std::string BucketInferenceToolBase::trimFeatureToken(std::string s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

std::vector<std::string> BucketInferenceToolBase::parseFeatureNames(const std::string& raw) {
  std::vector<std::string> out;
  const std::string s = trimFeatureToken(raw);
  if (s.empty()) return out;

  // Preferred exporter format: JSON list of strings.
  if (!s.empty() && s.front() == '[') {
    bool inQuote = false;
    std::string token;
    for (char c : s) {
      if (c == '"') {
        if (inQuote) {
          if (!token.empty()) out.push_back(token);
          token.clear();
        }
        inQuote = !inQuote;
        continue;
      }
      if (inQuote) token.push_back(c);
    }
    if (!out.empty()) return out;
  }

  // Backward-compatible format: comma-separated.
  std::istringstream ss(s);
  std::string tok;
  while (std::getline(ss, tok, ',')) {
    tok = trimFeatureToken(tok);
    if (!tok.empty()) out.push_back(tok);
  }
  return out;
}

Ort::Session& BucketInferenceToolBase::model() const {
  return m_onnxSessionTool->session();
}

StatusCode BucketInferenceToolBase::setupModel() {
  ATH_CHECK(m_onnxSessionTool.retrieve());
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_geoCtxKey.initialize());

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

StatusCode BucketInferenceToolBase::buildFeaturesOnly(const EventContext& ctx,
                                                      GraphRawData& graphData) const {
  
  graphData.graph.reset();
  graphData.srcEdges.clear();
  graphData.desEdges.clear();
  graphData.edgeIndexPacked.clear();
  graphData.featureLeaves.clear();
  graphData.spacePointsInBucket.clear();
  graphData.graph = std::make_unique<InferenceGraph>();
  graphData.graph->dataTensor.reserve(1); // features input; outputs are reserved in runNamedInference()

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

  const int64_t nFeatPerNode = static_cast<int64_t>(kBucketFeatureCount);
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
  const int64_t S = static_cast<int64_t>(featuresFlat.size() / kBucketFeatureCount);

  if (S == 0) {
    ATH_MSG_WARNING("No valid features for transformer input. Skipping inference.");
    return StatusCode::SUCCESS;
  }

  if (msgLvl(MSG::DEBUG)) {
    // DEBUG: Print transformer input features for first 10 nodes
    ATH_MSG_DEBUG("=== DEBUGGING: Transformer input features for first 10 nodes ===");
    const int64_t debugNodes = std::min(S, static_cast<int64_t>(10));
    for (int64_t nodeIdx = 0; nodeIdx < debugNodes; ++nodeIdx) {
      const int64_t baseIdx = nodeIdx * static_cast<int64_t>(kBucketFeatureCount);
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
  graphData.graph.reset();
  graphData.graph = std::make_unique<InferenceGraph>();
  graphData.graph->dataTensor.reserve(2); // features and pad_mask inputs; outputs are reserved in runNamedInference()

  Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

  // features: [1,S,6] (backed by graphData.featureLeaves to keep alive)
  std::vector<int64_t> fShape{1, S, static_cast<int64_t>(kBucketFeatureCount)};
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
  
  graphData.graph.reset();
  graphData.srcEdges.clear();
  graphData.desEdges.clear();
  graphData.featureLeaves.clear();
  graphData.spacePointsInBucket.clear();
  graphData.edgeIndexPacked.clear();
  graphData.graph = std::make_unique<InferenceGraph>();
  graphData.graph->dataTensor.reserve(2); // features and edge_index inputs; outputs are reserved in runNamedInference()

  const MuonR4::SpacePointContainer* buckets{nullptr};
  ATH_CHECK(SG::get(buckets, m_readKey, ctx));

  const ActsTrk::GeometryContext* gctx = nullptr;
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  std::vector<BucketGraphUtils::NodeAux> nodes;

  BucketGraphUtils::buildNodesAndFeatures(*buckets, *gctx, nodes,
                                          graphData.featureLeaves,
                                          graphData.spacePointsInBucket);

  const int64_t numNodes = static_cast<int64_t>(nodes.size());
  ATH_MSG_DEBUG("Total buckets: " << buckets->size()
                << " -> nodes (size>0): " << numNodes
                << " | features.size()=" << graphData.featureLeaves.size());

  if (numNodes == 0) {
    ATH_MSG_WARNING("No valid buckets found (all have size 0.0). Skipping graph building.");
    return StatusCode::SUCCESS;
  }

  const int64_t nFeatPerNode = static_cast<int64_t>(kBucketFeatureCount);
  if (numNodes * nFeatPerNode != static_cast<int64_t>(graphData.featureLeaves.size())) {
    ATH_MSG_ERROR("Feature size mismatch: expected " << (numNodes * nFeatPerNode)
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

  BucketGraphUtils::buildSparseEdges(nodes,
                                     m_minLayers,
                                     m_maxChamberDelta,
                                     m_maxSectorDelta,
                                     m_maxDistXY,
                                     m_maxAbsDz,
                                     graphData.srcEdges, graphData.desEdges);
  if (m_validateEdges) {
    size_t bad = 0;
    size_t write = 0;
    for (size_t k = 0; k < graphData.srcEdges.size(); ++k) {
      const int64_t u = graphData.srcEdges[k];
      const int64_t v = graphData.desEdges[k];
      const bool okU = (u >= 0 && u < numNodes);
      const bool okV = (v >= 0 && v < numNodes);
      if (okU && okV) {
        graphData.srcEdges[write] = u;
        graphData.desEdges[write] = v;
        ++write;
      } else {
        ++bad;
        ATH_MSG_DEBUG( "Drop invalid edge " << k << ": (" << u << "->" << v
                        << "), valid node range [0," << (numNodes-1) << "]");
      }
    }
    if (bad) {
      ATH_MSG_WARNING( "Removed " << bad << " invalid edges out of "
                        << graphData.srcEdges.size());
      graphData.srcEdges.resize(write);
      graphData.desEdges.resize(write);
    }
  }

  const size_t E = graphData.srcEdges.size();

  if (msgLvl(MSG::DEBUG)) {
    // DEBUG: Count connections per node
    ATH_MSG_DEBUG("Edges built: " << E);
    const size_t dumpE = std::min<std::size_t>(m_debugDumpFirstNEdges.value(), E);
    for (size_t k = 0; k < dumpE; ++k) {
      ATH_MSG_DEBUG("EDGE[" << k << "]: "
                    << graphData.srcEdges[k] << " -> "
                    << graphData.desEdges[k]);
    }

    std::vector<int> nodeConnections(numNodes, 0);
    for (size_t k = 0; k < graphData.srcEdges.size(); ++k) {
      const int64_t u = graphData.srcEdges[k];
      const int64_t v = graphData.desEdges[k];
      if (u >= 0 && u < numNodes) nodeConnections[u]++;
      if (v >= 0 && v < numNodes) nodeConnections[v]++;
    }

    ATH_MSG_DEBUG("=== DEBUGGING: Node Connections (first 10 nodes) ===");
    const int64_t debugNodeCount = std::min(numNodes, static_cast<int64_t>(10));
    for (int64_t i = 0; i < debugNodeCount; ++i) {
      ATH_MSG_DEBUG("Node[" << i << "] connections: " << nodeConnections[i]);
    }
    ATH_MSG_DEBUG("=== END DEBUG NODE CONNECTIONS ===");

    ATH_MSG_DEBUG("=== DEBUGGING: Detailed Edge Connections (first 10 nodes) ===");
    for (int64_t nodeIdx = 0; nodeIdx < debugNodeCount; ++nodeIdx) {
      std::stringstream connections;
      connections << "Node[" << nodeIdx << "] connected to: ";
      bool foundAny = false;

      for (size_t k = 0; k < graphData.srcEdges.size(); ++k) {
        const int64_t u = graphData.srcEdges[k];
        const int64_t v = graphData.desEdges[k];

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

  nodes = {};

  graphData.edgeIndexPacked.clear();
  const size_t Efinal = BucketGraphUtils::packEdgeIndex(graphData.srcEdges,
                                                       graphData.desEdges,
                                                       graphData.edgeIndexPacked);

  graphData.srcEdges.clear();
  graphData.desEdges.clear();

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

  // Reserve the final size here from the actual I/O lists instead 
  // of hard-coding assumptions in the graph builders.
  graphData.graph->dataTensor.reserve(inputNames.size() + outputNames.size());
  if (graphData.graph->dataTensor.size() < inputNames.size()) {
    ATH_MSG_ERROR("Prepared " << graphData.graph->dataTensor.size()
                  << " tensors but inference expects " << inputNames.size() << " inputs.");
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
      
      // Print up to 10 nodes; stride = nFeat from tensor shape
      const size_t nFeat = (featShape.size() > 1 && featShape[1] > 0) ? static_cast<size_t>(featShape[1]) : 1;
      const size_t nNodes = totalElements / nFeat;
      const size_t debugNodes = std::min(nNodes, static_cast<size_t>(10));

      // Try to read feature names from model custom metadata.
      // Prefer x_feature_names (current exporter), then fall back to legacy keys.
      std::vector<std::string> featNames;
      {
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::ModelMetadata meta = model().GetModelMetadata();
        auto keys = meta.GetCustomMetadataMapKeysAllocated(allocator);
        std::vector<std::string> keyNames;
        keyNames.reserve(keys.size());
        for (const auto& k : keys) keyNames.emplace_back(k.get());
        const std::array<std::string, 4> candidates{
            "x_feature_names", "node_feature_names", "feature_names", "input_feature_names"};
        for (const std::string& key : candidates) {
          if (std::find(keyNames.begin(), keyNames.end(), key) != keyNames.end()) {
            std::string val = meta.LookupCustomMetadataMapAllocated(key.c_str(), allocator).get();
            featNames = parseFeatureNames(val);
            break;
          }
        }
        if (featNames.empty()) {
          ATH_MSG_DEBUG("No usable feature-name metadata key found in model; using generic fN labels.");
        }
      }
      auto featLabel = [&](size_t f) -> std::string {
        if (f < featNames.size()) return featNames[f];
        return "f" + std::to_string(f);
      };

      // Print legend
      {
        std::ostringstream legend;
        legend << "Node feature legend (" << nFeat << " features):";
        for (size_t f = 0; f < nFeat; ++f) {
          legend << " f" << f << "=" << featLabel(f);
          if (f + 1 < nFeat) legend << ",";
        }
        ATH_MSG_DEBUG(legend.str());
      }

      for (size_t n = 0; n < debugNodes; ++n) {
        std::ostringstream row;
        row << "ONNXNode[" << n << "]:";
        for (size_t f = 0; f < nFeat; ++f) {
          row << " f" << f << "=" << featData[n * nFeat + f];
          if (f + 1 < nFeat) row << ",";
        }
        ATH_MSG_DEBUG(row.str());
      }
    }
    ATH_MSG_DEBUG("=== END DEBUG ONNX INPUT ===");
  }

  Ort::RunOptions run_options;
  run_options.SetRunLogSeverityLevel(ORT_LOGGING_LEVEL_ERROR);

  if (m_isCuda) {
    // ---- CUDA path: use IoBinding so tensors stay on device ----
    Ort::IoBinding binding(model());
    for (std::size_t i = 0; i < inputNames.size(); ++i) {
      binding.BindInput(inputNames[i], graphData.graph->dataTensor[i]);
    }
    // Bind outputs to CPU so predictions are directly readable after sync.
    Ort::MemoryInfo cpuOut = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    for (const char* outName : outputNames) {
      binding.BindOutput(outName, cpuOut);
    }

    model().Run(run_options, binding);
    binding.SynchronizeOutputs();

    std::vector<Ort::Value> outputs = binding.GetOutputValues();
    if (outputs.empty()) {
      ATH_MSG_ERROR("IoBinding inference returned empty output.");
      return StatusCode::FAILURE;
    }

    float* outData = outputs[0].GetTensorMutableData<float>();
    const size_t outSize = outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
    ATH_MSG_DEBUG("ONNX (IoBinding) raw output elementCount = " << outSize);

    if (m_sanitizeNonFinitePredictions.value()) {
      std::span<float> preds(outData, outData + outSize);
      for (size_t i = 0; i < outSize; ++i) {
        if (!std::isfinite(preds[i])) {
          ATH_MSG_WARNING("Non-finite prediction detected at " << i << " -> set to -100.");
          preds[i] = -100.0f;
        }
      }
    }

    for (auto& v : outputs) {
      graphData.graph->dataTensor.emplace_back(std::move(v));
    }
    return StatusCode::SUCCESS;
  }

  // ---- CPU path ----
  std::vector<Ort::Value> outputs =
      model().Run(run_options,
                  inputNames.data(),
                  graphData.graph->dataTensor.data(),
                  inputNames.size(),
                  outputNames.data(),
                  outputNames.size());

  if (outputs.empty()) {
    ATH_MSG_ERROR("Inference returned empty output.");
    return StatusCode::FAILURE;
  }

  float* outData = outputs[0].GetTensorMutableData<float>();
  const size_t outSize = outputs[0].GetTensorTypeAndShapeInfo().GetElementCount();
  ATH_MSG_DEBUG("ONNX raw output elementCount = " << outSize);

  if (m_sanitizeNonFinitePredictions.value()) {
    std::span<float> preds(outData, outData + outSize);
    for (size_t i = 0; i < outSize; ++i) {
      if (!std::isfinite(preds[i])) {
        ATH_MSG_WARNING("Non-finite prediction detected at " << i << " -> set to -100.");
        preds[i] = -100.0f;
      }
    }
  }

  for (auto& v : outputs) {
    graphData.graph->dataTensor.emplace_back(std::move(v));
  }
  return StatusCode::SUCCESS;
}

StatusCode BucketInferenceToolBase::runInference(GraphRawData& graphData) const {
  std::vector<const char*> inputNames  = {"features", "edge_index"};
  std::vector<const char*> outputNames = {m_outputName.value().c_str()};
  return runNamedInference(graphData, inputNames, outputNames);
}