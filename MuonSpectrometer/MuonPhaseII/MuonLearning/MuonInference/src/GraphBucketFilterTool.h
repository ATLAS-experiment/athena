#ifndef MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H
#define MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H

#include "BucketInferenceToolBase.h"
#include "StoreGate/WriteHandleKey.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace MuonML {

/**
 * @class GraphBucketFilterTool
 * @brief Runs GNN-based bucket-level inference and optionally writes filtered
 *        space-point buckets to the event store.
 *
 * The tool reads a SpacePointContainer, builds node features and a sparse edge
 * index, runs an ONNX model, and classifies each bucket as keep or reject.
 * Two ONNX output layouts are supported:
 *   - MultiClass3: [N, 3] logits — bucket class is determined by argmax.
 *   - SingleOutput: [N] or [N, 1] scalar score — bucket is kept when
 *     score (or sigmoid(score)) exceeds ScoreThreshold.
 *
 * When WriteSpacePointKey is non-empty the tool operates in filtering mode and
 * writes the accepted buckets to StoreGate.  If the key is empty it runs in
 * inference-only mode (useful for score dumping / efficiency studies).
 *
 * Optional label printing (PrintLabels) reproduces the training label
 * definition at reconstruction time so that per-event efficiency and fake rate
 * can be monitored from the log without a separate truth-matching step.
 */
class GraphBucketFilterTool : public BucketInferenceToolBase {
public:
  using BucketInferenceToolBase::BucketInferenceToolBase;
  ~GraphBucketFilterTool() override = default;

  /// Retrieve sub-tools, initialise StoreGate keys and validate configuration.
  StatusCode initialize() override final;

  /// Build the GNN graph for @p ctx and classify each bucket; optionally write
  /// accepted buckets to the store.
  StatusCode runGraphInference(const EventContext& ctx, GraphRawData& graphData) const override final;

private:
  /// ONNX output layout used for bucket selection.
  enum class OutputMode { MultiClass3, SingleOutput };

  /// Per-bucket truth/segment label information reconstructed at runtime to
  /// mirror the training label definition: label = hasTruth || hasSegment.
  struct BucketLabelInfo {
    int label{-1};        ///< Combined label (1 = good, 0 = background, -1 = unknown).
    int hasTruth{-1};     ///< 1 if any space-point in the bucket is truth-matched, else 0; -1 if unavailable.
    int hasSegment{-1};   ///< 1 if at least one reconstructed segment points to this bucket, else 0; -1 if unavailable.
    unsigned int nSegments{0}; ///< Number of segments pointing to this bucket.
  };

  /// Aggregates all per-event scalar and vector quantities passed to
  /// dumpDebugEvent.  Grouping avoids a long parameter list that would
  /// otherwise hurt readability and make future extensions error-prone.
  struct DebugDumpEventData {
    std::string outputMode;           ///< Human-readable output-mode tag ("multiclass3" or "single_output").
    std::size_t inputBucketCount{0};  ///< Total number of buckets in the input container.
    std::size_t validBuckets{0};      ///< Buckets with bucket_size > 0 that were sent to ONNX.
    std::size_t predictionsConsumed{0}; ///< Number of ONNX predictions consumed (== validBuckets on success).
    std::size_t kept{0};              ///< Total buckets written to the output container.
    std::vector<int> selectedClasses; ///< Argmax class (or 0/1) for each valid bucket.
    std::vector<int> keepFlags;       ///< 1 if the bucket was accepted, 0 otherwise; one entry per valid bucket.
    std::vector<int> labels;          ///< Training label for each valid bucket (populated when PrintLabels=true).
    std::vector<int> hasTruthFlags;   ///< hasTruth field for each valid bucket (populated when PrintLabels=true).
    std::vector<int> hasSegmentFlags; ///< hasSegment field for each valid bucket (populated when PrintLabels=true).
  };

  /// Map from bucket pointer to number of reconstructed segments pointing to it.
  using SegmentCountMap = std::unordered_map<const MuonR4::SpacePointBucket*, unsigned int>;

  /// Populate @p segmentCounts by iterating over the segment container
  /// identified by LabelSegmentKey.  Does nothing when PrintLabels is false or
  /// LabelSegmentKey is empty.
  StatusCode buildSegmentCountMap(const EventContext& ctx, SegmentCountMap& segmentCounts) const;

  /// Compute the training label for @p bucket using truth-matching (via
  /// LabelVisualizationTool) and/or segment counting (via @p segmentCounts).
  /// Returns a BucketLabelInfo with all available fields filled in.
  BucketLabelInfo computeBucketLabel(const MuonR4::SpacePointBucket& bucket,
                                     const SegmentCountMap& segmentCounts) const;

  /// Append one JSONL record to DebugDumpFile containing the raw ONNX tensors
  /// and per-bucket decisions from @p eventData.  Thread-safe; protected by
  /// m_debugDumpMutex.  Respects the DebugDumpMaxEvents limit.
  StatusCode dumpDebugEvent(const EventContext& ctx,
                            const GraphRawData& graphData,
                            const Ort::Value& outTensor,
                            const DebugDumpEventData& eventData) const;

  /// Output: buckets that pass the class selection
  SG::WriteHandleKey<MuonR4::SpacePointContainer> m_writeKey{
      this, "WriteSpacePointKey", "FilteredMlBuckets"};

  /// Keep these classes (argmax ∈ AcceptClasses)
  /// Score threshold used when model output is single-output [N] or [N,1]
  /// Keep bucket if score > ScoreThreshold
  Gaudi::Property<double> m_scoreThreshold{this, "ScoreThreshold", 0.0};
  Gaudi::Property<bool> m_singleOutputIsLogit{this, "SingleOutputIsLogit", true, 
    "If true, apply sigmoid to single-output ONNX values before ScoreThreshold"};

  /// Keep these classes (argmax ∈ AcceptClasses)
  Gaudi::Property<std::vector<int>> m_acceptClasses{this, "AcceptClasses", {1, 2}};

  /// Bias applied to class 0: logits += [-BiasClass0, 0, 0]
  Gaudi::Property<double> m_biasClass0{this, "BiasClass0", 2.71484375};

  /// Optional JSONL file containing the exact Athena-side ONNX inputs/outputs.
  /// Empty string disables file dumping.
  Gaudi::Property<std::string> m_debugDumpFile{this, "DebugDumpFile", ""};

  /// Maximum number of events to write to DebugDumpFile. 0 means no limit.
  Gaudi::Property<unsigned int> m_debugDumpMaxEvents{this, "DebugDumpMaxEvents", 0};

  /// Print per-event bucket-label performance using the same label definition as training:
  /// label = hasTruth || (bucket_segments > 0).
  Gaudi::Property<bool> m_printLabels{this, "PrintLabels", false};

  /// Print detailed per-bucket label/decision lines for the first N buckets per event.
  /// 0 disables per-bucket lines while keeping the per-event performance summary.
  Gaudi::Property<unsigned int> m_labelPrintFirstNBuckets{this, "LabelPrintFirstNBuckets", 20};

  /// Optional segment container used for the bucket_segments > 0 part of the training label.
  /// Leave empty when segments are not available before bucket inference.
  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_labelSegmentKey{this, "LabelSegmentKey", ""};

  /// Optional visualization tool used for the bucket_hasTruth part of the training label.
  ToolHandle<MuonValR4::IPatternVisualizationTool> m_labelVisualizationTool{
      this, "LabelVisualizationTool", ""};

  mutable std::mutex m_debugDumpMutex;
  mutable std::atomic<unsigned int> m_debugDumpEvents{0};

};

} // namespace MuonML

#endif
