/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTEDGECLASSIFIERTOOL_H
#define MUONINFERENCE_SEGMENTEDGECLASSIFIERTOOL_H

#include "BucketInferenceToolBase.h"
#include "MuonInferenceInterfaces/ISegmentEdgeClassifierTool.h"
#include "MuonMLEvent.h"
#include "Gaudi/Property.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace MuonML {

  /**
   * @struct BucketSegmentFeatures
   * @brief Segment features derived from or stored in bucket metadata.
   *
   * Replaces opaque array indexing for clarity and encapsulates the per-segment
   * bucket-level attributes needed for edge feature construction.
   */
  struct BucketSegmentFeatures {
    int chamberIndex{0};  ///< Muon chamber index of the segment
    int layers{0};        ///< Total number of active layers in the segment
    int sector{0};        ///< Sector number (typically 0–15)
    int nSegments{0};     ///< Count of segments in the same chamber/sector/eta group
  };

  /**
   * @class SegmentEdgeClassifierTool
   * @brief Runs a segment-level GNN on reconstructed muon segments to classify
   *        segment-pair edges as "good" or "background".
   *
   * The tool reads a xAOD::MuonSegmentContainer and builds a graph where:
   *   - **Nodes** are muon segments, each with 10 features:
   *     - Position and direction (6 floats)
   *     - Chamber index, layer count, sector, and segment multiplicity (4 floats)
   *   - **Edges** connect all segment pairs within an angular threshold
   *     (cos(angle) >= cos(MaxDeltaThetaDeg)) and sector distance, with 7 features:
   *     - Spatial displacement (3 floats: dx, dy, dz)
   *     - Distance magnitude (1 float)
   *     - Angle (dot product, 1 float)
   *     - Chamber and sector match flags (2 flags)
   *
   * The tool then runs an ONNX model (typically a GIN or GCN variant) to produce
   * a logit or probability for each edge, enabling downstream algorithms to filter
   * low-quality segment associations and improve reconstruction efficiency.
   *
   * **Key difference from GraphBucketFilterTool:** operates at segment (edge) level
   * rather than bucket (node) level, and the interface uses discrete graph
   * structures (SegmentEdgeGraph) rather than tensors for input/output.
   *
   * **Note:** runGraphInference() is not supported by this tool; use
   * SegmentEdgeInferenceAlg and the ISegmentEdgeClassifierTool methods instead.
   */
  class SegmentEdgeClassifierTool final : public BucketInferenceToolBase,
                                          virtual public ISegmentEdgeClassifierTool {
  public:
    using BucketInferenceToolBase::BucketInferenceToolBase;

    /// Retrieve the ONNX model and resolve node feature ordering from metadata.
    StatusCode initialize() override;

    /// Log a pre-ONNX candidate-edge pruning.
    StatusCode finalize() override;

    /// Not supported by this tool; returns FAILURE.
    /// Use SegmentEdgeInferenceAlg + buildGraph() + classifyEdges() instead.
    StatusCode runGraphInference(const EventContext& ctx,
                                 GraphRawData& graphData) const override;

    /// Build a GNN graph from @p segments, computing node and edge features
    /// and storing the graph structure in @p graph.
    StatusCode buildGraph(const EventContext& ctx,
                          const xAOD::MuonSegmentContainer& segments,
                          SegmentEdgeGraph& graph) const override;

    /// Run ONNX inference on @p graph and populate @p scores with logit and
    /// probability for each edge; called after buildGraph().
    StatusCode classifyEdges(const EventContext& ctx,
                             const SegmentEdgeGraph& graph,
                             std::vector<SegmentEdgeScore>& scores) const override;

  private:
    StatusCode dumpDebugEvent(const EventContext& ctx,
                              const SegmentEdgeGraph& graph,
                              const std::vector<SegmentEdgeScore>& scores) const;

    /// MC-only diagnostics: record, for every input segment and every pair of
    /// input segments sharing a truth particle, why it did or did not reach
    /// ONNX. Only called when EnableTruthDiagnostics and DEBUG output are set.
    void fillTruthDiagnostics(
        const xAOD::MuonSegmentContainer& segments,
        const std::unordered_set<const xAOD::MuonSegment*>& bucketRetained,
        SegmentEdgeGraph& graph) const;

    Gaudi::Property<float> m_maxDeltaThetaDeg{this, "MaxDeltaThetaDeg", 35.f};
    Gaudi::Property<int> m_maxDeltaSector{this, "MaxDeltaSector", 1};
    Gaudi::Property<int> m_sectorModulo{this, "SectorModulo", 16,
        "Number of muon sectors used when applying wrap-around sector distance"};
    Gaudi::Property<unsigned int> m_maxSegmentsPerBucket{this, "MaxSegmentsPerBucket", 0,
        "Keep at most this many quality-ranked segments per (sector, chamber, eta) bucket before inference; 0 keeps all"};
    Gaudi::Property<unsigned int> m_maxEdgesPerNodeBeforeInference{this, "MaxEdgesPerNodeBeforeInference", 0,
        "Keep at most this many geometrical neighbour pairs per node before ONNX inference; 0 keeps all"};
    Gaudi::Property<unsigned int> m_maxEdgesPerTargetChamberBeforeInference{
        this, "MaxEdgesPerTargetChamberBeforeInference", 0,
        "Keep at most this many pre-ONNX neighbours from one target chamber per node; 0 keeps all"};
    Gaudi::Property<bool> m_dropSameChamberEdgesBeforeInference{this, "DropSameChamberEdgesBeforeInference", true,
        "Drop same-chamber segment pairs before ONNX inference"};
    Gaudi::Property<bool> m_dropIsolatedNodesBeforeInference{this, "DropIsolatedNodesBeforeInference", true,
        "Remove nodes without a retained pre-ONNX edge before creating ONNX tensors"};
    Gaudi::Property<std::string> m_inputNodeName{this, "InputNodeName", "x"};
    Gaudi::Property<std::string> m_inputEdgeIndexName{this, "InputEdgeIndexName", "edge_index"};
    Gaudi::Property<std::string> m_inputEdgeAttrName{this, "InputEdgeAttrName", "edge_attr"};
    Gaudi::Property<std::string> m_outputName{this, "OutputName", "logits"};
    Gaudi::Property<std::string> m_debugDumpFile{this, "DebugDumpFile", ""};
    Gaudi::Property<unsigned int> m_debugDumpMaxEvents{this, "DebugDumpMaxEvents", 0};
    Gaudi::Property<bool> m_enableTruthDiagnostics{
        this, "EnableTruthDiagnostics", false,
        "MC-only: fill SegmentEdgeGraph's truth diagnostics."};
    float m_cosMin{0.f};

    /// Node feature order expected by the model metadata (resolved at initialize).
    std::vector<std::string> m_nodeFeatureNames{};
    std::vector<SegmentNodeFeatureId> m_nodeFeatureIds{};
    mutable std::mutex m_debugDumpMutex;
    mutable std::atomic<unsigned int> m_debugDumpEvents{0};

    /// Job-summed pre-ONNX pruning counters (see buildGraph()).
    mutable std::atomic<std::size_t> m_sumInputSegments{0};
    mutable std::atomic<std::size_t> m_sumCandidatePairs{0};
    mutable std::atomic<std::size_t> m_sumRetainedPairs{0};
    mutable std::atomic<std::size_t> m_sumNodesBeforeIsolatedDrop{0};
    mutable std::atomic<std::size_t> m_sumNodesAfterIsolatedDrop{0};
  };
}
#endif
