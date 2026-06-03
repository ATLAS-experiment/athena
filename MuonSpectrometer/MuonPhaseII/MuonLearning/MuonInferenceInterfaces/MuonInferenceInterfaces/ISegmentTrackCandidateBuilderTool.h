/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERFACES_ISEGMENTTRACKCANDIDATEBUILDERTOOL_H
#define MUONINFERENCEINTERFACES_ISEGMENTTRACKCANDIDATEBUILDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "MuonInferenceInterfaces/SegmentEdgeData.h"

namespace MuonML {
  /**
   * @class ISegmentTrackCandidateBuilderTool
   * @brief Interface for building track candidates from classified segment edges.
   *
   * This tool reads a classified segment-edge graph and converts high-confidence
   * edges into track candidates. Each candidate represents a potential muon track
   * as a sequence of segments connected by edges with high classification score.
   *
   * Typical usage:
   * 1. `ISegmentEdgeClassifierTool::buildGraph()` — Create the graph.
   * 2. `ISegmentEdgeClassifierTool::classifyEdges()` — Score all edges.
   * 3. `buildCandidates()` — Extract track candidates from high-score edges.
   *
   * **Data Flow:**
   * - **Input:** SegmentEdgeGraph (structure and node/edge features),
   *   std::vector<SegmentEdgeScore> (edge probabilities from classifier)
   * - **Output:** std::vector<std::vector<unsigned>> where each inner vector
   *   is a sequence of segment indices forming a candidate track.
   */
  class ISegmentTrackCandidateBuilderTool : virtual public IAlgTool {
  public:
    virtual ~ISegmentTrackCandidateBuilderTool() = default;
    DeclareInterfaceID(ISegmentTrackCandidateBuilderTool, 1, 0);

    /// Build track candidates by extracting high-confidence edge paths from
    /// the classified graph. Each candidate is a sequence of segment indices
    /// that together form a potential track. Populates @p candidateIdsPerSegment
    /// where each inner vector contains the indices of segments in that candidate
    /// track, in order.
    virtual StatusCode buildCandidates(const EventContext& ctx,
                                       const SegmentEdgeGraph& graph,
                                       const std::vector<SegmentEdgeScore>& scores,
                                       std::vector<std::vector<unsigned>>& candidateIdsPerSegment) const = 0;
  };
}
#endif
