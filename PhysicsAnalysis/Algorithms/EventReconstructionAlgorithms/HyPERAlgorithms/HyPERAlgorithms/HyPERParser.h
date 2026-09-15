/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include <memory>

#include "HyPERAlgorithms/HyPERGraph.h"
#include "HyPERAlgorithms/HyPERModel.h"

namespace EventReco {
/**
 * @class HyPERParser
 * @brief This class is in charge of parsing the information from:
 * Graph object -> onnxruntime object.
 * onnxruntime object -> Parser object (for the outputs).
 * Reconstruct the objects from the outputs.
 * It is a pure virtual class, and it must be implemented for each topology.
 */
class HyPERParser {
 public:
  HyPERParser() = default;
  virtual ~HyPERParser() = default;

  /**
   * @brief This method takes the information from the graph object and builds
   * the inputs for the onnxruntime object.
   */
  virtual void buildONNXInputs(const HyPERGraph& hyperGraph,
                               HyPERModel& hyperModel) = 0;

  /**
   * @brief This method takes the information from the onnxruntime object and
   * builds the outputs on the parser object.
   */
  virtual void getONNXOutputs(HyPERModel& hyperModel) = 0;

  /**
   * @brief This method implements the reconstruction of the outputs from the
   * parser object into physcally meaningful quantities.
   */
  virtual void reconstructOutputs(const HyPERGraph& hyperGraph) = 0;

  /**
   * @brief The following methods return the reconstruction labels, scores and
   * related indices from the parser object. This should be used after the
   * reconstructOutputs method.
   */
  virtual std::vector<std::string> getLabels() = 0;
  virtual std::vector<float> getScores() = 0;
  virtual std::vector<std::vector<int>> getIndices() = 0;
  virtual std::vector<std::vector<int>> getIds() = 0;
  virtual float getClassificationScore() = 0;

  /**
   * @brief This method clears the parser object.
   */
  virtual void clear() = 0;
  virtual void printOutputsForValidation() const = 0;
};
}  // namespace EventReco
