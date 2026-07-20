/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TrackFindingGNNAlg.h"
#include "src/detail/FitterHelperFunctions.h"
#include <numbers>

// Athena
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"

// ACTS
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Utilities/MathHelpers.hpp"

// ActsTrk
#include "ActsCalibBase/MeasurementCalibratorBase.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsInterop/Logger.h"
#include "src/detail/SharedHitCounter.h"
#include "src/detail/TrackFindingMeasurements.h"

// STL
#include <algorithm>
#include <optional>
#include <sstream>
#include <utility>
#include <boost/container/small_vector.hpp>

#include "ActsPlugins/Gnn/CudaTrackBuilding.hpp"
#include "ActsPlugins/Gnn/GnnPipeline.hpp"
#include "ActsPlugins/Gnn/ModuleMapCuda.hpp"
#include "ActsPlugins/Gnn/OnnxEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TensorRTEdgeClassifier.hpp"
#include "ActsPlugins/Gnn/TorchEdgeClassifier.hpp"


using namespace Acts::UnitLiterals;

// TODO: Use the function from the InDetGNN package, but it is not cmake module
// we can link to
namespace {
int compute_overlap_SP_flag(const int &eta_module_cl1,
                            const int &phi_module_cl1,
                            const int &eta_module_cl2,
                            const int &phi_module_cl2) {
  int flag = -999;

  if ((eta_module_cl1 == eta_module_cl2) &&
      (phi_module_cl1 == phi_module_cl2)) {
    flag = 0; // not an overlap Space Point
  } else if ((eta_module_cl1 != eta_module_cl2) &&
             (phi_module_cl1 == phi_module_cl2)) {
    flag = 1; // overlap Space Point in eta only
  } else if ((eta_module_cl1 == eta_module_cl2) &&
             (phi_module_cl1 != phi_module_cl2)) {
    flag = 2; // overlap Space Point in phi only
  } else {
    flag = 3; // "overlap" Space Point in eta and phi (not sure we can call it
              // overlap)
  }
  return flag;
}
} // namespace

