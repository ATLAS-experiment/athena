/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCETOOLS_DVINFERENCETOOLBASE_H
#define MUONINFERENCETOOLS_DVINFERENCETOOLBASE_H

#include "MuonInferenceInterfaces/GraphData.h"
#include "MuonInferenceInterfaces/IGraphInferenceTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "CaloEvent/CaloTowerContainer.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <onnxruntime_cxx_api.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace MuonML {

struct DVInferenceResult {
  bool valid{false};
  std::size_t nNodes{0};
  std::size_t nMuonNodes{0};
  std::size_t nCaloNodes{0};
  std::size_t nEdges{0};
  float rawOutput{0.f};
  float probability{0.f};
};

/**
 * @brief Athena tool for DisplacedVertex graph-level ONNX inference.
 *
 * The current exported model embeds any training normalization and consumes raw
 * tensors matching dv_converter_utils.py:
 *
 *   x            [num_nodes, 7]
 *   edge_index   [2, num_edges]
 *   edge_attr    [num_edges, 5]
 *   n_muon_nodes [1]
 *   logits       [1]
 *
 * Nodes are ordered as muon-segment nodes first, followed by calorimeter tower
 * nodes, because the model-side normalizer uses n_muon_nodes to split the raw
 * x tensor into muon and calo node blocks.
 */
class DVInferenceToolBase : public extends<AthAlgTool, IGraphInferenceTool> {
public:
  using base_class::base_class;
  ~DVInferenceToolBase() override = default;

  StatusCode initialize() override;

  /// IGraphInferenceTool entry point: build the DV event graph and run ONNX.
  StatusCode runGraphInference(const EventContext& ctx, GraphRawData& graphData) const override;

  /// Build the DV ONNX input tensors: x, edge_index, edge_attr, n_muon_nodes.
  StatusCode buildGraph(const EventContext& ctx, GraphRawData& graphData) const;

  /// Run the configured ONNX session on a graph already built by buildGraph.
  StatusCode runInference(GraphRawData& graphData) const;

  /// Convenience event-classifier API used by DVInferenceAlg.
  StatusCode inferEvent(const EventContext& ctx, DVInferenceResult& result) const;

protected:
  static constexpr std::size_t kNodeFeatureCount = 7;
  static constexpr std::size_t kEdgeFeatureCount = 5;
  static constexpr std::size_t kInputTensorCount = 4;

  static constexpr std::array<std::string_view, kNodeFeatureCount> kDefaultNodeFeatureNames = {
      "r_pos", "theta_pos", "phi_pos", "theta_dir", "phi_dir", "energy_like", "nCells_or_DoF"};
  static constexpr std::array<std::string_view, kEdgeFeatureCount> kDefaultEdgeFeatureNames = {
      "d_energy_like", "d_phi", "d_eta", "cos_angle", "same_sector"};

  StatusCode setupModel();
  Ort::Session& model() const;

  struct InputTensorSpec {
    std::string name{};
    std::size_t tensorIndex{0};
  };

  std::vector<std::string> modelInputNames() const;
  std::vector<std::string> modelOutputNames() const;

  StatusCode runNamedInference(GraphRawData& graphData,
                               const std::vector<InputTensorSpec>& inputSpecs,
                               const std::vector<std::string>& outputNames) const;

  float probabilityFromOutput(const Ort::Value& output, float& rawOutput) const;

  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{
      this, "SegmentKey", "MuonSegmentsFromR4", "Input R4 muon segment container"};
  SG::ReadHandleKeyArray<MuonR4::SpacePointContainer> m_spacePointKeys{
      this, "SpacePointKeys", {"MuonSpacePoints"},
      "Default is MuonSpacePoints only, matching the training MuonBucketDump SegmentKey alignment."};
  SG::ReadHandleKey<CaloTowerContainer> m_towerKey{
      this, "TowerContainerKey", "CombinedTower", "Input calorimeter tower container"};

  Gaudi::Property<float> m_minTowerEnergyMeV{
      this, "MinTowerEnergyMeV", 1000.f, "Minimum calo tower energy used as a DV graph node"};
  Gaudi::Property<float> m_maxTowerSegmentDR{
      this, "MaxTowerSegmentDR", 0.4f, "Maximum segment-calo deltaR used in the converter"};
  Gaudi::Property<float> m_caloRMaxMm{
      this, "CaloRMaxMm", 4250.f, "Barrel radius used for the calo-envelope intersection in mm"};
  Gaudi::Property<float> m_caloZMaxMm{
      this, "CaloZMaxMm", 6500.f, "Endcap |z| used for the calo-envelope intersection in mm"};
  Gaudi::Property<int> m_sectorModulo{
      this, "SectorModulo", 16, "Number of sectors used by the calo phi->sector converter"};
  Gaudi::Property<bool> m_requireEdges{
      this, "RequireEdges", false, "Skip inference when the event graph has no segment-tower edges"};
  Gaudi::Property<bool> m_useBucketSegmentSelection{
      this, "UseBucketSegmentSelection", true, "Build muon nodes from segment-parent SpacePoint buckets"};
  Gaudi::Property<bool> m_fallbackToAllSegments{
      this, "FallbackToAllSegments", false, "If bucket-segment matching fails, fall back to all SegmentKey segments."};
      Gaudi::Property<int> m_maxEdges{this, "MaxEdges", -1, "Maximum number of directed segment-tower edges to create; negative means no cap"};

  Gaudi::Property<std::string> m_inputNodeName{this, "InputNodeName", "x"};
  Gaudi::Property<std::string> m_inputEdgeIndexName{this, "InputEdgeIndexName", "edge_index"};
  Gaudi::Property<std::string> m_inputEdgeAttrName{this, "InputEdgeAttrName", "edge_attr"};
  Gaudi::Property<std::string> m_inputNMuonNodesName{this, "InputNMuonNodesName", "n_muon_nodes"};
  Gaudi::Property<std::string> m_outputName{this, "OutputName", "logits"};

  Gaudi::Property<std::string> m_singleOutputMode{
      this, "SingleOutputMode", "logit", "How to interpret a one-value output: auto, logit, or prob"};

  Gaudi::Property<unsigned int> m_debugDumpFirstNNodes{this, "DebugDumpFirstNNodes", 0};
  Gaudi::Property<unsigned int> m_debugDumpFirstNEdges{this, "DebugDumpFirstNEdges", 0};
  Gaudi::Property<bool> m_sanitizeNonFiniteInputs{
      this, "SanitizeNonFiniteInputs", true, "Replace non-finite input features with zero before creating ONNX tensors"};
  Gaudi::Property<bool> m_sanitizeNonFinitePredictions{
      this, "SanitizeNonFinitePredictions", false, "Replace non-finite ONNX outputs with -100 and log a warning"};

  bool m_isCuda{false};
  int m_cudaDeviceId{0};

private:
  ToolHandle<AthOnnx::IOnnxRuntimeSessionTool> m_onnxSessionTool{
      this, "ModelSession", "", "ONNX Runtime session tool for the DV classifier"};
};

}  // namespace MuonML

#endif
