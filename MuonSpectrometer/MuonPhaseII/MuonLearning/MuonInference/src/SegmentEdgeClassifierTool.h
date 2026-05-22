/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTEDGECLASSIFIERTOOL_H
#define MUONINFERENCE_SEGMENTEDGECLASSIFIERTOOL_H

#include "BucketInferenceToolBase.h"
#include "MuonInferenceInterfaces/ISegmentEdgeClassifierTool.h"
#include "MuonMLEvent.h"
#include "Gaudi/Property.h"

#include <cstdint>
#include <string>
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
    Gaudi::Property<float> m_maxDeltaThetaDeg{this, "MaxDeltaThetaDeg", 35.f};
    Gaudi::Property<int> m_maxDeltaSector{this, "MaxDeltaSector", 1};
    Gaudi::Property<int> m_sectorModulo{this, "SectorModulo", 16};
    Gaudi::Property<std::string> m_inputNodeName{this, "InputNodeName", "x"};
    Gaudi::Property<std::string> m_inputEdgeIndexName{this, "InputEdgeIndexName", "edge_index"};
    Gaudi::Property<std::string> m_inputEdgeAttrName{this, "InputEdgeAttrName", "edge_attr"};
    Gaudi::Property<std::string> m_outputName{this, "OutputName", "logits"};
    float m_cosMin{0.f};

    /// Node feature order expected by the model metadata (resolved at initialize).
    std::vector<std::string> m_nodeFeatureNames{};
    std::vector<SegmentNodeFeatureId> m_nodeFeatureIds{};
  };
}
#endif
