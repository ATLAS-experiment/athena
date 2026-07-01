/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERFACES_ISEGMENTEDGECLASSIFIERTOOL_H
#define MUONINFERENCEINTERFACES_ISEGMENTEDGECLASSIFIERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "MuonInferenceInterfaces/SegmentEdgeData.h"
#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonML {

  /**
   * @class ISegmentEdgeClassifierTool
   * @brief Interface for segment-edge GNN classification.
   *
   * This tool builds a graph representation of muon segments and classifies
   * edges (potential segment pairs) using a graph neural network. Segments
   * become nodes; edges connect nearby segments with compatible kinematics.
   *
   * **Workflow:**
   * 1. `buildGraph()` — Constructs node features from segment position/direction
   *    and bucket metadata, then builds sparse edges based on sector proximity
   *    and angular compatibility.
   * 2. `classifyEdges()` — Runs ONNX inference on the graph to score each edge
   *    as a potential track connection and returns edge predictions.
   *
   * **Data Flow:**
   * - **Input:** xAOD::MuonSegmentContainer (raw segment collection)
   * - **Intermediate:** SegmentEdgeGraph (nodes with features, sparse edge indices/features)
   * - **Output:** std::vector<SegmentEdgeScore> (edge probabilities)
   */
  class ISegmentEdgeClassifierTool : virtual public IAlgTool {
  public:
    virtual ~ISegmentEdgeClassifierTool() = default;
    DeclareInterfaceID(ISegmentEdgeClassifierTool, 1, 0);

    /// Build a GNN graph from segments, extracting node features and computing
    /// edge connections based on kinematic compatibility. Writes the graph
    /// structure (nodes, edges, and features) to @p graph.
    virtual StatusCode buildGraph(const EventContext& ctx,
                                  const xAOD::MuonSegmentContainer& segments,
                                  SegmentEdgeGraph& graph) const = 0;

    /// Run ONNX inference on the graph and return edge classification scores.
    /// Reads the prepared @p graph and populates @p scores with logit and
    /// probability for each edge; the scores indicate the likelihood of a
    /// valid track connection between the two endpoint segments.
    virtual StatusCode classifyEdges(const EventContext& ctx,
                                     const SegmentEdgeGraph& graph,
                                     std::vector<SegmentEdgeScore>& scores) const = 0;
  };
}
#endif
