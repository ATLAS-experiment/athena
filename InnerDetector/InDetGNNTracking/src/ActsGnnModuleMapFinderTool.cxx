/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGnnModuleMapFinderTool.h"

#include "ActsInterop/Logger.h"

#include "ActsPlugins/Gnn/CudaTrackBuilding.hpp"
#include "ActsPlugins/Gnn/GnnPipeline.hpp"
#include "ActsPlugins/Gnn/ModuleMapCuda.hpp"
#include "ActsPlugins/Gnn/OnnxEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TensorRTEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TorchEdgeClassifier.hpp"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ActsGnnHookTool.h"

#include <algorithm>
#include <numeric>


StatusCode InDet::ActsGnnModuleMapFinderTool::initialize() {
  ATH_CHECK(m_spacepointFeatureTool.retrieve());

  m_logger = makeActsAthenaLogger(this, "ActsGnnModuleMapFinderTool");

  // Build ACTS GNN pipeline components

  // 1. Graph constructor (ModuleMapCuda)
  ActsPlugins::ModuleMapCuda::Config gcCfg;
  gcCfg.rScale = kScaleR;
  gcCfg.zScale = kScaleZ;
  gcCfg.phiScale = kScalePhi;
  gcCfg.moduleMapPath = m_moduleMapPath.value();
  gcCfg.gpuBlocks = 512;
  auto gc = std::make_shared<ActsPlugins::ModuleMapCuda>(
      gcCfg, m_logger->cloneWithSuffix("ModuleMap"));

  // 2. Edge classifier (ONNX / Torch / TensorRT)
  // ONNX and Torch are not thread-safe: a mutex is emplaced to serialise run().
  // TensorRT manages concurrency internally via execution contexts: no guard needed.
  std::shared_ptr<ActsPlugins::EdgeClassificationBase> gnn;
  if (m_gnnPath.value().find(".onnx") != std::string::npos) {
#ifdef ACTS_GNN_ONNX_BACKEND
    ActsPlugins::OnnxEdgeClassifier::Config gnnCfg;
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.cut = m_edgeCut.value();
    gnn = std::make_shared<ActsPlugins::OnnxEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
    m_runMutex.emplace();
#else
    ATH_MSG_FATAL("Not compiled with ONNX, cannot interpret *.onnx files");
    return StatusCode::FAILURE;
#endif
  } else if (m_gnnPath.value().find(".pt") != std::string::npos) {
#ifdef ACTS_GNN_TORCH_BACKEND
    ActsPlugins::TorchEdgeClassifier::Config gnnCfg;
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.cut = m_edgeCut.value();
    gnn = std::make_shared<ActsPlugins::TorchEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
    m_runMutex.emplace();
#else
    ATH_MSG_FATAL("Not compiled with Torch, cannot interpret *.pt files");
    return StatusCode::FAILURE;
#endif
  } else if (m_gnnPath.value().find(".engine") != std::string::npos) {
#ifdef ACTS_GNN_WITH_TENSORRT
    ActsPlugins::TensorRTEdgeClassifier::Config gnnCfg;
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.cut = m_edgeCut.value();
    gnnCfg.numExecutionContexts = m_numTrtContexts.value();
    gnn = std::make_shared<ActsPlugins::TensorRTEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
#else
    ATH_MSG_FATAL("Not compiled with TensorRT, cannot interpret *.engine files");
    return StatusCode::FAILURE;
#endif
  } else {
    ATH_MSG_FATAL("Unknown extension for GNN model: " << m_gnnPath.value());
    return StatusCode::FAILURE;
  }

  // 3. Track builder
  ATH_MSG_INFO("Configure CC&JunctionRemoval as graph segmentation algorithm");
  ActsPlugins::CudaTrackBuilding::Config tbCfg;
  tbCfg.doJunctionRemoval = true;
  auto tb = std::make_shared<ActsPlugins::CudaTrackBuilding>(
      tbCfg, m_logger->cloneWithSuffix("CC&JR"));

  // 4. Assemble pipeline
  m_gnnPipeline = std::make_unique<ActsPlugins::GnnPipeline>(
      gc, std::vector{gnn}, tb, m_logger->cloneWithSuffix("Pipeline"));

  return StatusCode::SUCCESS;
}

