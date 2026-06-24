/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#include "GraphBucketFilterTool.h"
#include "InferenceUtils.h"
#include "BucketGraphUtils.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace MuonML {

StatusCode GraphBucketFilterTool::initialize() {
  // setupModel() already initializes m_readKey and m_geoCtxKey from base class
  ATH_CHECK(setupModel());
  
  ATH_MSG_DEBUG("Base class keys initialized: ReadSpacePoints=" << m_readKey.key() 
                << ", AlignmentKey=" << m_geoCtxKey.key());
  
  // Only initialize write key if it's configured (not empty)
  // This allows the tool to run in inference-only mode for score dumping
  const std::string& writeKey = m_writeKey.key();
  if (!writeKey.empty()) {
    ATH_CHECK(m_writeKey.initialize());
    ATH_MSG_INFO("Filtering mode enabled: writing filtered buckets to " << writeKey);
  } else {
    ATH_MSG_INFO("Inference-only mode enabled: no store writes");
  }

  auto& accept = m_acceptClasses.value();
  std::sort(accept.begin(), accept.end());
  accept.erase(std::unique(accept.begin(), accept.end()), accept.end());

  ATH_CHECK(m_labelSegmentKey.initialize(m_printLabels.value() && !m_labelSegmentKey.key().empty()));
  ATH_CHECK(m_labelVisualizationTool.retrieve(
      EnableTool{m_printLabels.value() && !m_labelVisualizationTool.empty()}));

  if (m_printLabels.value() &&
      m_labelSegmentKey.key().empty() &&
      m_labelVisualizationTool.empty()) {
    ATH_MSG_ERROR("PrintLabels=True requested, but neither LabelVisualizationTool nor "
                  "LabelSegmentKey is configured. Cannot reproduce the training label.");
    return StatusCode::FAILURE;
  }

  if (m_printLabels.value()) {
    ATH_MSG_INFO("Bucket label printing enabled. Label definition: "
                 "label = bucket_hasTruth || (bucket_segments > 0). "
                 "Truth tool configured=" << (!m_labelVisualizationTool.empty() ? "yes" : "no")
                 << ", segment key="
                 << (m_labelSegmentKey.key().empty() ? std::string("<disabled>")
                                                     : m_labelSegmentKey.key()));
  }

  if (!m_debugDumpFile.value().empty()) {
    std::ofstream out(m_debugDumpFile.value(), std::ios::out | std::ios::trunc);
    if (!out) {
      ATH_MSG_ERROR("Could not create bucket inference debug dump file: " << m_debugDumpFile.value());
      return StatusCode::FAILURE;
    }

    nlohmann::ordered_json metadata;
    metadata["record_type"] = "metadata";
    metadata["format_version"] = 2;
    metadata["tool"] = "GraphBucketFilterTool";
    metadata["input_names"] = {"features", "edge_index"};
    metadata["output_name"] = m_outputName.value();
    metadata["feature_names"] = {"bucket_positionX", "bucket_positionY", "bucket_positionZ",
                                   "bucket_layers", "bucket_spacePoints", "bucket_size"};
    metadata["label_definition"] = "bucket_hasTruth || (bucket_segments > 0)";
    metadata["labels_enabled"] = m_printLabels.value();
    metadata["label_segment_key"] = m_labelSegmentKey.key();
    metadata["label_truth_tool_configured"] = !m_labelVisualizationTool.empty();
    metadata["score_threshold"] = m_scoreThreshold.value();
    metadata["bias_class0"] = m_biasClass0.value();
    metadata["accept_classes"] = m_acceptClasses.value();
    out << metadata.dump() << '\n';

    ATH_MSG_INFO("Writing bucket inference debug dump to " << m_debugDumpFile.value()
                 << " (DebugDumpMaxEvents=" << m_debugDumpMaxEvents.value() << ")");
  }

  return StatusCode::SUCCESS;
}


