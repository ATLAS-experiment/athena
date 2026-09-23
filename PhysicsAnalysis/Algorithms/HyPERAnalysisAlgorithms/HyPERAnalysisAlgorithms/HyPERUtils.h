/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HYPERANALYSISALGORITHMS_HYPERUTILS_H
#define HYPERANALYSISALGORITHMS_HYPERUTILS_H

#include <algorithm>
#include <iostream>
#include <vector>

#include <AsgMessaging/MsgLevel.h>
#include "HyPERAnalysisAlgorithms/HyPERModel.h"

namespace EventReco {

// Standalone message level for HyPER library code (replaces RunHyPERAlg::s_msgLevelHyPER)
extern thread_local MSG::Level g_hyper_msg_level;
// HyPER particle IDs
enum HyPERParticleID { jet = 1, e = 2, mu = 3, met = 4, tau = 5 };

// This function is equivlant to `itertools.combination` in python.
template <typename T>
void combinations(const std::vector<T>& elements, int64_t k, int64_t offset,
                  std::vector<T>& current,
                  std::vector<std::vector<T>>& result) {
  // k is how many elements we are left to find, at the beginning is equal to
  // hyperEdgeOrder
  if (k == 0) {  // When we found all elements, store in the result.
    result.push_back(current);
    return;
  }
  // First time, we start with the first element of the array up until the last
  // possible combination.
  for (std::size_t i = offset; i <= elements.size() - k; ++i) {
    current.push_back(elements[i]);  // Push the current element.
    combinations(elements, k - 1, i + 1, current,
                 result);  // we now need just k-1 elements , and we move one
                           // position forward.
    current.pop_back();    // We remove the last one and go again.
  }
}

// This function find all possible combiantions of `order=hyperEdgeOrder`, from
// a given number of elements (nodes).
template <typename T>
std::vector<std::vector<T>> buildCombinations(const std::vector<T>& elements,
                                              int64_t hyperEdgeOrder) {
  std::vector<std::vector<T>>
      result;              // This stores all the possible combinations
  std::vector<T> current;  // This is a single combination, at the beginning
                           // they are empty.
  combinations<T>(elements, hyperEdgeOrder, 0, current,
                  result);  // Find the correct combinations.
  return result;
}

// Convert a 1D vector from type T to type V
template <typename T, typename V>
std::vector<T> vector1DTypeConverter(const std::vector<V>& input) {
  std::vector<T> out = {};
  for (std::size_t i = 0; i < static_cast<std::size_t>(input.size()); i++) {
    out.push_back(static_cast<T>(input[i]));
  }
  return out;
}

// Convert a 2D vector from type T to type V
template <typename T, typename V>
std::vector<std::vector<T>> vector2DTypeConverter(
    const std::vector<std::vector<V>>& input) {
  std::vector<std::vector<T>> out = {};

  for (const auto& row : input) {  // Each of the rows is a vector<V>
    std::vector<T> newRow;  // Use the previous function to convert each row.
    newRow = vector1DTypeConverter<T, V>(row);
    out.push_back(newRow);
  }

  return out;
}

// Convert an ONNX output (1D) to a 2D vector
template <typename T>
std::vector<std::vector<T>> convertONNXOutput(
    T* onnxOutput, const std::vector<int64_t>& shape) {
  std::vector<std::vector<T>> toSave = {};

  for (std::size_t i = 0; i < static_cast<std::size_t>(shape[0]); i++) {
    std::vector<T> row = {};
    for (std::size_t j = 0; j < static_cast<std::size_t>(shape[1]); j++) {
      row.push_back(onnxOutput[i * shape[1] + j]);
    }
    toSave.push_back(row);
  }
  return toSave;
}

// Flatten tensors
template <typename T>
void flatTensorAndSetShape(const std::vector<std::vector<T>>& input,
                           std::vector<T>& flatTensor,
                           std::vector<int64_t>& shape) {
  if (input.empty())
    throw std::runtime_error("Tensor is empty, it cannot be flatten.");
  shape = {static_cast<int64_t>(input.size()),
           static_cast<int64_t>(input[0].size())};
  for (std::size_t row = 0; row < static_cast<std::size_t>(input.size());
       row++) {
    flatTensor.insert(std::end(flatTensor), std::begin(input[row]),
                      std::end(input[row]));
  }
}

// This transposes a matrix
std::vector<std::vector<int64_t>> transpose(
    const std::vector<std::vector<int64_t>>& vecs);

// This generates a vector from 0 to nNodes - 1
std::vector<int64_t> range(int64_t n);

// @enum HyPERTopology
HyPERTopology strToHyPERTopology(const std::string& str);

// getIndexFromLabel
std::size_t getIndexFromLabel(const std::vector<std::string>& vec,
                              const std::string& label);

// Calculate delta phi between two angles in radians from -pi to pi
float deltaPhi(float phi1, float phi2);

}  // namespace EventReco

#endif  // HYPERANALYSISALGORITHMS_HYPERUTILS_H
