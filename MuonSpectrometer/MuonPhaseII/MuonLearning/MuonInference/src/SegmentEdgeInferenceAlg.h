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
#include <unordered_set>
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

    /// MC-only: given the per-muon component memberships and mixed-status
    /// gathered while the per-component classify every truth muon's final
    //  grouping (clean/mixed/fragmented).
    void classifyGroupPurity(
        const std::unordered_map<std::int32_t, std::unordered_set<unsigned>>& muonComponents,
        const std::unordered_map<std::int32_t, bool>& muonEverMixed,
        const std::unordered_map<std::int32_t, double>& truthIdToAbsEta) const;

    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MsSegments"};
    /// Per-segment payload consumed by MlMsTrackSeeder: [componentId, anchorRank + 1]
    /// Empty means that the segment was rejected by the ML preselection.
    SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_pairGateDecorKey{
        this, "PairGateDecoration", "MsSegments.mlTrackComponent"};
    /// Per-segment ML scores for MlMsTrackSeeder: [strongest selected cross-layer edge of the
    /// segment's component, strongest selected cross-layer edge of the segment itself]
    /// (-1 if none). Written for the segments of kept components.
    SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_componentScoreDecorKey{
        this, "ComponentScoreDecoration", "MsSegments.mlComponentScore"};
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
    Gaudi::Property<unsigned int> m_maxEdgesPerNodePerLayer{
        this, "MaxEdgesPerNodePerLayer", 0,
        "If > 0, each segment nominates its this-many highest-score edges towards EACH layer (Inner, "
        "Middle, Outer, Extended, ...) instead of its MaxEdgesPerNode best edges overall; zero keeps "
        "the global top-K"};
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
        "Restrict seed anchors to inner segment(s); with SeedAnchorsPerComponent > 0 keep only "
        "the inner segment(s) with the strongest selected edge towards another layer"};
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
    /// True cross-layer edges above PairGateThreshold, total and kept by the selection; barrel / endcap
    /// Every element is a std::atomic, updated only with atomic increments.
    mutable std::array<std::atomic<std::size_t>, 2> m_sumTrueCrossLayerEdges ATLAS_THREAD_SAFE{};
    mutable std::array<std::atomic<std::size_t>, 2> m_sumTrueCrossLayerSelected ATLAS_THREAD_SAFE{};
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

    /// Job-summed grouping-purity diagnostics: does a formed (kept) component
    /// mix segments from more than one truth muon, and does each truth muon's
    /// retained segments end up in one clean component, a component shared
    /// with another muon, or split across several components? 
    struct GroupPurityCounters {
      static constexpr std::size_t kSizeBins = 6;  // 2, 3, 4, 5-9, 10-19, 20+
      static constexpr std::size_t kColumns = 4;   // all, barrel, transition, endcap
      static constexpr std::size_t kScoreBins = 5; // <0.05, 0.05-0.1, 0.1-0.3, 0.3-0.5, >=0.5
      static constexpr std::size_t kLayerBins = 4; // 1, 2, 3, 4+ distinct layers
      std::array<std::atomic<std::size_t>, 3> componentsByOutcome{};  //!< clean / mixed / bkgOnly
      std::array<std::atomic<std::size_t>, kSizeBins> componentSizeHist{};
      /// Kept components split by outcome (clean / mixed / bkgOnly), index = outcome * nBins + bin
      std::array<std::atomic<std::size_t>, 3 * kScoreBins> scoreByOutcome{};   //!< strongest selected edge
      std::array<std::atomic<std::size_t>, 3 * kSizeBins> sizeByOutcome{};     //!< retained segments
      std::array<std::atomic<std::size_t>, 3 * kLayerBins> layersByOutcome{};  //!< distinct LayerIndex
      /// Components spanning >= 2 layers (the only ones that can seed), per outcome:
      static constexpr std::size_t kCrossScoreBins = 6;  // <0.05, 0.05-0.1, 0.1-0.3, 0.3-0.5, 0.5-0.8, >=0.8
      static constexpr std::size_t kCrossEdgeBins = 4;   // 0, 1, 2, 3+ selected cross-layer edges
      std::array<std::atomic<std::size_t>, 3 * kCrossScoreBins> crossScoreByOutcome{};  //!< strongest cross-layer edge
      std::array<std::atomic<std::size_t>, 3 * kCrossEdgeBins> crossEdgesByOutcome{};   //!< number of cross-layer edges
      std::atomic<std::size_t> weakCleanWithOther{0};
      std::atomic<std::size_t> weakCleanAlone{0};
      std::atomic<std::size_t> mixedComponentSegments{0};
      std::array<std::atomic<std::size_t>, kColumns * 3> muonOutcome{};  //!< clean/mixed/fragmented, per column
      std::atomic<std::uint64_t> worstMixedPacked{0};
    };
    /// Every element is a std::atomic, updated only with atomic additions/CAS.
    mutable GroupPurityCounters m_groupPurity ATLAS_THREAD_SAFE;
  };
}
#endif
