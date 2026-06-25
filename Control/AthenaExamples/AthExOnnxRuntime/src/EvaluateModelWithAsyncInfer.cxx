// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "EvaluateModelWithAsyncInfer.h"

// Framework include(s).
#include "AthOnnxUtils/OnnxUtils.h"
#include "EvaluateUtils.h"
#include "PathResolver/PathResolver.h"

// Standard include(s)
#include <algorithm>
#include <ranges>
#include <span>

namespace AthOnnx {

StatusCode EvaluateModelWithAsyncInfer::initialize() {
  if (m_batchSize.value() < 1) {
    ATH_MSG_ERROR("Requested an invalid batch size: " << m_batchSize.value());
    return StatusCode::FAILURE;
  }

  // Fetch tools
  ATH_CHECK(m_onnxTool.retrieve());

  // read input file, and the target file for comparison.
  std::string pixelFilePath =
      PathResolver::find_calib_file(m_pixelFileName.value());
  ATH_MSG_INFO("Using pixel file: " << pixelFilePath);

  try {
    m_input_tensor_values_notFlat =
        EvaluateUtils::read_mnist_pixel_notFlat(pixelFilePath);
    ATH_MSG_INFO(
        "Total no. of samples: " << m_input_tensor_values_notFlat.size());
  } catch (const std::exception& e) {
    ATH_MSG_ERROR(e.what());
    return StatusCode::FAILURE;
  }

  if (std::size_t(m_batchSize.value()) > m_input_tensor_values_notFlat.size()) {
    ATH_MSG_ERROR("The batch size requested ("
                  << m_batchSize.value()
                  << ") is greater than the number of available "
                     "samples ("
                  << m_input_tensor_values_notFlat.size() << ")");
    return StatusCode::FAILURE;
  }

  if (m_input_tensor_values_notFlat.size() % m_batchSize.value() != 0) {
    ATH_MSG_ERROR("The number of samples ("
                  << m_input_tensor_values_notFlat.size()
                  << ") is not a multiple of the requested batch size ("
                  << m_batchSize.value() << ")");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode EvaluateModelWithAsyncInfer::execute(
    [[maybe_unused]] const EventContext& ctx) const {
  // We know we have at least one image, otherwise we would have errored out
  // earlier
  const std::size_t n_batches =
      m_input_tensor_values_notFlat.size() / m_batchSize.value();
  const auto n_rows = std::int64_t(m_input_tensor_values_notFlat[0].size());
  const auto n_cols = std::int64_t(m_input_tensor_values_notFlat[0][0].size());

  for (std::size_t batch_idx = 0; batch_idx < n_batches; ++batch_idx) {
    // prepare inputs
    std::vector<float> inputDataVector;
    inputDataVector.reserve(m_batchSize.value() * n_rows * n_cols);
    for (const std::vector<std::vector<float>>& imageData :
         m_input_tensor_values_notFlat |
             std::views::drop(batch_idx * m_batchSize.value()) |
             std::views::take(m_batchSize.value())) {
      std::vector<float> flatten =
          AthOnnxUtils::flattenNestedVectors(imageData);
      inputDataVector.insert(inputDataVector.end(), flatten.begin(),
                             flatten.end());
    }

    std::vector<int64_t> inputShape = {m_batchSize.value(), n_rows, n_cols};

    AthInfer::InputDataMap inputData;
    inputData["flatten_input:0"] =
        std::make_pair(inputShape, std::move(inputDataVector));

    const std::int64_t n_scores = 10;
    AthInfer::OutputDataMap outputData;
    outputData["dense_1/Softmax:0"] = std::make_pair(
        std::vector<int64_t>{m_batchSize, n_scores}, std::vector<float>{});

    ATH_CHECK(m_onnxTool->inference(inputData, outputData));

    auto const& outputScores =
        std::get<std::vector<float>>(outputData["dense_1/Softmax:0"].second);

    if (outputScores.size() != std::size_t(n_scores * m_batchSize.value())) {
      ATH_MSG_ERROR("Got back " << outputScores.size()
                                << " scores when it should have been "
                                << n_scores << " * " << m_batchSize.value()
                                << " = " << n_scores * m_batchSize.value());
      return StatusCode::FAILURE;
    }

    for (int img_idx = 0; img_idx < m_batchSize.value(); img_idx++) {
      std::span scores(outputScores.begin() + img_idx * n_scores,
                       outputScores.begin() + (img_idx + 1) * n_scores);
      ATH_MSG_DEBUG("Scores for img " << img_idx << " of batch " << batch_idx
                                      << ": "
                                      << EvaluateUtils::spanToString(scores));
      const auto max_elem = std::ranges::max_element(scores);
      ATH_MSG_DEBUG("Class: " << max_elem - scores.begin()
                              << " has the highest score: " << *max_elem
                              << " in img " << img_idx << " of batch "
                              << batch_idx);
    }
  }
  return StatusCode::SUCCESS;
}

}  // namespace AthOnnx
