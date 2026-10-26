/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGALGS_MLMSTRACKSEEDER_H
#define MUONTRACKFINDINGALGS_MLMSTRACKSEEDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingDiagnosticsTool.h"
#include "ActsEvent/ContextUtility.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <span>
#include <string>

namespace MuonR4 {

/** @brief ITrackSeedingTool implementation which constructs seeds directly
 *         from sufficiently large ML-nominated segment components.
 *
 * For each component anchor, the seed is built from the component segments that
 * are direction-compatible with the anchor and that project, onto the standard
 * seeder's cylinder, within PointingWindow of the window centre (by default the
 * segment around which most layers cluster). One segment per layer is kept,
 * chosen by segment quality. 2-segment seeds additionally need a strong ML
 * cross-layer association in their component.
 *
 * The wrapped conventional tool is used only for initial-parameter estimation and
 * the cylinder projection; it never constructs seeds for this tool. Consequently
 * MsTrackFindingAlg sees only the standard ITrackSeedingTool interface and selects
 * this implementation entirely through job configuration.
 */
class MlMsTrackSeeder final :
    public extends<AthAlgTool, ITrackSeedingTool> {
 public:
  using base_class::base_class;

  StatusCode initialize() override final;
  /// Prints the job-summed seeding counters (DEBUG only)
  StatusCode finalize() override final;

  /** @copydoc ITrackSeedingTool::findTrackSeeds */
  StatusCode findTrackSeeds(const EventContext& ctx,
                            std::vector<MsTrackSeed>& outSeeds) const override final;

  /** @copydoc ITrackSeedingTool::estimateStartParameters */
  Acts::Result<Acts::BoundTrackParameters> estimateStartParameters(
      const EventContext& ctx,
      const MsTrackSeed& seed) const override final;

 private:
  ToolHandle<ITrackSeedingDiagnosticsTool> m_seedParameterEstimator{
      this, "BaselineSeeder", "",
      "Standard seeder: initial-parameter estimates for ML-built seeds and the projection "
      "onto its seeding cylinder used by PointingWindow"};
  ActsTrk::ContextUtility m_ctxProvider{this};
  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{
      this, "SegmentContainer", "MsSegments"};
  SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_candidateDecorKey{
      this, "CandidateDecoration", m_segmentKey, "trackCandidateIds",
      "Scheduler dependency on the ML candidate decoration"};
  SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_componentScoreKey{
      this, "ComponentScoreDecoration", m_segmentKey, "mlComponentScore",
      "ML scores per segment: [component's strongest cross-layer edge, segment's strongest cross-layer edge]"};
  Gaudi::Property<unsigned int> m_minSegmentsPerCandidate{
      this, "MinSegmentsPerCandidate", 2};
  Gaudi::Property<unsigned int> m_maxSegmentsPerCandidate{
      this, "MaxSegmentsPerCandidate", 0,
      "Discard ML components with more than this many segments; zero disables the cap"};
  Gaudi::Property<unsigned int> m_minLayers{
      this, "MinLayers", 2, "Minimum number of distinct muon layers a seed must span"};
  Gaudi::Property<double> m_minCosConsistency{
      this, "MinCosConsistency", 0.819,  // cos(35 deg), matching SegmentEdgeClassifierTool's default MaxDeltaThetaDeg
      "Minimum cos(angle) between a segment's direction and its seed anchor's "
      "direction for the segment to be included in the seed"};
  Gaudi::Property<double> m_pointingWindow{
      this, "PointingWindow", 250.,
      "Keep only seed segments whose position projected onto the standard seeder's cylinder "
      "lies within this distance [mm] of the window centre (the standard seeder uses "
      "SeedHalfLength = 250 mm); zero disables"};
  Gaudi::Property<bool> m_clusterWindowCentre{
      this, "ClusterWindowCentre", true,
      "PointingWindow: centre the window on the segment whose window covers the most distinct "
      "layers (ties: segment quality) instead of the anchor"};
  Gaudi::Property<unsigned int> m_extraClusterSeeds{
      this, "ExtraClusterSeeds", 2,
      "PointingWindow: after the first seed of an anchor, build up to this many further window seeds "
      "from the component segments not covered by an earlier window; zero disables"};
  Gaudi::Property<double> m_extraClusterMinScore{
      this, "ExtraClusterMinScore", 0.8,
      "ExtraClusterSeeds: an extra seed needs a segment whose own strongest selected cross-layer ML "
      "edge (ComponentScoreDecoration) reaches this value"};
  Gaudi::Property<double> m_innerPointingMaxDeg{
      this, "InnerPointingMaxDeg", 2.,
      "Remove from the seed pools the endcap inner segments whose |theta(direction) - theta(position)| "
      "exceeds this value [deg], i.e. that do not point back to the IP; zero disables"};
  Gaudi::Property<double> m_innerRescueWindow{
      this, "InnerRescueWindow", 0.,
      "After the MinLayers check and the 2-segment rule: fill a missing Inner layer with the closest "
      "IP-pointing (InnerRescueMaxDeg), direction-compatible inner segment of any component that "
      "projects within this distance [mm] of the window centre; zero disables"};
  Gaudi::Property<double> m_innerRescueMaxDeg{
      this, "InnerRescueMaxDeg", 1.,
      "InnerRescueWindow: maximum |theta(direction) - theta(position)| [deg] of the filled segment"};
  Gaudi::Property<bool> m_variantSeeds{
      this, "VariantSeeds", true,
      "PointingWindow: after a seed passing the Variant* gates, emit a second seed in which every "
      "layer takes its runner-up segment from the window (or, without any alternative, the weakest "
      "non-inner member is dropped), as a fallback if the fit of the first seed fails"};
  Gaudi::Property<unsigned int> m_variantMinLayers{
      this, "VariantMinLayers", 3,
      "VariantSeeds: minimum number of station layers of the seed"};
  Gaudi::Property<double> m_variantMaxPointingDeg{
      this, "VariantMaxPointingDeg", 1.,
      "VariantSeeds: endcap seeds need an inner segment with |theta(dir) - theta(pos)| below this [deg]"};
  Gaudi::Property<double> m_variantMinScore{
      this, "VariantMinScore", 0.8,
      "VariantSeeds: minimum strongest cross-layer ML edge of the seed's component (ComponentScoreDecoration); "
      "zero disables"};
  Gaudi::Property<double> m_twoSegMinScore{
      this, "TwoSegmentMinComponentScore", 0.3,
      "Keep a 2-segment seed only if its component's strongest selected cross-layer edge "
      "(ComponentScoreDecoration) reaches this value; zero disables"};
  ToolHandle<ISegmentSelectionTool> m_segSelector{
      this, "SegmentSelectionTool", "",
      "Seeding-quality check of the standard seeder, used by the truth diagnostics"};
  Gaudi::Property<bool> m_truthDiag{
      this, "EnableTruthDiagnostics", false,
      "MC only, and only at DEBUG level: truth-classify the seeds and time their fits"};

  /// Job-summed counters, only touched when the tool runs at DEBUG level.
  /// Every element is a std::atomic, updated only with atomic additions.
  struct SeedingCounters {
    std::atomic<std::size_t> events{0};
    std::atomic<std::size_t> components{0};
    std::atomic<std::size_t> rejectedTooSmall{0};
    std::atomic<std::size_t> rejectedTooLarge{0};
    std::atomic<std::size_t> rejectedNoAnchor{0};
    std::atomic<std::size_t> acceptedComponents{0};
    std::atomic<std::size_t> anchorsTooFewLayers{0};     //!< anchor yielded < MinLayers layers
    std::atomic<std::size_t> windowCentreNotAnchor{0};   //!< PointingWindow: centred on another segment than the anchor
    std::atomic<std::size_t> windowCentreOutOfBounds{0}; //!< PointingWindow: centre outside the cylinder, window not applied
    std::atomic<std::size_t> windowDropped{0};           //!< PointingWindow: segments further than the window
    std::atomic<std::size_t> windowOutOfBounds{0};       //!< PointingWindow: segments projecting outside the cylinder
    std::atomic<std::size_t> twoSegLowScore{0};          //!< TwoSegmentMinComponentScore: seeds dropped
    std::atomic<std::size_t> twoSegNoScore{0};           //!< 2-segment seeds without a score (kept)
    std::atomic<std::size_t> extraSeeds{0};              //!< ExtraClusterSeeds: seeds built
    std::atomic<std::size_t> extraNoStrong{0};           //!< ExtraClusterSeeds: clusters without a strong segment
    std::atomic<std::size_t> pointingRemoved{0};         //!< InnerPointingMaxDeg: segments removed (first clusters)
    std::atomic<std::size_t> pointingCentreSkipped{0};   //!< InnerPointingMaxDeg: clusters skipped
    std::atomic<std::size_t> innerRescued{0};            //!< InnerRescueWindow: inner layers filled
    std::atomic<std::size_t> variantSeeds{0};            //!< VariantSeeds: seeds built
    std::atomic<std::size_t> variantNone{0};             //!< VariantSeeds: gated seeds without a different variant
    std::atomic<std::size_t> seeds{0};
    std::array<std::atomic<std::size_t>, 4> seedSegments{};  //!< 2 / 3 / 4 / 5+ segments
    std::atomic<std::size_t> startParCalls{0};
    std::atomic<std::size_t> startParFailures{0};
    std::atomic<std::uint64_t> findSeedsNs{0};   //!< wall time spent in findTrackSeeds
    std::atomic<std::uint64_t> startParNs{0};    //!< wall time spent in estimateStartParameters
  };
  mutable SeedingCounters m_counters ATLAS_THREAD_SAFE;

  /// Truth diagnostics (EnableTruthDiagnostics && DEBUG) on the seeds that are handed to
  /// MsTrackFindingAlg. Classes: 0 pure (all segments from one truth particle), 1 partial
  /// (>= 2 share a particle), 2 junk without any truth-matched segment, 3 junk mixing particles.
  struct SeedAnatomy {
    static constexpr std::size_t kClasses = 4;
    /// D1: wall time between consecutive estimateStartParameters calls, i.e. the fit of the
    /// previous seed in MsTrackFindingAlg; [class][start parameters ok]
    std::array<std::atomic<std::uint64_t>, kClasses * 2> fitNs{};
    std::array<std::atomic<std::size_t>, kClasses * 2> fitCount{};
    std::atomic<std::size_t> untimedLastSeeds{0};   //!< last seed of an event: no following call
    /// D3: descriptors of the fitted seeds by class
    std::array<std::atomic<std::size_t>, kClasses * 3> nSegments{};     //!< 2 / 3 / 4+
    std::array<std::atomic<std::size_t>, kClasses * 4> thetaSpread{};   //!< <2 / 2-4 / 4-8 / >=8 deg
    /// D5: momentum estimate of the start parameters, <3 / 3-6 / 6-10 / 10-20 / >=20 GeV
    static constexpr std::size_t kPBins = 5;
    std::array<std::atomic<std::size_t>, kClasses * kPBins> seedsByP{};      //!< seeds with valid start parameters
    std::array<std::atomic<std::uint64_t>, kClasses * kPBins> fitNsByP{};    //!< D1 fit time by momentum
    std::array<std::atomic<std::size_t>, kClasses * kPBins> fitCountByP{};
    std::array<std::atomic<std::size_t>, 2 * 2> fitCountBySegs{};           //!< [pure, junk][2, 3+ segments]
    std::array<std::atomic<std::uint64_t>, 2 * 2> fitNsBySegs{};
    /// D7: pure seeds whose start parameters fail: [0 no segment passes the seeding quality,
    /// 1 some passes] and [0 without, 1 with an NSW segment]
    std::array<std::atomic<std::size_t>, 2> pureNoStartParsQuality{};
    std::array<std::atomic<std::size_t>, 2> pureNoStartParsNsw{};
    std::atomic<std::size_t> pureNoStartPars{0};

    /// D9: fitted (valid start parameters) 2-segment seeds; groups [0 pure, 1 partial, 2 junk]
    static constexpr std::size_t kGroups = 3;
    static constexpr std::size_t kDistBins = 4;   // projected distance <50 / 50-100 / 100-250 / >=250 or n.a. [mm]
    static constexpr std::size_t kQualBins = 3;   // 0 / 1 / 2 segments passing the seeding quality
    static constexpr std::size_t kScoreBins = 5;  // <0.05 / 0.05-0.3 / 0.3-0.8 / >=0.8 / n.a.
    static constexpr std::size_t kPairBins = 5;   // layer pair: I-M / M-O / I-O / with BEE-EE / other
    std::array<std::atomic<std::size_t>, kGroups * kDistBins> twoSegDistN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kDistBins> twoSegDistNs{};
    std::array<std::atomic<std::size_t>, kGroups * kQualBins> twoSegQualN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kQualBins> twoSegQualNs{};
    std::array<std::atomic<std::size_t>, kGroups * kScoreBins> twoSegCompScoreN{};   //!< component's best cross-layer edge
    std::array<std::atomic<std::uint64_t>, kGroups * kScoreBins> twoSegCompScoreNs{};
    std::array<std::atomic<std::size_t>, kGroups * kScoreBins> twoSegPairScoreN{};   //!< weaker of the two segments' best cross-layer edges
    std::array<std::atomic<std::uint64_t>, kGroups * kScoreBins> twoSegPairScoreNs{};
    std::array<std::atomic<std::size_t>, kGroups * kPairBins> twoSegLayersN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kPairBins> twoSegLayersNs{};
    std::array<std::atomic<std::size_t>, kGroups * 2> twoSegRegionN{};               //!< barrel / endcap seed
    std::array<std::atomic<std::uint64_t>, kGroups * 2> twoSegRegionNs{};
    /// D9b: joint table of the fitted 2-segment seeds: [group][distance bin * kScoreBins + score bin]
    std::array<std::atomic<std::size_t>, kGroups * kDistBins * kScoreBins> twoSegJointN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kDistBins * kScoreBins> twoSegJointNs{};
    /// D10: all fitted seeds by number of measurements <10 / 10-15 / 15-20 / 20-30 / >=30
    static constexpr std::size_t kMeasBins = 5;
    std::array<std::atomic<std::size_t>, kGroups * kMeasBins> measN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kMeasBins> measNs{};
    /// 2-segment rule: seeds dropped by the score cut per group
    std::array<std::atomic<std::size_t>, kGroups> twoSegDropped{};
    /// D13: fitted seeds by IP pointing of the inner segment, |theta(dir) - theta(pos)|
    /// <1 / 1-2 / 2-5 / 5-10 / >=10 deg / no inner segment; [group][2-seg barrel, 2-seg endcap,
    /// >=3-seg barrel, >=3-seg endcap][bin]
    static constexpr std::size_t kPointBins = 6;
    static constexpr std::size_t kPointCats = 4;
    std::array<std::atomic<std::size_t>, kGroups * kPointCats * kPointBins> pointN{};
    std::array<std::atomic<std::uint64_t>, kGroups * kPointCats * kPointBins> pointNs{};
    /// ExtraClusterSeeds: extra seeds pure / partial / junk
    std::array<std::atomic<std::size_t>, 3> extraSeedClass{};
    /// D11: pure 2-segment seeds: [0 the owner also has a >= 3-segment pure/partial seed, 1 it has not]
    std::array<std::atomic<std::size_t>, 2> pureTwoSegOwnerHasLonger{};
  };
  mutable SeedAnatomy m_anatomy ATLAS_THREAD_SAFE;

  /// PointingWindow truth diagnostics (EnableTruthDiagnostics && DEBUG)
  struct WindowCounters {
    static constexpr std::size_t kDistBins = 6;  // <100 / 100-250 / 250-500 / 500-1000 / >=1000 mm / out of bounds
    /// projected distance to the window centre; [0 same particle as the centre, 1 other / none][bin]
    std::array<std::atomic<std::size_t>, 2 * kDistBins> distance{};
    std::array<std::atomic<std::size_t>, 2> dropped{};   //!< dropped: same particle / other
    std::array<std::atomic<std::size_t>, 3> seedsWithout{};  //!< seeds without window, pure / partial / junk
    std::array<std::atomic<std::size_t>, 3> seedsLost{};     //!< ... of which < MinLayers after the window
    std::array<std::atomic<std::size_t>, 3> seedsPureAfter{}; //!< ... of which the windowed seed is pure
    std::atomic<std::size_t> particlesSeeded{0};   //!< particles with a pure/partial seed without window
    std::atomic<std::size_t> particlesLost{0};     //!< ... of which none with the window
    /// D6 (seeds with a truth owner without window):
    std::atomic<std::size_t> ownerSeeds{0};        //!< pool seeds with an owner particle
    std::atomic<std::size_t> centreOnOwner{0};     //!< ... whose chosen centre belongs to the owner
    std::atomic<std::size_t> oracleKeeps{0};       //!< ... an owner-centred window keeps >= MinLayers owner layers
    std::atomic<std::size_t> actualKeeps{0};       //!< ... the emitted seed holds >= 2 owner segments
  };
  mutable WindowCounters m_window ATLAS_THREAD_SAFE;

  /// D12 seeding funnel per truth muon (EnableTruthDiagnostics && DEBUG, PointingWindow on):
  /// selected truth muons (pT > 20 GeV, |eta| < 2.5, segments in >= 2 layers), by the furthest
  /// stage reached over all ML components, split by truth |eta|.
  struct EtaFunnel {
    static constexpr std::size_t kRegions = 4;      // |eta| <1.05 / 1.05-1.3 / 1.3-2.0 / >=2.0
    /// 0 not in ML / 1 split over components / 2 component rejected / 3 cos cut / 4 window /
    /// 5 layer pick / 6 2-segment score / 7 seeded, imperfect / 8 seeded, clean
    static constexpr std::size_t kStages = 9;
    /// why a muon layer is missing from the stage-7 seed: other component or not in ML /
    /// cos cut / window / centre's layer / quality pick took another segment
    static constexpr std::size_t kMissReasons = 5;
    std::array<std::atomic<std::size_t>, kRegions * kStages> stage{};
    std::array<std::atomic<std::size_t>, kRegions> notSeedable{};  //!< selected muons with segments in < 2 layers
    std::array<std::atomic<std::size_t>, kRegions * kMissReasons> missingLayers{};
    std::array<std::atomic<std::size_t>, kRegions> withMissing{};  //!< stage-7 muons missing >= 1 layer
    std::array<std::atomic<std::size_t>, kRegions * 2> fakes{};     //!< fake seed segments: background / other particle
    std::array<std::atomic<std::size_t>, kRegions> withFake{};      //!< stage-7 muons with >= 1 fake
  };
  mutable EtaFunnel m_etaFunnel ATLAS_THREAD_SAFE;

  /// InnerPointingMaxDeg / InnerRescueWindow truth diagnostics (EnableTruthDiagnostics && DEBUG)
  struct RescueCounters {
    std::array<std::atomic<std::size_t>, 2> pointingRemoved{};  //!< removed segment truth-matched / not
    /// filled segment belongs to [0 the seed's truth owner, 1 another particle or none, 2 seed without owner]
    std::array<std::atomic<std::size_t>, 3> filled{};
  };
  mutable RescueCounters m_rescue ATLAS_THREAD_SAFE;

  /// VariantSeeds truth diagnostics (EnableTruthDiagnostics && DEBUG)
  struct VariantCounters {
    std::array<std::atomic<std::size_t>, 3> seedClass{};  //!< variant seeds pure / partial / junk
    std::atomic<std::size_t> pureMainNotPure{0};          //!< pure variants whose main seed is not pure
  };
  mutable VariantCounters m_variant ATLAS_THREAD_SAFE;

  /// D1 bookkeeping per event slot: the seeds of the current event and the last timed call.
  struct SlotState {
    const MsTrackSeed* base{nullptr};
    std::vector<std::uint8_t> seedClass{};
    std::vector<std::uint8_t> startOk{};
    std::vector<std::uint8_t> pBin{};
    std::size_t lastNSeg{0};
    /// D9/D10/D13 bins per seed: dist, quality, component score, pair score, layer pair, region,
    /// measurements, IP pointing; dist = 255 marks seeds that are not 2-segment seeds
    std::vector<std::array<std::uint8_t, 8>> desc{};
    long lastIndex{-1};
    std::chrono::steady_clock::time_point lastEnd{};
  };
  mutable std::mutex m_slotMutex;
  mutable std::unordered_map<std::size_t, SlotState> m_slots ATLAS_THREAD_SAFE;
};

}  // namespace MuonR4

#endif
