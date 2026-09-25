/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H
#define MUONINFERENCE_SEGMENTEDGEINFERENCEALG_H

#include "AthContainers/ConstDataVector.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "Gaudi/Property.h"
#include "MuonInferenceInterfaces/ISegmentEdgeClassifierTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace MuonML {
  class SegmentEdgeInferenceAlg final : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

    /// Log of the post-ONNX rank/mutuality and min-component-size selection
    StatusCode finalize() override;
  private:
    /// MC-only: classify every truth-labeled input segment that did not make
    /// it into the filtered container by the stage that removed it. Called only when truth diagnostics are on.
    void classifyLostTruthSegments(
        const xAOD::MuonSegmentContainer& segments,
        const SegmentEdgeGraph& graph,
        const std::unordered_map<std::uint64_t, float>& pairProbability,
        const std::vector<unsigned char>& thresholded,
        const std::vector<std::uint8_t>& keptNode,
        const std::vector<unsigned char>& nodeDropReason) const;

    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
    /**
     * Per-segment payload consumed by MlMsTrackSeeder: [componentId, isSeedAnchor]
     * Empty means that the segment was rejected by the ML preselection.
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
    /**
     * Whether truth-particle matching is available/enabled this job.
     */
    bool m_truthDiagEnabled{false};

    /// Job-summed post-ONNX selection counters (see execute()).
    mutable std::atomic<std::size_t> m_sumGraphNodes{0};
    mutable std::atomic<std::size_t> m_sumThresholdedNodes{0};
    mutable std::atomic<std::size_t> m_sumMutualTopKPairs{0};
    mutable std::atomic<std::size_t> m_sumOneSidedTopKPairs{0};
    mutable std::atomic<std::size_t> m_sumOrphanRecoveryPairs{0};
    mutable std::atomic<std::size_t> m_sumTopologyNodes{0};
    mutable std::atomic<std::size_t> m_sumRetainedNodes{0};
    mutable std::atomic<std::size_t> m_sumChamberSuppressedNodes{0};
    mutable std::atomic<std::size_t> m_sumNodesRejectedByMinComponent{0};
    mutable std::atomic<std::size_t> m_sumRejectedComponents{0};
    mutable std::atomic<std::size_t> m_sumComponentsKept{0};

    // Job-summed truth-vs-background funnel (see execute()); only touched
    // when EnableTruthDiagnostics && msgLvl(MSG::DEBUG). "Truth" segment
    // means getTruthMatchedParticle(seg) != nullptr (SegmentDumperAlg's
    // m_segmentHasTruth criterion). A "true edge" is a candidate pair whose
    // two segments share the same truth-particle index.
    mutable std::atomic<std::size_t> m_sumInputTruthSegments{0};
    mutable std::atomic<std::size_t> m_sumInputBkgSegments{0};
    mutable std::atomic<std::size_t> m_sumOnnxTruthSegments{0};
    mutable std::atomic<std::size_t> m_sumOnnxBkgSegments{0};
    mutable std::atomic<std::size_t> m_sumRetainedTruthSegments{0};
    mutable std::atomic<std::size_t> m_sumRetainedBkgSegments{0};
    mutable std::atomic<std::size_t> m_sumTrueEdgesTotal{0};
    mutable std::atomic<std::size_t> m_sumTrueEdgesPassed{0};
    mutable std::atomic<std::size_t> m_sumBkgEdgesTotal{0};
    mutable std::atomic<std::size_t> m_sumBkgEdgesPassed{0};

    /// Job-summed classification of lost truth segments (classifyLostTruthSegments()).
    /// category[c * kColumns + col]: col 0 = all, 1-3 = barrel / transition /
    /// endcap (truth muon |eta|), 4 = segments of muons that lost seedability.
    struct TruthLossCounters {
      static constexpr std::size_t kCategories = 11;
      static constexpr std::size_t kColumns = 5;
      std::array<std::atomic<std::size_t>, kCategories * kColumns> category{};
      std::array<std::atomic<std::size_t>, 5> scoreBin{};       //!< best true-pair score of category C
      std::array<std::atomic<std::size_t>, 2> sectorDelta{};    //!< same / adjacent sector (C)
      std::array<std::atomic<std::size_t>, 4> layerPair{};      //!< IM / MO / IO / other (C)
      std::array<std::atomic<std::size_t>, 4> muonSegments{};   //!< muon has 2 / 3 / 4 / 5+ input segments (C)
      std::array<std::atomic<std::size_t>, 3> precisionHits{};  //!< nPrecisionHits <=4 / 5-6 / >=7 (C)
      std::array<std::atomic<std::size_t>, 4> muonsSeedable{};  //!< truth muons with >= 2 input chambers (all / regions)
      std::array<std::atomic<std::size_t>, 4> muonsSeedLost{};  //!< ... of which < 2 chambers retained
    };
    /// Every element is a std::atomic, updated only with atomic additions.
    mutable TruthLossCounters m_truthLoss ATLAS_THREAD_SAFE;
    mutable std::atomic<bool> m_warnedMissingToolDiagnostics{false};
  };
}
#endif