namespace ActsTrk {


TrackFindingGNNAlg::TrackFindingGNNAlg(const std::string &name,
                                       ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

TrackFindingGNNAlg::~TrackFindingGNNAlg() = default;

// === initialize ==========================================================

StatusCode TrackFindingGNNAlg::initialize() {
  // Athena tools
  m_logger = makeActsAthenaLogger(this, "Acts GNN Algorithm");
  ACTS_DEBUG("TrackFindingGNNAlg::initialize() - begin");
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_ctxProvider.initialize());
  ATH_CHECK(m_xaodPixelSpacePointContainerKey.initialize());
  ATH_CHECK(m_xaodStripSpacePointContainerKey.initialize());
  ATH_CHECK(m_xaodStripSpacePointOverlapContainerKey.initialize());
  ATH_CHECK(m_trackContainerKey.initialize());
  ATH_CHECK(m_chronoSvc.retrieve());
  ATH_CHECK(m_paramEstimationTool.retrieve());
  ATH_CHECK(m_fitterTool.retrieve());

  m_uncalibMeasSurfAccessor =
      detail::xAODUncalibMeasSurfAcc{m_trackingGeometryTool.get()};
  m_uncalibMeasCalibrator =
      detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend>::
          NoCalibration(m_trackingGeometryTool.get());

  // ACTS tools
  ActsPlugins::ModuleMapCuda::Config gcCfg;
  gcCfg.rScale = 1000.f;
  gcCfg.zScale = 1000.f;
  gcCfg.phiScale = std::numbers::pi_v<float>;
  gcCfg.moduleMapPath = m_moduleMapPath.value();
  gcCfg.gpuBlocks = 512;
  std::shared_ptr<ActsPlugins::GraphConstructionBase> gc =
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
  tbCfg.doJunctionRemoval = true;
  std::shared_ptr<ActsPlugins::TrackBuildingBase> tb =
      std::make_shared<ActsPlugins::CudaTrackBuilding>(
          tbCfg, m_logger->cloneWithSuffix("GraphSeg"));

  m_gnnPipeline = std::make_unique<ActsPlugins::GnnPipeline>(
      gc, std::vector{gnn}, tb, m_logger->cloneWithSuffix("Pipeline"));

  // Limit the total number of instances on the GPU to avoid out of memory
  m_gpuInstanceCount.emplace(m_maxGpuInstances.value());

  // Parameter estimation and fitter come from Athena tools now

  ATH_CHECK(detStore()->retrieve(m_stripIdHelper, "SCT_ID"));
  ACTS_INFO("Use phi overlap spacepoints: " << std::boolalpha
                                            << m_usePhiOverlapSps.value());

  using TSC = Acts::TrackSelector::Config;
  m_trackSelectorConfig = Acts::TrackSelector::EtaBinnedConfig(0.0);

  auto commonConfig = [&](TSC &config) {
    config.requireReferenceSurface = true;
    config.loc1Min = m_offlineZ0Sel.value() ? -200_mm : -150_mm; // z0 min
    config.loc1Max = m_offlineZ0Sel.value() ? 200_mm : 150_mm;   // z0 max
  };

  m_trackSelectorConfig
      .addCuts(2.0,
               [&](TSC &config) {
                 commonConfig(config);
                 config.maxHoles = m_relaxCentralHoleSel.value() ? 4 : 2; 
                 config.minMeasurements = m_relaxMeasurementSel.value() ? 7 : 9; 
                 config.ptMin = 900_MeV;
                 config.loc0Max = 2_mm;    // d0 max
                 config.loc0Min = -2_mm;   // d0 min
               })
      .addCuts(2.6,
               [&](TSC &config) {
                 commonConfig(config);
                 config.maxHoles = m_relaxCentralHoleSel.value() ? 4 : 2;
                 config.minMeasurements = m_relaxMeasurementSel.value() ? 7 : 8;
                 config.ptMin = 400_MeV;
                 config.loc0Max = 2_mm;    // d0 max
                 config.loc0Min = -2_mm;   // d0 min
               })
      .addCuts([&](TSC &config) {
        commonConfig(config);
        config.maxHoles = 2;
        config.minMeasurements = 7;
        config.ptMin = 400_MeV;
        config.loc0Max = 10_mm;   // d0 max
        config.loc0Min = -10_mm;  // d0 min
      });

  ACTS_INFO("Track selector config:\n" << m_trackSelectorConfig);

  ACTS_DEBUG("TrackFindingGNNAlg::initialize() - end");
  return StatusCode::SUCCESS;
}

// === execute =============================================================

StatusCode TrackFindingGNNAlg::execute(const EventContext &ctx) const {
  ACTS_DEBUG("TrackFindingGNNAlg::execute() - begin");

  std::optional<Athena::Chrono> timer;
  timer.emplace("GNN get spacepoint handles", m_chronoSvc.get());

  const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);
  const Acts::MagneticFieldContext mctx = m_ctxProvider.getMagneticFieldContext(ctx);
  const Acts::CalibrationContext cctx = m_ctxProvider.getCalibrationContext(ctx);

  auto detElToGeoIdMap = m_trackingGeometryTool->surfaceIdMap();

  // Build features
  auto pixelSPHandle = SG::makeHandle(m_xaodPixelSpacePointContainerKey, ctx);
  ATH_CHECK(pixelSPHandle.isValid());
  const auto &pixelSPContainer = *pixelSPHandle.cptr();

  auto stripSPHandle = SG::makeHandle(m_xaodStripSpacePointContainerKey, ctx);
  ATH_CHECK(stripSPHandle.isValid());
  const auto &stripSPContainer = *stripSPHandle.cptr();

  auto stripSPOVHandle =
      SG::makeHandle(m_xaodStripSpacePointOverlapContainerKey, ctx);
  ATH_CHECK(stripSPOVHandle.isValid());
  const auto &stripSPOVContainer = *stripSPOVHandle.cptr();

  constexpr std::size_t nFeatures = 12;
  std::size_t nSP = pixelSPContainer.size() + stripSPContainer.size() +
                    stripSPOVContainer.size();