StatusCode InDet::ActsGnnModuleMapFinderTool::getTracks(
    const std::vector<const Trk::SpacePoint*>& spacepoints,
    std::vector<std::vector<uint32_t>>& tracks,
    std::unordered_map<int, std::unordered_map<int, float>>* edgeMap) const {

  const std::size_t nSP = spacepoints.size();

  ATH_MSG_DEBUG("Processing " << nSP << " spacepoints with " << NUM_FEATURES << " features");

  // Sort spacepoint indices by module ID (required by module map graph construction)
  std::vector<std::size_t> sortIdx(nSP);
  std::iota(sortIdx.begin(), sortIdx.end(), 0);
  std::ranges::sort(sortIdx, std::less{}, [&](std::size_t i) {
    return spacepoints[i]->clusterList().first->detectorElement()->identify().get_compact();
  });

  // Build features, module IDs, and IDs directly in sorted order
  std::vector<float> features(NUM_FEATURES * nSP);
  std::vector<std::uint64_t> moduleIds(nSP);
  std::vector<int> ids(nSP);

  for (std::size_t k = 0; k < nSP; ++k) {
    const std::size_t origIdx = sortIdx[k];
    auto featureMap = m_spacepointFeatureTool->getFeatures(spacepoints[origIdx]);
    // Use detector element identifier, not cluster identifier, to get the module ID
    moduleIds[k] = spacepoints[origIdx]->clusterList().first->detectorElement()->identify().get_compact();
    ids[k] = static_cast<int>(k);
    for (std::size_t j = 0; j < NUM_FEATURES; ++j) {
      features[k * NUM_FEATURES + j] = featureMap[FEATURE_NAMES[j]] / FEATURE_SCALES[j];
    }
  }

  // Run GNN pipeline (mutex present for ONNX/Torch, absent for TRT)
  auto candidates = [&] {
    std::unique_lock<std::mutex> lock;
    if (m_runMutex) lock = std::unique_lock<std::mutex>(*m_runMutex);
    
    if (edgeMap != nullptr) {
      ScoredGraphHook hook;
      auto result = m_gnnPipeline->run(features, moduleIds, ids, ActsPlugins::Device::Cuda(0), hook);

      // Retrieve edgeScores and edgeIndex from hook
      const std::vector<float>& edgeScores = hook.getEdgeScores();
      const std::vector<std::int64_t>& edgeIndex = hook.getEdgeIndex();
      const std::size_t nEdges = edgeScores.size();

      // Create a map to acces edge score (sorted indices back to original spacepoint indices)
      for (std::size_t i = 0; i < nEdges; ++i) {
          std::int64_t src = edgeIndex[i];
          std::int64_t dst = edgeIndex[nEdges + i];
          (*edgeMap)[sortIdx[src]][sortIdx[dst]] = edgeScores[i];
      }
      return result;
    }

    return m_gnnPipeline->run(features, moduleIds, ids, ActsPlugins::Device::Cuda(0));;
  }();

  ATH_MSG_DEBUG("GNN pipeline returned " << candidates.size() << " candidates");

  // Filter by minimum measurements and convert indices back to original ordering
  tracks.clear();
  tracks.reserve(candidates.size());

  for (const auto& candidate : candidates) {
    if (candidate.size() < m_minCandidateMeasurements.value()) {
      continue;
    }

    // Map sorted indices back to original spacepoint indices
    std::vector<uint32_t> track;
    track.reserve(candidate.size());
    for (int sortedIdx : candidate) {
      track.push_back(static_cast<uint32_t>(sortIdx[sortedIdx]));
    }
    tracks.push_back(std::move(track));
  }

  ATH_MSG_DEBUG("Returning " << tracks.size() << " track candidates after filtering (>= "
                << m_minCandidateMeasurements.value() << " measurements)");

  return StatusCode::SUCCESS;
}

MsgStream& InDet::ActsGnnModuleMapFinderTool::dump(MsgStream& out) const {
  out << std::endl;
  out << "|---------------------------------------------------------------------|" << std::endl;
  out << "| ActsGnnModuleMapFinderTool                                          |" << std::endl;
  out << "|---------------------------------------------------------------------|" << std::endl;
  return out;
}

std::ostream& InDet::ActsGnnModuleMapFinderTool::dump(std::ostream& out) const {
  return out;
}
