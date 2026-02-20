/*
  Copyright (C) 2002-2025 CERN
  for the benefit of the ATLAS collaboration
*/
#include "GraphBucketFilterTool.h"
#include "BucketGraphUtils.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <fmt/format.h>

namespace MuonML {

StatusCode GraphBucketFilterTool::initialize() {
  ATH_CHECK(setupModel());
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_writeKey.initialize());

  auto& accept = m_acceptClasses.value();
  std::sort(accept.begin(), accept.end());
  accept.erase(std::unique(accept.begin(), accept.end()), accept.end());

  return StatusCode::SUCCESS;
}

StatusCode GraphBucketFilterTool::runGraphInference(const EventContext& ctx,
                                                    GraphRawData& graphData) const {
  ATH_CHECK(buildGraph(ctx, graphData));
  
  const MuonR4::SpacePointContainer* inputBuckets{nullptr};
  ATH_CHECK(SG::get(inputBuckets, m_readKey, ctx));

  SG::WriteHandle filteredBuckets{m_writeKey, ctx};
  ATH_CHECK(filteredBuckets.record(std::make_unique<MuonR4::SpacePointContainer>()));

  if (inputBuckets->empty()) {
    ATH_MSG_DEBUG("No input buckets found.");
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
                                              << "]  (expect [num_buckets,3])");

  if (outShape.size() != 2 || outShape[1] != 3) {
    ATH_MSG_ERROR("Unexpected ONNX output tensor shape = [" << fmt::format("{}", fmt::join(outShape, ","))
                    << "]  (expected [num_buckets,3]).");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("ONNX output tensor shape = [" << fmt::format("{}", fmt::join(outShape, ",")) << "]");

  const size_t numPred = static_cast<size_t>(outShape[0]);

  const float* logitsPtr = outTensor.GetTensorData<float>(); // const_cast avoided with GetTensorData

  const size_t nElems = info.GetElementCount();
  if (nElems < numPred * 3) {
    ATH_MSG_ERROR("Output tensor element count (" << nElems
                  << ") smaller than needed (" << numPred * 3 << ").");
    return StatusCode::FAILURE;
  }

  const float bias0 = static_cast<float>(m_biasClass0.value());

  // DEBUG: Print filter configuration
  ATH_MSG_DEBUG("=== DEBUGGING: Filter Configuration ===");
  ATH_MSG_DEBUG("AcceptClasses: [" << m_acceptClasses[0] 
               << (m_acceptClasses.size() > 1 ? (", " + std::to_string(m_acceptClasses[1])) : "")
               << (m_acceptClasses.size() > 2 ? (", " + std::to_string(m_acceptClasses[2])) : "") << "]");
  ATH_MSG_DEBUG("BiasClass0: " << bias0);
  ATH_MSG_DEBUG("Total valid buckets to classify: " << numPred);
  ATH_MSG_DEBUG("=== END DEBUG CONFIG ===");

  size_t predIdx = 0;
  size_t kept = 0;
  size_t validBuckets = 0;
  size_t skippedZeroSize = 0;
  size_t class0_count = 0;
  size_t class1_count = 0;
  size_t class2_count = 0;

  for (const MuonR4::SpacePointBucket* bucket : *inputBuckets) {
    ++validBuckets;

    if (predIdx >= numPred) {
      // stop if the model gave fewer predictions than valid buckets
      ATH_MSG_WARNING("Fewer predictions than valid buckets; stopping at predIdx="
                      << predIdx << " / " << numPred);
      break;
    }

    // read 3 logits for this valid bucket
    float l0 = logitsPtr[3 * predIdx + 0] - bias0;
    float l1 = logitsPtr[3 * predIdx + 1];
    float l2 = logitsPtr[3 * predIdx + 2];

    int argmax = 0;
    float best = l0;
    if (l1 > best) { best = l1; argmax = 1; }
    if (l2 > best) { best = l2; argmax = 2; }

    // Count classifications
    if (argmax == 0) class0_count++;
    else if (argmax == 1) class1_count++;
    else if (argmax == 2) class2_count++;

    ATH_MSG_DEBUG( "Bucket idx(valid)=" << predIdx
                  << " logits(biased0)=(" << l0 << ", " << l1 << ", " << l2
                  << ") -> class " << argmax);
    
    if (std::binary_search(m_acceptClasses.begin(), m_acceptClasses.end(), argmax)) {
        auto copied = std::make_unique<MuonR4::SpacePointBucket>(*bucket);
        filteredBuckets->push_back(std::move(copied));
        ++kept;
    }
    
    ++predIdx; // consume exactly one prediction per valid bucket
  }

  ATH_MSG_DEBUG( "Valid buckets (size>0): " << validBuckets
                  << ", predictions consumed: " << predIdx
                  << ", kept: " << kept);

  if (predIdx < numPred) {
    ATH_MSG_WARNING("Model produced more predictions (" << numPred
                     << ") than valid buckets consumed (" << predIdx << ").");
  }

  // DEBUG: Print classification summary
  ATH_MSG_DEBUG("=== DEBUGGING: Classification Summary ===");
  ATH_MSG_DEBUG("Total input buckets: " << inputBuckets->size());
  ATH_MSG_DEBUG("Skipped buckets (size==0): " << skippedZeroSize);
  ATH_MSG_DEBUG("Valid buckets (size>0): " << validBuckets);
  ATH_MSG_DEBUG("Total predictions: " << predIdx);
  ATH_MSG_DEBUG("Class 0 (reject): " << class0_count << " (" << (100.0 * class0_count / std::max(predIdx, size_t(1))) << "%)");
  ATH_MSG_DEBUG("Class 1 (accept): " << class1_count << " (" << (100.0 * class1_count / std::max(predIdx, size_t(1))) << "%)");
  ATH_MSG_DEBUG("Class 2 (accept): " << class2_count << " (" << (100.0 * class2_count / std::max(predIdx, size_t(1))) << "%)");
  ATH_MSG_DEBUG("Total kept: " << kept << " (" << (100.0 * kept / std::max(validBuckets, size_t(1))) << "% efficiency)");
  ATH_MSG_DEBUG("Expected kept (class1+class2): " << (class1_count + class2_count));
  ATH_MSG_DEBUG("=== END DEBUG SUMMARY ===");

  return StatusCode::SUCCESS;
}

} // namespace MuonML

