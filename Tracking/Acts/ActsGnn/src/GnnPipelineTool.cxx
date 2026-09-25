/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GnnPipelineTool.h"

#ifdef ACTS_GNN_WITH_MODULEMAP

#include <algorithm>
#include <numbers>
#include <numeric>

#include "Acts/Utilities/MathHelpers.hpp"
#include "ActsPlugins/Gnn/CudaTrackBuilding.hpp"
#include "ActsPlugins/Gnn/GnnPipeline.hpp"
#include "ActsPlugins/Gnn/ModuleMapCuda.hpp"
#include "ActsPlugins/Gnn/OnnxEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TensorRTEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TorchEdgeClassifier.hpp"
#include "AthenaKernel/Chrono.h"
#include "detail/GnnFeatures.h"

#endif

namespace ActsTrk {

GnnPipelineTool::GnnPipelineTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : base_class(type, name, parent) {}

GnnPipelineTool::~GnnPipelineTool() = default;

#ifdef ACTS_GNN_WITH_MODULEMAP

StatusCode GnnPipelineTool::initialize() {
  m_logger = makeActsAthenaLogger(this, "ActsGnn");

  if (m_numFeatures.value() != 4 && m_numFeatures.value() != 12) {
    ATH_MSG_ERROR("numFeatures must be 4 or 12, got " << m_numFeatures.value());
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_chronoSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_pixelIdHelper, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripIdHelper, "SCT_ID"));

  ActsPlugins::ModuleMapCuda::Config gcCfg;
  gcCfg.rScale = 1000.f;
  gcCfg.zScale = 1000.f;
  gcCfg.phiScale = std::numbers::pi_v<float>;
  gcCfg.moduleMapPath = m_moduleMapPath.value();
  gcCfg.gpuBlocks = 512;
  std::shared_ptr<ActsPlugins::ModuleMapCuda> gc =
    std::make_shared<ActsPlugins::ModuleMapCuda>(
      gcCfg, m_logger->cloneWithSuffix("ModuleMap"));
  
  std::shared_ptr<ActsPlugins::EdgeClassificationBase> gnn;
  if (m_gnnPath.value().find(".onnx") != std::string::npos) {
#ifdef ACTS_GNN_ONNX_BACKEND
    ActsPlugins::OnnxEdgeClassifier::Config gnnCfg;
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.cut = m_edgeCut.value();
    gnn = std::make_shared<ActsPlugins::OnnxEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
#else
    ATH_MSG_ERROR("GNN .onnx selected but build lacks ONNX backend");
    return StatusCode::FAILURE;
#endif
  } else if (m_gnnPath.value().find(".pt") != std::string::npos) {
#ifdef ACTS_GNN_TORCH_BACKEND
    ActsPlugins::TorchEdgeClassifier::Config gnnCfg;
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.cut = m_edgeCut.value();
    gnnCfg.useEdgeFeatures = true;
    gnn = std::make_shared<ActsPlugins::TorchEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
#else
    ATH_MSG_ERROR("GNN .pt selected but build lacks libtorch backend");
    return StatusCode::FAILURE;
#endif
  } else if (m_gnnPath.value().find(".engine") != std::string::npos) {
#ifdef ACTS_GNN_WITH_TENSORRT
    ActsPlugins::TensorRTEdgeClassifier::Config gnnCfg;
    gnnCfg.cut = m_edgeCut.value();
    gnnCfg.modelPath = m_gnnPath.value();
    gnnCfg.numExecutionContexts = m_numTrtContexts.value();
    gnn = std::make_shared<ActsPlugins::TensorRTEdgeClassifier>(
        gnnCfg, m_logger->cloneWithSuffix("GNN"));
#else
    ATH_MSG_ERROR("GNN .engine selected but build lacks TensorRT backend");
    return StatusCode::FAILURE;
#endif
  } else {
    ATH_MSG_ERROR("Unknown GNN model extension: " << m_gnnPath.value());
    return StatusCode::FAILURE;
  }

  ActsPlugins::CudaTrackBuilding::Config tbCfg;
  std::shared_ptr<ActsPlugins::TrackBuildingBase> tb;
  tbCfg.doJunctionRemoval = true;
  tb = std::make_shared<ActsPlugins::CudaTrackBuilding>(
      tbCfg, m_logger->cloneWithSuffix("GraphSeg"));