StatusCode GraphBucketFilterTool::buildSegmentCountMap(const EventContext& ctx,
                                                       SegmentCountMap& segmentCounts) const {
  segmentCounts.clear();

  if (!m_printLabels.value() || m_labelSegmentKey.key().empty()) {
    return StatusCode::SUCCESS;
  }

  const xAOD::MuonSegmentContainer* segments{nullptr};
  ATH_CHECK(SG::get(segments, m_labelSegmentKey, ctx));

  for (const xAOD::MuonSegment* segment : *segments) {
    const auto* detailed = MuonR4::detailedSegment(*segment);
    if (!detailed) {
        continue;
    }
    const MuonR4::SpacePointBucket* parentBucket = detailed->parent()->parentBucket();
    ++segmentCounts[parentBucket];
  }

  return StatusCode::SUCCESS;
}

GraphBucketFilterTool::BucketLabelInfo
GraphBucketFilterTool::computeBucketLabel(const MuonR4::SpacePointBucket& bucket,
                                          const SegmentCountMap& segmentCounts) const {
  BucketLabelInfo out{};

  bool hasTruth = false;
  if (m_labelVisualizationTool.isEnabled()) {
    for (const auto& sp : bucket) {
      if (m_labelVisualizationTool->isLabeled(*sp)) {
        hasTruth = true;
        break;
      }
    }
    out.hasTruth = hasTruth;
  }

  unsigned int nSegments = 0;
  if (!m_labelSegmentKey.key().empty()) {
    const auto itr = segmentCounts.find(&bucket);
    if (itr != segmentCounts.end()) nSegments = itr->second;
    out.hasSegment = nSegments > 0;
    out.nSegments = nSegments;
  }

  const bool truthAvailable = (out.hasTruth >= 0);
  const bool segmentAvailable = (out.hasSegment >= 0);
  if (truthAvailable || segmentAvailable) {
    out.label = (out.hasTruth == 1 || out.hasSegment == 1) ? 1 : 0;
  }

  return out;
}

