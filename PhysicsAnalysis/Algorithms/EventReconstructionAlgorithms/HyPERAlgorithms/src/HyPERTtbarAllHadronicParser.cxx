/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <AsgMessaging/MessageCheck.h>  // To access ANA_MSG

#include "HyPERAlgorithms/HyPERParser.h"
#include "HyPERAlgorithms/HyPERTopoReco.h"
#include "HyPERAlgorithms/HyPERTtbarAllHadronicParser.h"
#include "HyPERAlgorithms/HyPERUtils.h"

namespace EventReco {
// Build and add all ONNX inputs
void HyPERTtbarAllHadronicParser::buildONNXInputs(const HyPERGraph& hyperGraph,
                                                  HyPERModel& hyperModel) {

  // Build edge input tensors
  std::vector<std::vector<int64_t>> l_edge_index_ = {{}, {}};
  std::vector<std::vector<float>> l_edge_attr_s_ = {};
  for (std::size_t i = 0; i < static_cast<std::size_t>(hyperGraph.nEdges());
       i++) {
    l_edge_index_[0].push_back(
        static_cast<int64_t>(hyperGraph.getEdgeIndicesVector()[i].first));
    l_edge_index_[1].push_back(
        static_cast<int64_t>(hyperGraph.getEdgeIndicesVector()[i].second));
    l_edge_attr_s_.push_back(hyperGraph.getEdgeFeats(i));
  }

  m_edge_out_shape = std::vector<int64_t>{
      static_cast<int64_t>(l_edge_index_[0].size()),
      static_cast<int64_t>(
          1)};  // Currently only support binary classification for edges
  // Edege attributes tensor
  flatTensorAndSetShape<float>(l_edge_attr_s_, m_l_edge_attr_s_,
                               m_l_edge_attr_s_shape);
  hyperModel.setInputs<float>(m_l_edge_attr_s_name, m_l_edge_attr_s_,
                              m_l_edge_attr_s_shape);
  // Edge indices tensor
  flatTensorAndSetShape<int64_t>(l_edge_index_, m_l_edge_index_,
                                 m_l_edge_index_shape);
  hyperModel.setInputs<int64_t>(m_l_edge_index_name, m_l_edge_index_,
                                m_l_edge_index_shape);

  // Build node input tensor
  std::vector<std::vector<float>> l_x_s_ = {};
  std::vector<int64_t> l_batch_ = {};
  for (std::size_t i = 0; i < static_cast<std::size_t>(hyperGraph.nNodes());
       i++) {
    l_x_s_.push_back(hyperGraph.getNodeFeats(i));
    l_batch_.push_back(static_cast<int64_t>(0));
  }

  // Node features tensor
  flatTensorAndSetShape<float>(l_x_s_, m_l_x_s_, m_l_x_s_shape);
  hyperModel.setInputs<float>(m_l_x_s_name, m_l_x_s_, m_l_x_s_shape);
  // Batch tensor
  m_l_batch_shape = std::vector<int64_t>{static_cast<int64_t>(l_batch_.size())};
  m_l_batch_ = l_batch_;
  hyperModel.setInputs<int64_t>(m_l_batch_name, m_l_batch_, m_l_batch_shape);

  // Build global input tensor
  std::vector<std::vector<float>> l_u_s_ = {hyperGraph.getGlobalFeats()};
  flatTensorAndSetShape<float>(l_u_s_, m_l_u_s_, m_l_u_s_shape);
  hyperModel.setInputs<float>(m_l_u_s_name, m_l_u_s_, m_l_u_s_shape);

  // Build hyperedge input tensor
  std::vector<std::vector<int64_t>> l_edge_index_h_ = {};
  std::vector<int64_t> l_edge_index_h_batch_ = {};
  bool h_batch_filled = false;
  for (std::size_t i = 0;
       i < static_cast<std::size_t>(hyperGraph.hyperEdgeOrder()); i++) {
    std::vector<int64_t> e_m = {};
    for (std::size_t j = 0;
         j < static_cast<std::size_t>(hyperGraph.nHyperEdges()); j++) {
      if (!h_batch_filled) {
        l_edge_index_h_batch_.push_back(static_cast<int64_t>(0));
      }
      e_m.push_back(hyperGraph.getHyperEdgeIndices(j)[i]);
    }
    l_edge_index_h_.push_back(e_m);
    h_batch_filled = true;
  }

  m_hyperedge_out_shape = std::vector<int64_t>{
      static_cast<int64_t>(l_edge_index_h_[0].size()),
      static_cast<int64_t>(
          1)};  // Currently only support binary classification for hyperedges
  // Hyperedge indices tensor
  flatTensorAndSetShape<int64_t>(l_edge_index_h_, m_l_edge_index_h_,
                                 m_l_edge_index_h_shape);
  hyperModel.setInputs<int64_t>(m_l_edge_index_h_name, m_l_edge_index_h_,
                                m_l_edge_index_h_shape);
  // Hyperedge batch tensor
  m_l_edge_index_h_batch_shape =
      std::vector<int64_t>{static_cast<int64_t>(l_edge_index_h_batch_.size())};
  m_l_edge_index_h_batch_ = l_edge_index_h_batch_;
  hyperModel.setInputs<int64_t>(m_l_edge_index_h_batch_name,
                                m_l_edge_index_h_batch_,
                                m_l_edge_index_h_batch_shape);
}

// Get HyPER outputs
void HyPERTtbarAllHadronicParser::getONNXOutputs(HyPERModel& hyperModel) {

  // Get model-specific output names
  std::vector<std::string> outputNames = hyperModel.getOutputNames();

  // Get model outputs
  float* hyperedge_out = hyperModel.getOutputs<float>(outputNames[0]);
  float* edge_out = hyperModel.getOutputs<float>(outputNames[1]);

  m_hyperedge_out =
      convertONNXOutput<float>(hyperedge_out, m_hyperedge_out_shape);
  m_edge_out = convertONNXOutput<float>(edge_out, m_edge_out_shape);
}

void HyPERTtbarAllHadronicParser::printOutputsForValidation() const {
  using namespace asg::msgUserCode;
  setMsgLevel(MSG::INFO);
  ANA_MSG_INFO("HYPEREDGE OUT:");
  std::string row = "[";
  for (const auto& i : m_hyperedge_out) {
    for (const auto j : i) {
      row += std::to_string(j) + ", ";
    }
  }
  row += "],";
  ANA_MSG_INFO(row);

  ANA_MSG_INFO("EDGE OUT:");
  row = "[";
  for (const auto& i : m_edge_out) {
    for (const auto j : i) {
      row += std::to_string(j) + ", ";
    }
  }
  row += "],";
  ANA_MSG_INFO(row);
}

// Parser outputs
void HyPERTtbarAllHadronicParser::reconstructOutputs(
    const HyPERGraph& hyperGraph) {
  RecoTtbarAllHadronic(hyperGraph, m_edge_out, m_hyperedge_out,
                       m_reco_indices_out, m_reco_scores_out,
                       m_reco_labels_out);
}

// Clear the parser
void HyPERTtbarAllHadronicParser::clear() {
  m_edge_out_shape.clear();
  m_hyperedge_out_shape.clear();
  m_edge_out.clear();
  m_hyperedge_out.clear();
  m_reco_indices_out.clear();
  m_reco_scores_out.clear();
  m_reco_labels_out.clear();

  m_l_edge_index_.clear();
  m_l_edge_index_shape.clear();
  m_l_edge_attr_s_.clear();
  m_l_edge_attr_s_shape.clear();
  m_l_x_s_.clear();
  m_l_x_s_shape.clear();
  m_l_batch_.clear();
  m_l_batch_shape.clear();
  m_l_edge_index_h_.clear();
  m_l_edge_index_h_shape.clear();
  m_l_edge_index_h_batch_.clear();
  m_l_edge_index_h_batch_shape.clear();
  m_l_u_s_.clear();
  m_l_u_s_shape.clear();
}

}  // namespace EventReco
