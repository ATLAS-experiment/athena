/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HYPERANALYSISALGORITHMS_HYPERTTBARALLHADRONICMODEL_H
#define HYPERANALYSISALGORITHMS_HYPERTTBARALLHADRONICMODEL_H

#include <string>
#include <vector>

// HyPER includes
#include "HyPERAnalysisAlgorithms/HyPERModel.h"

namespace EventReco {
/**
 * @class HyPERTtbarAllHadronicModel
 * @brief This class is in charge of loading the correct HyPER model based on
 * the Ttbar all hadronic topology.
 */
class HyPERTtbarAllHadronicModel : public HyPERModel {
 public:
  HyPERTtbarAllHadronicModel(
      const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnEven,
      const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnOdd)
      : HyPERModel(trainedOnEven, trainedOnOdd,
                   HyPERTopology::TtbarAllHadronic) {}

  // Input nodes in the order declared by the ONNX graph. Note that this
  // topology names the hyperedge batch tensor differently to the others.
  std::vector<std::string> getInputNames() const override {
    return std::vector<std::string>{"x_s",           "edge_index",
                                    "edge_attr_s",   "u_s",
                                    "batch",         "edge_index_h",
                                    "edge_index_h_batch"};
  }

  // Every output of the ONNX graph, in declaration order. Note that
  // "edge_index_h_batch" is passed straight through by the model and is not
  // consumed by the parser, but it still has to be bound. This topology has
  // no classification score.
  std::vector<HyPEROutputNode> getModelOutputs() const override {
    return std::vector<HyPEROutputNode>{
        {"sigmoid_1", true, HyPEROutputDim::HyperEdges, true},
        {"edge_index_h_batch", false, HyPEROutputDim::HyperEdges, false},
        {"sigmoid", true, HyPEROutputDim::Edges, true}};
  }

  std::vector<std::string> getOutputNames() const override {
    return std::vector<std::string>{"sigmoid_1", "sigmoid"};
  }
};
}  // namespace EventReco

#endif  // HYPERANALYSISALGORITHMS_HYPERTTBARALLHADRONICMODEL_H