  m_gnnPipeline = std::make_unique<ActsPlugins::GnnPipeline>(
      gc, std::vector{gnn}, tb, m_logger->cloneWithSuffix("Pipeline"));

  // Limit the total number of instances on the GPU to avoid out of memory
  m_gpuInstanceCount.emplace(m_maxGpuInstances.value());

  ACTS_INFO("Use phi overlap spacepoints: " << std::boolalpha
                                            << m_usePhiOverlapSps.value());
  return StatusCode::SUCCESS;
}

StatusCode GnnPipelineTool::buildSeed(
    const std::vector<const xAOD::SpacePointContainer*>& spacePointCollections,
    ActsTrk::SeedContainer& seeds) const {
  std::vector<float> features;
  std::vector<std::uint64_t> moduleIds;
  std::vector<int> ids;
  std::vector<const xAOD::SpacePoint*> allSPPtrs;
  ATH_CHECK(buildFeatures(spacePointCollections, features, moduleIds, ids,
                          allSPPtrs, m_numFeatures.value()));

  std::optional<Athena::Chrono> timer;
  timer.emplace("GNN inference", m_chronoSvc.get());

  m_gpuInstanceCount->acquire();
  auto candidates =
      m_gnnPipeline->run(features, moduleIds, ids,
                         ActsPlugins::Device::Cuda(m_cudaDeviceIndex.value()));
  m_gpuInstanceCount->release();

  ACTS_DEBUG("Have " << candidates.size() << " candidates after GNN");

  // Rough estimate of the number of spacepoints per seed
  seeds.reserve(candidates.size(), 1 + m_minCandidateMeasurements.value() * 2);

  // Remove candidates that have too few measurements or no pixel hit
  auto candidateSelector = [&](const std::vector<int>& c) {
    bool tooFewMeasurements =
        std::accumulate(c.begin(), c.end(), 0ul, [&](auto sum, auto spi) {
          return sum + allSPPtrs.at(spi)->measurements().size();
        }) < m_minCandidateMeasurements.value();
    bool noPixelHits = !std::ranges::any_of(c, [&](auto spi) {
      return allSPPtrs.at(spi)->measurements().size() == 1;
    });
    return tooFewMeasurements || noPixelHits;
  };

  for (const auto& candidate : candidates) {
    if (candidateSelector(candidate)) {
      continue;
    }
    std::vector<const xAOD::SpacePoint*> seedSPs;
    seedSPs.reserve(candidate.size());
    for (int spi : candidate) {
      seedSPs.push_back(allSPPtrs.at(spi));
    }
    // Order the space points from the innermost to the outermost one, as
    // expected by the downstream parameter estimation and track finding
    std::ranges::sort(seedSPs, [](const xAOD::SpacePoint* a,
                                  const xAOD::SpacePoint* b) {
      return Acts::fastHypot(a->x(), a->y(), a->z()) <
             Acts::fastHypot(b->x(), b->y(), b->z());
    });
    constexpr float quality = 0.f; // quality is not computed in the GNN pipeline
    constexpr float vertexZ = 0.f; // vertexZ is not computed in the GNN pipeline
    seeds.push_back(ActsTrk::SpacePointRange(seedSPs.data(), seedSPs.size()),
                    quality, vertexZ);
  }

  ACTS_DEBUG("Candidates left with >= " << m_minCandidateMeasurements.value()
                                        << " measurements: " << seeds.size());
  return StatusCode::SUCCESS;
}

#else  // ACTS_GNN_WITH_MODULEMAP

StatusCode GnnPipelineTool::initialize() {
  m_logger = makeActsAthenaLogger(this, "ActsGnn");
  ACTS_ERROR("Cannot initialize ActsGnn without the Acts GNN plugin");
  return StatusCode::FAILURE;
}

StatusCode GnnPipelineTool::buildSeed(
    const std::vector<const xAOD::SpacePointContainer*>& /*spacePointCollections*/,
    ActsTrk::SeedContainer& /*seeds*/) const {
  return StatusCode::FAILURE;
}

#endif  // ACTS_GNN_WITH_MODULEMAP

}  // namespace ActsTrk