  ACTS_DEBUG("Number spacepoints: "
             << nSP << " (" << "pixel: " << pixelSPContainer.size() << ", "
             << "strip: " << stripSPContainer.size() << ", "
             << "strip overlap: " << stripSPOVContainer.size() << ")");


  timer.emplace("GNN extract data", m_chronoSvc.get());

  std::vector<std::uint64_t> moduleIds;
  moduleIds.reserve(nSP);
  std::vector<const xAOD::SpacePoint *> allSPPtrs;
  allSPPtrs.reserve(nSP);
  std::vector<Acts::GeometryIdentifier> geoIds, sortedGeoIds(nSP);
  geoIds.reserve(nSP);

  std::size_t skipped = 0;
  for (const auto &spc :
       {pixelSPContainer, stripSPContainer, stripSPOVContainer}) {
    for (auto sp : spc) {
      auto cl1 = sp->measurements().front();
      auto geoIdCl1 =
          ActsTrk::getSurfaceGeometryIdOfMeasurement(*detElToGeoIdMap, *cl1);
      Identifier atlasIdCl1(static_cast<Identifier::value_type>(cl1->identifier()));

      if (/*!m_usePhiOverlapSps.value() &&*/ sp->measurements().size() == 2) {
        auto cl2 = sp->measurements().at(1);
        Identifier atlasIdCl2(static_cast<Identifier::value_type>(cl2->identifier()));

        auto overlapFlag =
            compute_overlap_SP_flag(m_stripIdHelper->eta_module(atlasIdCl1),
                                    m_stripIdHelper->phi_module(atlasIdCl1),
                                    m_stripIdHelper->eta_module(atlasIdCl2),
                                    m_stripIdHelper->phi_module(atlasIdCl2));

        if (overlapFlag == 2 || overlapFlag == 3) {
          skipped++;
          ACTS_VERBOSE("Skip phi overlap spacepoint (flag=" << overlapFlag
                                                            << ")");
          continue;
        }
      }

      geoIds.push_back(geoIdCl1);
      moduleIds.push_back(atlasIdCl1.get_compact());
      allSPPtrs.push_back(sp);
    }
  }

  ACTS_DEBUG("Skipped " << skipped << " SPs because of phi overlap");
  nSP = allSPPtrs.size();
  ACTS_DEBUG("Keep " << nSP << " SPs for feature creation");

  timer.emplace("GNN build input tensor", m_chronoSvc.get());

  std::vector<std::size_t> idxs(nSP);
  std::iota(idxs.begin(), idxs.end(), 0);

  std::ranges::sort(
      idxs, [&](auto a, auto b) { return moduleIds.at(a) < moduleIds.at(b); });
  std::ranges::sort(moduleIds);

  std::vector<float> features(nFeatures * nSP);
  std::vector<boost::container::static_vector<Acts::SourceLink, 2>> sourceLinks(
      nSP);
  std::vector<int> id(nSP);

  for (auto k = 0ul; k < nSP; k++) {
    id.at(k) = k;
    auto i = idxs.at(k);

    std::span<float> f(features.data() + k * nFeatures, nFeatures);
    const auto &sp = *allSPPtrs.at(i);

    using namespace Acts::VectorHelpers;
    using namespace Acts::AngleHelpers;

    Acts::Vector3 spp{sp.x(), sp.y(), sp.z()};

    if (sp.measurements().size() == 1) {
      for (auto j = 0ul; j < nFeatures; j += 4) {
        f[j + 0] = perp(spp) / 1000.f;
        f[j + 1] = phi(spp) / std::numbers::pi_v<float>;
        f[j + 2] = sp.z() / 1000.f;
        f[j + 3] = eta(spp);
      }
    } else {
      std::size_t j = 0;
      f[j + 0] = perp(spp) / 1000.f;
      f[j + 1] = phi(spp) / std::numbers::pi_v<float>;
      f[j + 2] = sp.z() / 1000.f;
      f[j + 3] = eta(spp);

      for (auto m : sp.measurements()) {
        auto cl = static_cast<const xAOD::StripCluster *>(m);
        auto gp = cl->globalPosition();
        j += 4;
        f[j + 0] = perp(gp) / 1000.f;
        f[j + 1] = phi(gp) / std::numbers::pi_v<float>;
        f[j + 2] = gp.z() / 1000.f;
        f[j + 3] = eta(gp);
      }
    }

    for (const xAOD::UncalibratedMeasurement* m : sp.measurements()) {
      sourceLinks.at(k).push_back(detail::MeasurementCalibratorBase::pack(m));
    }

    sortedGeoIds.at(k) = geoIds.at(i);
  }

