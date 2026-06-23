/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTTRACKCANDIDATEBUILDERTOOL_H
#define MUONINFERENCE_SEGMENTTRACKCANDIDATEBUILDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonInferenceInterfaces/ISegmentTrackCandidateBuilderTool.h"
#include "Gaudi/Property.h"

namespace MuonML {
  class SegmentTrackCandidateBuilderTool final : public extends<AthAlgTool, ISegmentTrackCandidateBuilderTool> {
  public:
    using base_class::base_class;
    StatusCode buildCandidates(const EventContext& ctx,
                               const SegmentEdgeGraph& graph,
                               const std::vector<SegmentEdgeScore>& scores,
                               std::vector<std::vector<unsigned>>& candidateIdsPerSegment) const override;
  private:
    Gaudi::Property<float> m_edgeThreshold{
      this, "EdgeThreshold", 0.25f,
      "Loose edge probability threshold used for recall/recovery components"};

    Gaudi::Property<float> m_overlapThreshold{
      this, "OverlapThreshold", 0.8f,
      "High-purity edge probability threshold used to build core components"};

    Gaudi::Property<bool> m_useRecoveryComponents{
      this, "UseRecoveryComponents", true,
      "Add additional low-threshold connected components to recover true candidates missed by the high-purity core"};

    Gaudi::Property<bool> m_symmetrizeDirectedEdges{
      this, "SymmetrizeDirectedEdges", true,
      "Use max(score i->j, score j->i) for undirected segment association"};

    Gaudi::Property<bool> m_addAllSegmentsRecoveryCandidate{
      this, "AddAllSegmentsRecoveryCandidate", false,
      "Add one final candidate containing all graph nodes; use only for no-loss validation/debugging"};

    Gaudi::Property<bool> m_keepIsolatedSegments{this, "KeepIsolatedSegments", false};
    Gaudi::Property<unsigned> m_minCandidateSize{this, "MinCandidateSize", 2};
  };
}
#endif
