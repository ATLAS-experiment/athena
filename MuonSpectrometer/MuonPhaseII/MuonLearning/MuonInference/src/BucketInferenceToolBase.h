/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCETOOLS_BUCKETINFERENCETOOLBASE_H
#define MUONINFERENCETOOLS_BUCKETINFERENCETOOLBASE_H

#include "MuonInferenceInterfaces/IGraphInferenceTool.h"
#include "MuonInferenceInterfaces/GraphData.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "MuonSpacePoint/SpacePointContainer.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <onnxruntime_cxx_api.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class ActsGeometryContext;

namespace MuonML {

/**
 * BucketInferenceToolBase
 * -----------------------
 * Common infra to:
 *  - read buckets & (optionally) geometry
 *  - build node features
 *  - (optionally) build GNN sparse edges (via BucketGraphUtils)
 *  - wrap tensors and run ONNX sessions
 *
 * GNN-specific operations are in BucketGraphUtils.*
 * Transformer tools reuse feature building without edges and add a pad mask.
 */
class BucketInferenceToolBase : public extends<AthAlgTool, IGraphInferenceTool> {
public:
  using base_class::base_class;
  ~BucketInferenceToolBase() override = default;

  /// GNN-style graph builder (features + edges). Kept for tools that want it.
  StatusCode buildGraph(const EventContext& ctx, GraphRawData& graphData) const;

  /// Default ONNX run for GNN case: inputs {"features","edge_index"} -> outputs {"logits"}
  StatusCode runInference(GraphRawData& graphData) const;

protected:
  static constexpr std::size_t kBucketFeatureCount = 6;
  static constexpr std::size_t kNodeFeatureCount = 10;
  static constexpr std::size_t kEdgeFeatureCount = 7;
  static constexpr std::array<std::string_view, kNodeFeatureCount> kDefaultNodeFeatureNames = {
      "segmentPositionX_m", "segmentPositionY_m", "segmentPositionZ_m",
      "segmentDirectionX", "segmentDirectionY", "segmentDirectionZ",
      "bucket_chamberIndex", "bucket_layers", "bucket_sector", "bucket_segments"};

  StatusCode setupModel();
  Ort::Session& model() const;

  static std::string trimFeatureToken(std::string s);
  static std::vector<std::string> parseFeatureNames(const std::string& raw);

  /// Build only features (N,6); attaches one tensor in graph.dataTensor[0]
  StatusCode buildFeaturesOnly(const EventContext& ctx, GraphRawData& graphData) const;

  /// Build Transformer inputs:
  ///   features [1,S,6] and pad_mask [1,S] (False = valid), as tensors 0 and 1.
  StatusCode buildTransformerInputs(const EventContext& ctx, GraphRawData& graphData) const;

  /// Generic named inference, for tools with different I/O conventions
  StatusCode runNamedInference(GraphRawData& graphData,
                               const std::vector<const char*>& inputNames,
                               const std::vector<const char*>& outputNames) const;

  SG::ReadHandleKey<MuonR4::SpacePointContainer> m_readKey{this, "ReadSpacePoints", "MuonSpacePoints"};
  ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

  // ONNX I/O name for the GNN output tensor (model-dependent)
  Gaudi::Property<std::string> m_outputName{this, "OutputName", "logits"};

  // Sparse-graph parameters (GNN)
  Gaudi::Property<int>    m_minLayers{this, "MinLayersValid", 3};
  Gaudi::Property<int>    m_maxChamberDelta{this, "MaxChamberDelta", 13};
  Gaudi::Property<int>    m_maxSectorDelta{this, "MaxSectorDelta", 1};
  Gaudi::Property<double> m_maxDistXY{this, "MaxDistXY", 6800.0};
  Gaudi::Property<double> m_maxAbsDz{this, "MaxAbsDz", 15000.0};

  // Debug/validation knobs
  Gaudi::Property<unsigned int> m_debugDumpFirstNNodes{this, "DebugDumpFirstNNodes", 5};
  Gaudi::Property<unsigned int> m_debugDumpFirstNEdges{this, "DebugDumpFirstNEdges", 12};
  Gaudi::Property<bool>         m_validateEdges{this, "ValidateEdges", true};
  Gaudi::Property<bool>         m_sanitizeNonFinitePredictions{
      this, "SanitizeNonFinitePredictions", false,
      "When true, replace non-finite ONNX outputs with -100 and log a warning."};

  // CUDA / IoBinding state (set in setupModel via dynamic_cast)
  bool m_isCuda{false};
  int  m_cudaDeviceId{0};

private:
  ToolHandle<AthOnnx::IOnnxRuntimeSessionTool> m_onnxSessionTool{
      this, "ModelSession", ""};
};

} // namespace MuonML

#endif