StatusCode GraphBucketFilterTool::runGraphInference(const EventContext& ctx,
                                                    GraphRawData& graphData) const {
  ATH_MSG_DEBUG("runGraphInference called");
  
  // buildGraph reads the container and builds features + edges
  ATH_CHECK(buildGraph(ctx, graphData));
  
  // Read the container again for classification (buildGraph doesn't store it)
  const MuonR4::SpacePointContainer* inputBuckets{nullptr};
  ATH_MSG_DEBUG("Reading container from key: " << m_readKey.key());
  ATH_CHECK(SG::get(inputBuckets, m_readKey, ctx));
  ATH_MSG_DEBUG("Container read successfully, size: " << (inputBuckets ? inputBuckets->size() : 0));

  // Check if we're in filtering mode (write key configured) or inference-only mode
  const bool doStoreWrite = !m_writeKey.key().empty();
  ATH_MSG_DEBUG("Store write mode: " << (doStoreWrite ? "YES" : "NO (inference-only)"));
  
  // Only create WriteHandle if we're actually writing to store
  std::unique_ptr<SG::WriteHandle<MuonR4::SpacePointContainer>> filteredBuckets;
  if (doStoreWrite) {
    filteredBuckets = std::make_unique<SG::WriteHandle<MuonR4::SpacePointContainer>>(m_writeKey, ctx);
    ATH_CHECK(filteredBuckets->record(std::make_unique<MuonR4::SpacePointContainer>()));
  }

  if (inputBuckets->empty()) {
    ATH_MSG_DEBUG("No input buckets found.");
    return StatusCode::SUCCESS;
  }

  // Buckets with bucket_size == 0 are not part of the training graph population,
  // so they are not fed to ONNX.  They are also not filtered here.
  if (graphData.featureLeaves.size() % kBucketFeatureCount != 0u) {
    ATH_MSG_ERROR("Feature tensor is not divisible by feature count per bucket: size="
                  << graphData.featureLeaves.size() << ", featureCount=" << kBucketFeatureCount);
    return StatusCode::FAILURE;
  }
  const size_t nGraphNodes = graphData.featureLeaves.size() / kBucketFeatureCount;
  if (nGraphNodes == 0u) {
    size_t passThroughNonModelBuckets = 0u;
    if (doStoreWrite) {
      for (const MuonR4::SpacePointBucket* bucket : *inputBuckets) {
        auto copied = std::make_unique<MuonR4::SpacePointBucket>(*bucket);
        (*filteredBuckets)->push_back(std::move(copied));
        ++passThroughNonModelBuckets;
      }
    }
    ATH_MSG_DEBUG("No training-like graph nodes found. Passed through "
                  << passThroughNonModelBuckets
                  << " non-model buckets without ONNX inference.");
    if (m_printFilterSummary) {
      ATH_MSG_INFO("BucketFilterSummary event=" << ctx.evt()
                   << " input=" << inputBuckets->size()
                   << " model_input=0"
                   << " model_kept=0"
                   << " output=" << passThroughNonModelBuckets
                   << " rejected=0"
                   << " non_model_passthrough=" << passThroughNonModelBuckets
                   << " expected_signal=n/a"
                   << " expected_signal_kept=n/a");
    }
    return StatusCode::SUCCESS;
  }

  ATH_CHECK(runInference(graphData));

  if (graphData.graph->dataTensor.size() <= 2) {
    ATH_MSG_ERROR("Missing output logits tensor at index 2.");
    return StatusCode::FAILURE;
  }

  const Ort::Value& outTensor = graphData.graph->dataTensor[2];
  const auto& info = outTensor.GetTensorTypeAndShapeInfo();
  std::vector<int64_t> outShape = info.GetShape();

  ATH_MSG_DEBUG("ONNX output tensor shape = [" << (outShape.empty() ? -1 : outShape[0])
                                               << (outShape.size()>1 ? ("," + std::to_string(outShape[1])) : std::string(""))
                                               << "]");

  OutputMode outputMode{};
  size_t numPred = 0;

  if (outShape.size() == 2 && outShape[1] == 3) {
    outputMode = OutputMode::MultiClass3;
    numPred = static_cast<size_t>(outShape[0]);
  } else if ((outShape.size() == 2 && outShape[1] == 1) || outShape.size() == 1) {
    outputMode = OutputMode::SingleOutput;
    numPred = static_cast<size_t>(outShape[0]);
  } else {
    ATH_MSG_ERROR("Unexpected ONNX output tensor shape = [" << fmt::format("{}", fmt::join(outShape, ","))
                  << "]  (expected [N,3] or [N] or [N,1]).");
    return StatusCode::FAILURE;
  }

  const float* outputPtr = outTensor.GetTensorData<float>();

  const size_t nElems = info.GetElementCount();
  const size_t expectedElems = (outputMode == OutputMode::MultiClass3) ? (numPred * 3) : numPred;
  if (nElems < expectedElems) {
    ATH_MSG_ERROR("Output tensor element count (" << nElems
                  << ") smaller than needed (" << expectedElems << ").");
    return StatusCode::FAILURE;
  }

  const float bias0 = static_cast<float>(m_biasClass0.value());
  const float scoreThreshold = static_cast<float>(m_scoreThreshold.value());

  // DEBUG: Print filter configuration
  ATH_MSG_DEBUG("=== DEBUGGING: Filter Configuration ===");
  ATH_MSG_DEBUG("Mode: " << (doStoreWrite ? "Filtering" : "Inference-only"));
  ATH_MSG_DEBUG("Output mode: " << (outputMode == OutputMode::MultiClass3 ? "multiclass [N,3]" : "single-output [N] or [N,1]"));
  if (outputMode == OutputMode::MultiClass3) {
    ATH_MSG_DEBUG("AcceptClasses: [" << m_acceptClasses[0]
                 << (m_acceptClasses.size() > 1 ? (", " + std::to_string(m_acceptClasses[1])) : "")
                 << (m_acceptClasses.size() > 2 ? (", " + std::to_string(m_acceptClasses[2])) : "") << "]");
    ATH_MSG_DEBUG("BiasClass0: " << bias0);
  } else {
    ATH_MSG_DEBUG("ScoreThreshold: " << scoreThreshold
                  << (m_singleOutputIsLogit.value()
                      ? " (keep if sigmoid(raw_output) > threshold)"
                      : " (keep if raw_output > threshold)"));
  }
  ATH_MSG_DEBUG("Total valid buckets to classify: " << numPred);
  ATH_MSG_DEBUG("=== END DEBUG CONFIG ===");

  size_t predIdx = 0;
  size_t kept = 0;
  size_t keptByModel = 0;
  size_t passThroughNonModelBuckets = 0;
  size_t validBuckets = 0;
  size_t class0_count = 0;
  size_t class1_count = 0;
  size_t class2_count = 0;
  size_t single_keep_count = 0;
  size_t single_reject_count = 0;

  SegmentCountMap segmentCounts;
  if (m_printLabels.value()) {
    ATH_CHECK(buildSegmentCountMap(ctx, segmentCounts));
  }

  const bool doDebugDump =
      !m_debugDumpFile.value().empty() &&
      (m_debugDumpMaxEvents.value() == 0 ||
       m_debugDumpEvents.load(std::memory_order_relaxed) < m_debugDumpMaxEvents.value());

  std::vector<int> selectedClasses;
  std::vector<int> keepFlags;
  std::vector<int> labels;
  std::vector<int> hasTruthFlags;
  std::vector<int> hasSegmentFlags;
  if (doDebugDump) {
    selectedClasses.reserve(numPred);
    keepFlags.reserve(numPred);
    if (m_printLabels.value()) {
      labels.reserve(numPred);
      hasTruthFlags.reserve(numPred);
      hasSegmentFlags.reserve(numPred);
    }
  }

  size_t labelledGoodBuckets = 0;
  size_t labelledGoodKept = 0;
  size_t labelledBadBuckets = 0;
  size_t labelledBadKept = 0;
  size_t labelledUnknownBuckets = 0;

  for (const MuonR4::SpacePointBucket* bucket : *inputBuckets) {
    if (!BucketGraphUtils::keepBucketAsTrainingNode(*bucket)) {
      ++passThroughNonModelBuckets;
      if (doStoreWrite) {
        auto copied = std::make_unique<MuonR4::SpacePointBucket>(*bucket);
        (*filteredBuckets)->push_back(std::move(copied));
        ++kept;
      }
      continue;
    }
    ++validBuckets;

    if (predIdx >= numPred) {
      // stop if the model gave fewer predictions than valid buckets
      ATH_MSG_WARNING("Fewer predictions than valid buckets; stopping at predIdx="
                      << predIdx << " / " << numPred);
      break;
    }

    bool keepBucket = false;
    int selectedClass = -1;
    if (outputMode == OutputMode::MultiClass3) {
      // read 3 logits for this valid bucket
      float l0 = outputPtr[3 * predIdx + 0] - bias0;
      float l1 = outputPtr[3 * predIdx + 1];
      float l2 = outputPtr[3 * predIdx + 2];

      int argmax = 0;
      float best = l0;
      if (l1 > best) { best = l1; argmax = 1; }
      if (l2 > best) { best = l2; argmax = 2; }

      if (argmax == 0) class0_count++;
      else if (argmax == 1) class1_count++;
      else if (argmax == 2) class2_count++;
      selectedClass = argmax;

      ATH_MSG_DEBUG("Bucket idx(valid)=" << predIdx
                    << " logits(biased0)=(" << l0 << ", " << l1 << ", " << l2
                    << ") -> class " << argmax);

      keepBucket = std::binary_search(m_acceptClasses.begin(), m_acceptClasses.end(), argmax);
    } else {
      const float rawScore = outputPtr[predIdx];
      const float score = m_singleOutputIsLogit.value()
          ? InferenceUtils::sigmoid(rawScore)
          : rawScore;
      keepBucket = (score > scoreThreshold);
      if (keepBucket) {
        ++single_keep_count;
      } else {
        ++single_reject_count;
      }
      selectedClass = keepBucket;

      ATH_MSG_DEBUG("Bucket idx(valid)=" << predIdx
                    << " raw_score=" << rawScore
                    << " interpreted_score=" << score
                    << " threshold=" << scoreThreshold
                    << " singleOutputIsLogit=" << (m_singleOutputIsLogit.value() ? 1 : 0)
                    << " -> keep=" << (keepBucket ? "yes" : "no"));
    }

    if (doDebugDump) {
      selectedClasses.push_back(selectedClass);
      keepFlags.push_back(keepBucket);
    }

    if (m_printLabels.value()) {
      const BucketLabelInfo labelInfo = computeBucketLabel(*bucket, segmentCounts);
      if (doDebugDump) {
        labels.push_back(labelInfo.label);
        hasTruthFlags.push_back(labelInfo.hasTruth);
        hasSegmentFlags.push_back(labelInfo.hasSegment);
      }

      if (labelInfo.label == 1) {
        ++labelledGoodBuckets;
        if (keepBucket) ++labelledGoodKept;
      } else if (labelInfo.label == 0) {
        ++labelledBadBuckets;
        if (keepBucket) ++labelledBadKept;
      } else {
        ++labelledUnknownBuckets;
      }

      if (m_labelPrintFirstNBuckets.value() != 0 &&
          predIdx < static_cast<size_t>(m_labelPrintFirstNBuckets.value())) {
        ATH_MSG_INFO("BucketLabel event=" << ctx.evt()
                     << " bucket_idx=" << predIdx
                     << " label=" << labelInfo.label
                     << " hasTruth=" << labelInfo.hasTruth
                     << " hasSegment=" << labelInfo.hasSegment
                     << " nSegments=" << labelInfo.nSegments
                     << " selectedClass=" << selectedClass
                     << " keep=" << keepBucket);
      }
    }

    if (keepBucket) {
      ++keptByModel;
      if (doStoreWrite) {
        auto copied = std::make_unique<MuonR4::SpacePointBucket>(*bucket);
        (*filteredBuckets)->push_back(std::move(copied));
        ++kept;
      }
    }

    ++predIdx; // consume exactly one prediction per valid bucket
  }

  ATH_MSG_DEBUG("Valid training-like buckets (bucket_size != 0): " << validBuckets
                  << ", pass-through non-model buckets: " << passThroughNonModelBuckets
                  << ", predictions consumed: " << predIdx
                  << ", kept by model: " << keptByModel
                  << ", total kept: " << kept);

  if (predIdx < numPred) {
    ATH_MSG_WARNING("Model produced more predictions (" << numPred
                     << ") than valid buckets consumed (" << predIdx << ").");
  }

  if (m_printLabels.value()) {
    const double goodEfficiency = labelledGoodBuckets > 0
        ? static_cast<double>(labelledGoodKept) / static_cast<double>(labelledGoodBuckets)
        : 0.0;
    const double badAcceptRate = labelledBadBuckets > 0
        ? static_cast<double>(labelledBadKept) / static_cast<double>(labelledBadBuckets)
        : 0.0;

    ATH_MSG_INFO("BucketLabelPerformance event=" << ctx.evt()
                 << " good(label=1)=" << labelledGoodBuckets
                 << " kept_good=" << labelledGoodKept
                 << " good_efficiency=" << goodEfficiency
                 << " bad(label=0)=" << labelledBadBuckets
                 << " kept_bad=" << labelledBadKept
                 << " bad_accept_rate=" << badAcceptRate
                 << " unknown_label=" << labelledUnknownBuckets);
  }

  if (m_printFilterSummary) {
    ATH_MSG_INFO("BucketFilterSummary event=" << ctx.evt()
                 << " input=" << inputBuckets->size()
                 << " model_input=" << validBuckets
                 << " model_kept=" << keptByModel
                 << " output=" << kept
                 << " rejected=" << (inputBuckets->size() - kept)
                 << " non_model_passthrough=" << passThroughNonModelBuckets);
    if (m_printLabels) {
      ATH_MSG_INFO("BucketFilterTruthSummary event=" << ctx.evt()
                   << " expected_signal=" << labelledGoodBuckets
                   << " expected_signal_kept=" << labelledGoodKept
                   << " expected_background=" << labelledBadBuckets
                   << " expected_background_kept=" << labelledBadKept
                   << " unknown_label=" << labelledUnknownBuckets);
    }
  }

  if (doDebugDump) {
    DebugDumpEventData eventData;
    eventData.outputMode =
        (outputMode == OutputMode::MultiClass3) ? "multiclass3" : "single_output";
    eventData.inputBucketCount = inputBuckets->size();
    eventData.validBuckets = validBuckets;
    eventData.predictionsConsumed = predIdx;
    eventData.kept = kept;
    eventData.selectedClasses = selectedClasses;
    eventData.keepFlags = keepFlags;
    eventData.labels = labels;
    eventData.hasTruthFlags = hasTruthFlags;
    eventData.hasSegmentFlags = hasSegmentFlags;

    ATH_CHECK(dumpDebugEvent(ctx, graphData, outTensor, eventData));
  }

  // DEBUG: Print classification summary
  ATH_MSG_DEBUG("=== DEBUGGING: Classification Summary ===");
  ATH_MSG_DEBUG("Total input buckets: " << inputBuckets->size());
  ATH_MSG_DEBUG("Training-like buckets classified by model: " << validBuckets);
  ATH_MSG_DEBUG("Non-model buckets passed through without ONNX prediction: " << passThroughNonModelBuckets);
  ATH_MSG_DEBUG("Total predictions: " << predIdx);
  if (outputMode == OutputMode::MultiClass3) {
    ATH_MSG_DEBUG("Class 0 (reject): " << class0_count << " (" << (100.0 * class0_count / std::max(predIdx, size_t(1))) << "%)");
    ATH_MSG_DEBUG("Class 1 (accept): " << class1_count << " (" << (100.0 * class1_count / std::max(predIdx, size_t(1))) << "%)");
    ATH_MSG_DEBUG("Class 2 (accept): " << class2_count << " (" << (100.0 * class2_count / std::max(predIdx, size_t(1))) << "%)");
  } else {
    ATH_MSG_DEBUG("Single-output reject (score <= threshold): " << single_reject_count
                  << " (" << (100.0 * single_reject_count / std::max(predIdx, size_t(1))) << "%)");
    ATH_MSG_DEBUG("Single-output accept (score > threshold): " << single_keep_count
                  << " (" << (100.0 * single_keep_count / std::max(predIdx, size_t(1))) << "%)");
  }
  if (doStoreWrite) {
    ATH_MSG_DEBUG("Total kept: " << kept << " = " << keptByModel << " by model + "
                  << passThroughNonModelBuckets << " pass-through non-model buckets");
    if (outputMode == OutputMode::MultiClass3) {
      ATH_MSG_DEBUG("Expected model-kept (class1+class2): " << (class1_count + class2_count));
    } else {
      ATH_MSG_DEBUG("Expected model-kept (score > threshold): " << single_keep_count);
    }
  }
  ATH_MSG_DEBUG("=== END DEBUG SUMMARY ===");

  return StatusCode::SUCCESS;
}

