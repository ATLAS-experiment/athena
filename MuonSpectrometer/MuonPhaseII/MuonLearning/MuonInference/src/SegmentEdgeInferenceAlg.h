/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H
#define MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonInferenceInterfaces/ISegmentEdgeClassifierTool.h"
#include "MuonInferenceInterfaces/ISegmentTrackCandidateBuilderTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonML {
  class SegmentEdgeInferenceAlg final : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;
  private:
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
    SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_candidateDecorKey{this, "CandidateDecoration", "MuonSegmentsFromR4.trackCandidateIds"};
    ToolHandle<ISegmentEdgeClassifierTool> m_edgeClassifier{this, "EdgeClassifierTool", "MuonML::SegmentEdgeClassifierTool/SegmentEdgeClassifierTool"};
    ToolHandle<ISegmentTrackCandidateBuilderTool> m_candidateBuilder{this, "CandidateBuilderTool", "MuonML::SegmentTrackCandidateBuilderTool/SegmentTrackCandidateBuilderTool"};
  };
}
#endif
