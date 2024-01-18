/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TFCSGANLWTNNHandler.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// class header include
#include "ISF_FastCaloSimEvent/TFCSGANLWTNNHandler.h"

#include "TFile.h" //Needed for TBuffer

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

TFCSGANLWTNNHandler::TFCSGANLWTNNHandler() { m_graph = nullptr; }

TFCSGANLWTNNHandler::~TFCSGANLWTNNHandler() {
  if (m_input != nullptr) {
    delete m_input;
  }
  if (m_graph != nullptr) {
    delete m_graph;
  }
}

bool TFCSGANLWTNNHandler::LoadGAN(const std::string &inputFile) {
  std::ifstream input(inputFile);
  std::stringstream sin;
  sin << input.rdbuf();
  input.close();
  // build the graph
  auto config = lwt::parse_json_graph(sin);
  m_graph = new lwt::LightweightGraph(config);
  if (m_graph == nullptr) {
    return false;
  }
  if (m_input != nullptr) {
    delete m_input;
  }
  m_input = new std::string(sin.str());
  return true;
}

void TFCSGANLWTNNHandler::Streamer(TBuffer &R__b) {
  // Stream an object of class TFCSGANLWTNNHandler
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(TFCSGANLWTNNHandler::Class(), this);
    if (m_graph != nullptr) {
      delete m_graph;
      m_graph = nullptr;
    }
    if (m_input != nullptr) {
      std::stringstream sin;
      sin.str(*m_input);
      auto config = lwt::parse_json_graph(sin);
      m_graph = new lwt::LightweightGraph(config);
    }
#ifndef __FastCaloSimStandAlone__
    // When running inside Athena, delete config to free the memory
    delete m_input;
    m_input = nullptr;
  }
  // build the graph
  ATH_MSG_VERBOSE("m_json has size " << m_json.length());
  ATH_MSG_DEBUG("m_json starts with  " << m_json.substr(0, 10));
  ATH_MSG_VERBOSE("Reading the m_json string stream into a graph network");
  std::stringstream json_stream(m_json);
  const lwt::GraphConfig config = lwt::parse_json_graph(json_stream);
  m_lwtnn_graph = std::make_unique<lwt::LightweightGraph>(config);
  // Get the output layers
  ATH_MSG_VERBOSE("Getting output layers for neural network");
  for (auto node : config.outputs) {
    const std::string node_name = node.first;
    const lwt::OutputNodeConfig node_config = node.second;
    for (const std::string & label : node_config.labels) {
      ATH_MSG_VERBOSE("Found output layer called " << node_name << "_"
                                                   << label);
      m_outputLayers.push_back(node_name + "_" + label);
    }
  };
  ATH_MSG_VERBOSE("Removing prefix from stored layers.");
  removePrefixes(m_outputLayers);
  ATH_MSG_VERBOSE("Finished output nodes.");
};

std::vector<std::string> TFCSGANLWTNNHandler::getOutputLayers() const {
  return m_outputLayers;
};

// This is implement the specific compute, and ensure the output is returned in
// regular format. For LWTNN, that's easy.
TFCSGANLWTNNHandler::NetworkOutputs TFCSGANLWTNNHandler::compute(
    TFCSGANLWTNNHandler::NetworkInputs const &inputs) const {
  ATH_MSG_DEBUG("Running computation on LWTNN graph network");
  NetworkInputs local_copy = inputs;
  if (inputs.find("Noise") != inputs.end()) {
    // Graphs from EnergyAndHitsGANV2 have the local_copy encoded as Noise =
    // node_0 and mycond = node_1
    auto noiseNode = local_copy.extract("Noise");
    noiseNode.key() = "node_0";
    local_copy.insert(std::move(noiseNode));
    auto mycondNode = local_copy.extract("mycond");
    mycondNode.key() = "node_1";
    local_copy.insert(std::move(mycondNode));
  }
  // now we can compute
  TFCSGANLWTNNHandler::NetworkOutputs outputs =
      m_lwtnn_graph->compute(local_copy);
  removePrefixes(outputs);
  ATH_MSG_DEBUG("Computation on LWTNN graph network done, returning.");
  return outputs;
};

// Giving this it's own streamer to call setupNet
void TFCSGANLWTNNHandler::Streamer(TBuffer &buf) {
  ATH_MSG_DEBUG("In streamer of " << __FILE__);
  if (buf.IsReading()) {
    ATH_MSG_DEBUG("Reading buffer in TFCSGANLWTNNHandler ");
    // Get the persisted variables filled in
    TFCSGANLWTNNHandler::Class()->ReadBuffer(buf, this);
    ATH_MSG_DEBUG("m_json has size " << m_json.length());
    ATH_MSG_DEBUG("m_json starts with  " << m_json.substr(0, 10));
    // Setup the net, creating the non persisted variables
    // exactly as in the constructor
    this->setupNet();
#ifndef __FastCaloSimStandAlone__
    // When running inside Athena, delete persisted information
    // to conserve memory
    this->deleteAllButNet();
#endif
  } else {
    R__b.WriteClassBuffer(TFCSGANLWTNNHandler::Class(), this);
  }
}
