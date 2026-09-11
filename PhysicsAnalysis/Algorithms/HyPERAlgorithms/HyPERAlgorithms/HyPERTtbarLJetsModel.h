// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#pragma once

#include <string>
#include <vector>

// HyPER includes
#include "HyPERAlgorithms/HyPERModel.h"

namespace EventReco {
/**
 * @class HyPERTtbarLJetsModel
 * @brief This class is in charge of loading the correct HyPER model based on
 * the Ttbar single lepton topology.
 */
class HyPERTtbarLJetsModel : public HyPERModel {
 public:
  HyPERTtbarLJetsModel(const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnEven,
                       const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnOdd)
      : HyPERModel(trainedOnEven, trainedOnOdd, HyPERTopology::TtbarLJets) {}

  // Input nodes in the order declared by the ONNX graph.
  std::vector<std::string> getInputNames() const override {
    return std::vector<std::string>{"x_s",           "edge_index",
                                    "edge_attr_s",   "u_s",
                                    "batch",         "edge_index_h",
                                    "batch_hyperedge"};
  }

  // Every output of the ONNX graph, in declaration order. Note that
  // "batch_hyperedge" is passed straight through by the model and is not
  // consumed by the parser, but it still has to be bound.
  std::vector<HyPEROutputNode> getModelOutputs() const override {
    return std::vector<HyPEROutputNode>{
        {"hyperedge_prime", true, HyPEROutputDim::HyperEdges, true},
        {"batch_hyperedge", false, HyPEROutputDim::HyperEdges, false},
        {"edge_prime", true, HyPEROutputDim::Edges, true},
        {"classification_score", true, HyPEROutputDim::Single, true}};
  }

  std::vector<std::string> getOutputNames() const override {
    return std::vector<std::string>{"hyperedge_prime", "edge_prime",
                                    "classification_score"};
  }
};
}  // namespace EventReco