  timer.reset();
  timer.emplace("GNN inference", m_chronoSvc.get());

  m_gpuInstanceCount->acquire();
  auto candidates =
      m_gnnPipeline->run(features, moduleIds, id, ActsPlugins::Device::Cuda(m_cudaDeviceIndex.value()));
  m_gpuInstanceCount->release();

  ACTS_DEBUG("Have " << candidates.size() << " candidates after GNN");

  // Remove candidates if they either have less then the configured amount of measurements, or no pixel hit
  auto candidateSelector = [&](const std::vector<int> &c) {
    bool tooFewMeasurements = std::accumulate(c.begin(), c.end(), 0ul, [&](auto sum, auto spi) {
      return sum + allSPPtrs.at(spi)->measurements().size();
    }) < m_minCandidateMeasurements.value();
    bool noPixelHits = !std::ranges::any_of(c, [&](auto spi) { return allSPPtrs.at(spi)->measurements().size() == 1; });
    return tooFewMeasurements || noPixelHits;
  };

  candidates.erase(std::remove_if(candidates.begin(), candidates.end(), candidateSelector),
                   candidates.end());
  ACTS_DEBUG("Candidates left with >= " << m_minCandidateMeasurements.value()
                                        << " measurements: " << candidates.size());

  timer.reset();
  timer.emplace("GNN parameter estimation + fit", m_chronoSvc.get());

  Acts::VectorTrackContainer trackBackend;
  Acts::VectorMultiTrajectory trackStateBackend;
  constexpr std::size_t nTracksExpected = 3000;
  trackBackend.reserve(nTracksExpected);
  trackStateBackend.reserve(nTracksExpected * 30);
  detail::RecoTrackContainer tracks(trackBackend, trackStateBackend);

  // v45: Create SeedContainer to hold seeds (Seeds are now proxy objects)
  ActsTrk::SeedContainer seedContainer;

  auto makeSeedFromCandidate = [&](const std::vector<int> &cand) -> std::optional<boost::container::small_vector<const xAOD::SpacePoint*, 3>> {
    // Select at least 3 SPs with deltaR spacing in cylindrical coordinates
    boost::container::small_vector<const xAOD::SpacePoint*, 3> picked;
    if (cand.empty()) return std::nullopt;
    auto r_of = [&](const xAOD::SpacePoint* sp) {
      Acts::Vector3 v{sp->x(), sp->y(), sp->z()};
      return v.perp();
    };
    const xAOD::SpacePoint* last = allSPPtrs.at(cand.front());
    picked.push_back(last);
    for (std::size_t i = 1; i < cand.size() && picked.size() < 3; ++i) {
      const xAOD::SpacePoint* sp = allSPPtrs.at(cand.at(i));
      if (std::abs(r_of(sp) - r_of(last)) > m_minDeltaR.value()) {
        picked.push_back(sp);
        last = sp;
      }
    }
    if (picked.size() < 3) return std::nullopt;
    return picked;
  };

  auto retrieveSurface = [&](const ActsTrk::Seed& seed, bool useTopSp) -> const Acts::Surface& {
    const xAOD::SpacePoint* sp = useTopSp ? seed.sp().front() : seed.sp().back();
    auto geoId = ActsTrk::getSurfaceGeometryIdOfMeasurement(*detElToGeoIdMap, *sp->measurements().front());
    const auto* surface = m_trackingGeometryTool->trackingGeometry()->findSurface(geoId);
    if (!surface) {
      throw std::runtime_error("retrieveSurface: no Acts surface for GeometryIdentifier " + std::to_string(geoId.value()));
    }
    return *surface;
  };