StatusCode GraphBucketFilterTool::dumpDebugEvent(const EventContext& ctx,
                                                 const GraphRawData& graphData,
                                                 const Ort::Value& outTensor,
                                                 const DebugDumpEventData& eventData) const {
  std::lock_guard<std::mutex> lock(m_debugDumpMutex);

  if (m_debugDumpMaxEvents.value() != 0 &&
      m_debugDumpEvents.load(std::memory_order_relaxed) >= m_debugDumpMaxEvents.value()) {
  return StatusCode::SUCCESS;
  }

  if (!graphData.graph || graphData.graph->dataTensor.size() < 3) {
    ATH_MSG_ERROR("Cannot write debug dump: graph tensors are incomplete.");
    return StatusCode::FAILURE;
  }

  const Ort::Value& featureTensor = graphData.graph->dataTensor[0];
  const Ort::Value& edgeTensor = graphData.graph->dataTensor[1];

  const auto featureInfo = featureTensor.GetTensorTypeAndShapeInfo();
  const auto edgeInfo = edgeTensor.GetTensorTypeAndShapeInfo();
  const auto outputInfo = outTensor.GetTensorTypeAndShapeInfo();

  const std::vector<int64_t> featureShape = featureInfo.GetShape();
  const std::vector<int64_t> edgeShape = edgeInfo.GetShape();
  const std::vector<int64_t> outputShape = outputInfo.GetShape();

  const float* featureData = featureTensor.GetTensorData<float>();
  const int64_t* edgeData = edgeTensor.GetTensorData<int64_t>();
  const float* outputData = outTensor.GetTensorData<float>();

  const std::size_t featureSize = featureInfo.GetElementCount();
  const std::size_t edgeSize = edgeInfo.GetElementCount();
  const std::size_t outputSize = outputInfo.GetElementCount();

  auto toJsonFloatArray = [](const float* data, std::size_t size) {
    nlohmann::json values = nlohmann::json::array();
    values.get_ref<nlohmann::json::array_t&>().reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
      values.push_back(std::isfinite(data[i]) ? nlohmann::json(data[i]) : nlohmann::json(nullptr));
    }
    return values;
  };
  auto toJsonInt64Array = [](const int64_t* data, std::size_t size) {
    nlohmann::json values = nlohmann::json::array();
    values.get_ref<nlohmann::json::array_t&>().reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
      values.push_back(data[i]);
    }
    return values;
  };

  std::ofstream out(m_debugDumpFile.value(), std::ios::out | std::ios::app);
  if (!out) {
    ATH_MSG_ERROR("Could not append to bucket inference debug dump file: " << m_debugDumpFile.value());
    return StatusCode::FAILURE;
  }

  const unsigned int dumpIndex = m_debugDumpEvents.fetch_add(1, std::memory_order_relaxed);

  nlohmann::ordered_json eventJson;
  eventJson["record_type"] = "event";
  eventJson["format_version"] = 2;
  eventJson["dump_index"] = dumpIndex;
  eventJson["event_context_evt"] = ctx.evt();
  eventJson["slot"] = ctx.slot();
  eventJson["input_bucket_count"] = eventData.inputBucketCount;
  eventJson["valid_buckets"] = eventData.validBuckets;
  eventJson["predictions_consumed"] = eventData.predictionsConsumed;
  eventJson["kept"] = eventData.kept;
  eventJson["output_mode"] = eventData.outputMode;
  eventJson["score_threshold"] = m_scoreThreshold.value();
  eventJson["bias_class0"] = m_biasClass0.value();
  eventJson["accept_classes"] = m_acceptClasses.value();
  eventJson["features_shape"] = featureShape;
  eventJson["edge_index_shape"] = edgeShape;
  eventJson["logits_shape"] = outputShape;
  eventJson["features"] = toJsonFloatArray(featureData, featureSize);
  eventJson["edge_index"] = toJsonInt64Array(edgeData, edgeSize);
  eventJson["logits"] = toJsonFloatArray(outputData, outputSize);
  eventJson["selected_class"] = eventData.selectedClasses;
  eventJson["keep"] = eventData.keepFlags;

  if (m_printLabels.value()) {
    eventJson["labels"] = eventData.labels;
    eventJson["has_truth"] = eventData.hasTruthFlags;
    eventJson["has_segment"] = eventData.hasSegmentFlags;
  }

  out << eventJson.dump() << '\n';

  ATH_MSG_DEBUG("Wrote bucket inference debug event " << dumpIndex << " to " << m_debugDumpFile.value());
  return StatusCode::SUCCESS;
}

} // namespace MuonML