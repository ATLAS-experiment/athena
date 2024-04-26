// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef Onnx_UTILS_H
#define Onnx_UTILS_H

#include <memory>
#include <vector>

// Onnx Runtime include(s).
#include <onnxruntime_cxx_api.h>

namespace AthOnnx {

// @author Xiangyang Ju <xiangyang.ju@cern.ch>

// @brief Convert a vector of vectors to a single vector.
// @param features The vector of vectors to be flattened.
// @return A single vector containing all the elements of the input vector of vectors.
template<typename T>
inline std::vector<T> flattenNestedVectors( const std::vector<std::vector<T>>& features) {
  // 1. Compute the total size required.
  int total_size = 0;
  for (const auto& feature : features) total_size += feature.size();

  std::vector<T> flatten1D;
  flatten1D.reserve(total_size);

  for (const auto& feature : features)
    for (const auto& elem : feature)
      flatten1D.push_back(elem);

  return flatten1D;
}

// @brief Get the input data shape and node names (in the computational graph) from the onnx model
// @param session The onnx session.
// @param dataShape The shape of the input data. Note that there may be multiple inputs.
// @param nodeNames The names of the input nodes in the computational graph.
// the dataShape and nodeNames will be updated.
void getInputNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape,
    std::vector<std::string>& nodeNames);

// @brief Get the output data shape and node names (in the computational graph) from the onnx model
// @param session The onnx session.
// @param dataShape The shape of the output data.
// @param nodeNames The names of the output nodes in the computational graph.
// the dataShape and nodeNames will be updated.
void getOutputNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape,
    std::vector<std::string>& nodeNames);

// Heleper function to get node info
void getNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape,
    std::vector<std::string>& nodeNames,
    bool isInput
);

// @brief to count the total number of elements in a tensor
// They are useful for reserving spaces for the output data.
int64_t getTensorSize(const std::vector<int64_t>& dataShape);

// Inference with IO binding. Better for performance, particularly for GPUs.
// See https://onnxruntime.ai/docs/performance/tune-performance/iobinding.html
void inferenceWithIOBinding(Ort::Session& session,
    const std::vector<std::string>& inputNames,
    const std::vector<Ort::Value>& inputData,
    const std::vector<std::string>& outputNames,
    const std::vector<Ort::Value>& outputData
);

// @brief Create a tensor from a vector of data and its shape.
Ort::Value createTensor(std::vector<float>& data, const std::vector<int64_t>& dataShape);


}
#endif