  auto R_of = [](const xAOD::SpacePoint* sp) {
    return Acts::fastHypot(sp->x(), sp->y(), sp->z());
  };

  for (const auto &cand : candidates) {
    auto pickedOpt = makeSeedFromCandidate(cand);
    if (!pickedOpt.has_value()) continue;

    auto picked = *pickedOpt;
    std::sort(picked.begin(), picked.end(),
              [&](const xAOD::SpacePoint* a, const xAOD::SpacePoint* b) {
                return R_of(a) < R_of(b);
              });
    ActsTrk::Seed seed = seedContainer.push_back(
        ActsTrk::SpacePointRange(picked.data(), picked.size()), 0.f, 0.f);

    auto initialParamsOpt = m_paramEstimationTool->estimateTrackParameters(
        seed, /*useTopSp=*/true, gctx, mctx, retrieveSurface);
    if (!initialParamsOpt.has_value()) continue;

    boost::container::small_vector<const xAOD::SpacePoint*, 16> sortedSP;
    sortedSP.reserve(cand.size());
    for (int spi : cand) sortedSP.push_back(allSPPtrs.at(spi));
    std::sort(sortedSP.begin(), sortedSP.end(),
              [&](const xAOD::SpacePoint* a, const xAOD::SpacePoint* b) {
                return R_of(a) < R_of(b);
              });

    std::vector<const xAOD::UncalibratedMeasurement*> measList;
    measList.reserve(sortedSP.size() * 2);
    for (const xAOD::SpacePoint* sp : sortedSP) {
      for (const xAOD::UncalibratedMeasurement* m : sp->measurements()) {
        measList.push_back(m);
      }
    }

    auto fitted = m_fitterTool->fit(measList, *initialParamsOpt, gctx, mctx, cctx);
    if (fitted) {
      for (auto track : *fitted) {
        auto newTrack = tracks.makeTrack();
        newTrack.copyFrom(track);
      }
    }
  }

  ACTS_DEBUG("After track fit: " << tracks.size() << " / " << candidates.size()
                                 << " successfull");

  // For single muon/electron case
  if (candidates.size() == 1 && tracks.size() == 1) {
    const auto &t = *tracks.begin();
    ACTS_DEBUG("Single particle case: " << candidates.front().size() << " -> "
                                        << t.nMeasurements()
                                        << " measurements");
  }

  timer.reset();
  timer.emplace("Track selection & conversion", m_chronoSvc.get());

  Acts::VectorTrackContainer selTrackBackend;
  selTrackBackend.reserve(trackBackend.size());
  detail::RecoTrackContainer selectedTracks(selTrackBackend, trackStateBackend);

  Acts::TrackSelector selector(m_trackSelectorConfig);
  for (auto track : tracks) {
    if (selector.isValidTrack(track)) {
      auto newTrack = selectedTracks.makeTrack();
      
      // v45: copyFrom now copies everything including tip/stem indices
      newTrack.copyFrom(track);
    }
  }

  ACTS_DEBUG("GNN cand: " << candidates.size() << ", fitted: " << tracks.size()
                          << ", selected: " << selectedTracks.size());

  // Write tracks to storage again
  Acts::ConstVectorTrackContainer constTrackBackend(std::move(selTrackBackend));
  Acts::ConstVectorMultiTrajectory constTrackStateBackend(std::move(trackStateBackend));
  std::unique_ptr<ActsTrk::TrackContainer> constTracksContainer
    = std::make_unique<ActsTrk::TrackContainer>(std::move(constTrackBackend), std::move(constTrackStateBackend) );
   
  ACTS_DEBUG("Storing track collection with key '" << m_trackContainerKey.key() << "'");
  SG::WriteHandle<ActsTrk::TrackContainer> trackContainerHandle = SG::makeHandle(m_trackContainerKey, ctx);
  ATH_CHECK(trackContainerHandle.record(std::move(constTracksContainer)));

  return StatusCode::SUCCESS;
}
} // namespace ActsTrk
