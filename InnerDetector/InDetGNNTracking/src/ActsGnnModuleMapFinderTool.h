/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ActsGnnModuleMapFinderTool_H
#define ActsGnnModuleMapFinderTool_H

#include <array>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"
#include "ISpacepointFeatureTool.h"
#include "Acts/Utilities/Logger.hpp"
#include "ActsPlugins/Gnn/GnnPipeline.hpp"


class MsgStream;

namespace InDet {

  /**
   * @class InDet::ActsGnnModuleMapFinderTool
   * @brief Tool that produces track candidates using the ACTS GNN pipeline
   * with module-map graph construction, edge classification, and track building.
   * Implements the IGNNTrackFinder interface for use with SiSPGNNTrackMaker.
   */
  class ActsGnnModuleMapFinderTool : public extends<AthAlgTool, IGNNTrackFinder>
  {
  public:
    using extends::extends; 

    virtual StatusCode initialize() override;

    virtual StatusCode getTracks(
      const std::vector<const Trk::SpacePoint*>& spacepoints,
      std::vector<std::vector<uint32_t>>& tracks,
      std::unordered_map<int, std::unordered_map<int, float>>* edgeMap = nullptr) const override;

    virtual MsgStream&    dump(MsgStream&    out) const override;
    virtual std::ostream& dump(std::ostream& out) const override;

  private:
    // Hardcoded feature configuration for module-map chain
    static constexpr std::size_t NUM_FEATURES = 12;

    static constexpr std::array<const char*, NUM_FEATURES> FEATURE_NAMES = {{
      "r", "phi", "z", "eta",
      "cluster_r_1", "cluster_phi_1", "cluster_z_1", "cluster_eta_1",
      "cluster_r_2", "cluster_phi_2", "cluster_z_2", "cluster_eta_2"
    }};

    static constexpr float kScaleR   = 1000.f;
    static constexpr float kScalePhi = 3.14159265359f;
    static constexpr float kScaleZ   = 1000.f;
    static constexpr float kScaleEta = 1.f;

    static constexpr std::array<float, NUM_FEATURES> FEATURE_SCALES = {{
      kScaleR, kScalePhi, kScaleZ, kScaleEta,
      kScaleR, kScalePhi, kScaleZ, kScaleEta,
      kScaleR, kScalePhi, kScaleZ, kScaleEta
    }};

    // Gaudi properties
    StringProperty m_moduleMapPath{this, "moduleMapPath", "", "Path to module map ROOT files"};
    StringProperty m_gnnPath{this, "gnnPath", "", "Path to GNN model (.onnx, .pt, or .engine)"};
    FloatProperty m_edgeCut{this, "edgeCut", 0.5, "Edge classification cut"};
    UnsignedIntegerProperty m_numTrtContexts{this, "numTrtContexts", 1, "Number of TensorRT execution contexts (controls concurrency)"};
    UnsignedIntegerProperty m_minCandidateMeasurements{this, "minCandidateMeasurements", 7, "Min measurements per candidate"};
    BooleanProperty m_useEdgeLayerConnector{this, "useEdgeLayerConnector", false, "Use the EdgeLayerConnector instead of CC&JR as graph segmentation algorithm"};
    IntegerProperty m_elcMaxHitsPerTrack{this, "elcMaxHitsPerTrack", 30, "Max hits per track config for the EdgeLayerConnector"};

    // Tool handles
    ToolHandle<ISpacepointFeatureTool> m_spacepointFeatureTool{
      this, "SpacepointFeatureTool", "InDet::SpacepointFeatureTool"};

    // ACTS pipeline
    std::unique_ptr<ActsPlugins::GnnPipeline> m_gnnPipeline;
    std::unique_ptr<const Acts::Logger> m_logger;

    // Present for ONNX and Torch (not thread-safe); absent for TRT (handles concurrency internally)
    mutable std::optional<std::mutex> m_runMutex ATLAS_THREAD_SAFE{};
  };

}

#endif
