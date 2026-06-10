// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef ATHEXTRITON_EVALUATEUTILS_H
#define ATHEXTRITON_EVALUATEUTILS_H

#include <string>
#include <vector>

namespace EvaluateUtils {
//*******************************************************************
// for reading MNIST images
std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(
    const std::string& full_path);

// flatten a vector of vectors into a vector
std::vector<float> flattenNestedVectors(
    const std::vector<std::vector<float>>& nestedVector);
}  // namespace EvaluateUtils

#endif  // ATHEXTRITON_EVALUATEUTILS_H
