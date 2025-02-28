//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "LinearTransformAsyncExampleAlg.h"

namespace AthCUDAExamples {

StatusCode LinearTransformAsyncExampleAlg::execute(const EventContext&) const {
  ATH_MSG_INFO("Starting execute");

  // Create a dummy array variable in pinned memory that will be multiplied by
  // some amount.
  static const std::size_t ARRAY_SIZE = 10000;
  static const float ARRAY_ELEMENT = 3.141592f;
  std::vector<float> dummyArray(ARRAY_SIZE, ARRAY_ELEMENT);

  // Run calculation. See LinearTransformAsyncExampleAlg.cu.
  static const float MULTIPLIER = 1.23f;
  ATH_MSG_INFO("Starting linearTransform");
  ATH_CHECK(linearTransform(dummyArray, MULTIPLIER));

  // Check if the operation succeeded.
  static const float EXPECTED_RESULT = ARRAY_ELEMENT * MULTIPLIER;
  for (std::size_t i = 0; i < ARRAY_SIZE; ++i) {
    if (std::abs(dummyArray[i] - EXPECTED_RESULT) > 0.001) {
      ATH_MSG_ERROR("The CUDA transformation failed to run");
      return StatusCode::FAILURE;
    }
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace AthCUDAExamples
