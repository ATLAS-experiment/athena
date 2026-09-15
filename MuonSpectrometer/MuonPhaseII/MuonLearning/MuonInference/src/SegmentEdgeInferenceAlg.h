/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H
#define MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H

#include "AthContainers/ConstDataVector.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "MuonInferenceInterfaces/ISegmentEdgeClassifierTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonML {
  class SegmentEdgeInferenceAlg final : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;
  private:
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
    /**
     * Per-segment payload consumed by MlMsTrackSeeder:
     *   [componentId, isSeedAnchor]
     * Empty means that the segment was rejected by the ML preselection.
     * The anchor bit nominates an ML-supported segment that launches
     * direct component seeding.
     */
    SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_pairGateDecorKey{
        this, "PairGateDecoration", "MuonSegmentsFromR4.mlTrackComponent"};
    SG::WriteHandleKey<ConstDataVector<xAOD::MuonSegmentContainer>> m_filteredSegmentKey{
        this, "FilteredSegmentKey", "",
        "Optional VIEW container containing only segments incident to a "
        "post-classifier selected edge; empty disables the output"};
    Gaudi::Property<float> m_pairGateThreshold{
        this, "PairGateThreshold", 0.5f,
        "Minimum edge probability used to form ML track components"};
    Gaudi::Property<unsigned int> m_maxEdgesPerNode{
        this, "MaxEdgesPerNode", 2,
        "Keep at most this many highest-score neighbours per segment when forming the ML path graph; zero keeps all"};
    Gaudi::Property<bool> m_useDegreeCappedComponents{
        this, "UseDegreeCappedComponents", false,
        "Use a global greedy degree cap instead of the mutual top-K path graph"};
    Gaudi::Property<bool> m_requireMutualTopKEdges{
        this, "RequireMutualTopKEdges", true,
        "Retain an ML association only when both endpoint segments rank it in their top MaxEdgesPerNode scores"};
    Gaudi::Property<bool> m_recoverOrphanNodes{
        this, "RecoverOrphanNodes", true,
        "For a thresholded node without a mutual top-K edge, retain its best one-sided top-K association"};
    Gaudi::Property<unsigned int> m_seedAnchorsPerComponent{
        this, "SeedAnchorsPerComponent", 0,
        "Number of highest-score segments that may seed each ML component; zero keeps every retained segment"};
    Gaudi::Property<bool> m_anchorInnermostLayer{
        this, "AnchorInnermostLayer", false,
        "Restrict seed anchors to inner segment(s)."};
    Gaudi::Property<bool> m_keepBestSegmentPerChamber{
        this, "KeepBestSegmentPerChamber", true,
        "Keep only the ML-best segment in each chamber within a component"};
    Gaudi::Property<unsigned int> m_minSegmentsPerComponent{
        this, "MinSegmentsPerComponent", 2,
        "Discard ML components with fewer retained segments"};
    ToolHandle<ISegmentEdgeClassifierTool> m_edgeClassifier{this, "EdgeClassifierTool", "MuonML::SegmentEdgeClassifierTool/SegmentEdgeClassifierTool"};
  };
}
#endif
