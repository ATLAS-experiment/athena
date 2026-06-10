/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/GNN.h"
#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"

#include "PathResolver/PathResolver.h"

#include "InDetGNNHardScatterSelection/TracksLoader.h"
#include "InDetGNNHardScatterSelection/JetsLoader.h"
#include "InDetGNNHardScatterSelection/PhotonsLoader.h"
#include "InDetGNNHardScatterSelection/ElectronsLoader.h"
#include "InDetGNNHardScatterSelection/MuonsLoader.h"
#include "InDetGNNHardScatterSelection/IParticlesLoader.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

#include <algorithm>
#include <cfenv>
#include <cmath>
#include <array>
#include <sstream>
#include <stdexcept>

namespace InDetGNNHardScatterSelection {

  GNN::GNN(const std::string& nn_file):
    m_saltModel(nullptr),
    m_modelPath(),
    m_input_node_name()
  {

    // Load and initialize the neural network model from the given file path.
    std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_file);
    m_modelPath = fullPathToOnnxFile;
    m_saltModel = std::make_shared<FlavorTagInference::SaltModel>(fullPathToOnnxFile);
    auto graph_config = m_saltModel->getGraphConfig();

    // Vertex tensor key for runInference: must match InputNodeConfig::name in embedded gnn_config
    // (same metadata SaltModel uses for graph_config).
    std::vector<std::string> input_node_names;
    input_node_names.reserve(graph_config.inputs.size());
    for (const auto& node : graph_config.inputs) {
      input_node_names.push_back(node.name);
    }

    if (std::find(input_node_names.begin(), input_node_names.end(), "vertice_features") != input_node_names.end()) {
      m_input_node_name = "vertice_features";
    } else if (std::find(input_node_names.begin(), input_node_names.end(), "vertex_features") != input_node_names.end()) {
      m_input_node_name = "vertex_features";
    } else if (graph_config.inputs.size() == 1) {
      m_input_node_name = graph_config.inputs.front().name;
    } else {
      std::ostringstream msg;
      msg << "Unsupported scalar vertex input name in model '" << nn_file << "'. Graph config inputs: ";
      for (const auto& name : input_node_names) {
        msg << name << " ";
      }
      throw std::runtime_error(msg.str());
    }

    // Create configuration objects for data preprocessing.
    auto [inputs, constituents_configs] = dataprep::createGetterConfig(graph_config);
    
    for (const auto& config : constituents_configs){
      switch (config.type){
      case ConstituentsType::TRACK:
        m_constituentsLoaders.push_back(std::make_shared<TracksLoader>(config));
        break;
      case ConstituentsType::ELECTRON:
        m_constituentsLoaders.push_back(std::make_shared<ElectronsLoader>(config));
        break;
      case ConstituentsType::MUON:
        m_constituentsLoaders.push_back(std::make_shared<MuonsLoader>(config));
        break;
      case ConstituentsType::JET:
        m_constituentsLoaders.push_back(std::make_shared<JetsLoader>(config));
        break;
      case ConstituentsType::PHOTON:
        m_constituentsLoaders.push_back(std::make_shared<PhotonsLoader>(config));
        break;
      case ConstituentsType::IPARTICLE:
        m_constituentsLoaders.push_back(std::make_shared<IParticlesLoader>(config));
        break;
      }
    }

    m_varsFromVertex = dataprep::createVertexVarGetters(inputs);

    // Retrieve the configuration for the model outputs.
    FlavorTagInference::OutputConfig gnn_output_config = m_saltModel->getOutputConfig();

    for (const auto& outNode : gnn_output_config) {
      // the node's output name will be used to define the decoration name
      std::string dec_name = outNode.name;
      m_decorators.vertexFloat.emplace_back(outNode.name, Dec<float>(dec_name));
      if (outNode.name == "salt_phsvertex") {
        m_decorators.vertexFloat.emplace_back("salt_phsvertex", Dec<float>("HSGN2_phsvertex"));
      }
    }
  }

  GNN::~GNN() = default;

  float GNN::decorate(const xAOD::Vertex& vertex) const {
    /* Main function for decorating a vertex with GNN outputs. */
    using namespace internal;

    // prepare input
    // -------------
    FlavorTagInference::InputMap gnn_input;

    std::vector<float> vertex_feat;
    vertex_feat.reserve(m_varsFromVertex.size());
    for (const auto& getter: m_varsFromVertex) {
      vertex_feat.push_back(getter(vertex).second);
    }
    std::vector<int64_t> vertexfeat_dim = {1, static_cast<int64_t>(vertex_feat.size())};

    for (const auto& value : vertex_feat) {
      if (!std::isfinite(value)) {
        throw std::runtime_error("Non-finite scalar vertex input before runInference");
      }
    }

    FlavorTagInference::Inputs vertex_info (vertex_feat, vertexfeat_dim);
    gnn_input.insert({m_input_node_name, vertex_info});
    // Provide common scalar aliases to absorb metadata/model naming mismatches.
    constexpr std::array<const char*, 4> scalar_input_aliases = {
      "vertice_features", "vertex_features", "vertice_var", "vertex_var"
    };
    for (const char* alias : scalar_input_aliases) {
      gnn_input.insert({alias, vertex_info});
    }

    for (const auto& loader : m_constituentsLoaders){
      auto [sequence_name, sequence_data, sequence_constituents] = loader->getData(vertex);
      gnn_input.insert({sequence_name, sequence_data});
      const std::string legacy_sequence_name = loader->getName();
      if (legacy_sequence_name != sequence_name) {
        gnn_input.insert({legacy_sequence_name, sequence_data});
      }
    }

    // run inference
    // -------------
    // FPE warning are hidden but should be resolved.
    // Related JIRA Ticket : https://its.cern.ch/jira/browse/ATLASRECTS-8386
    std::feclearexcept(FE_ALL_EXCEPT);

    const FlavorTagInference::InferenceOutput inference_output =
      m_saltModel->runInference(gnn_input);
    const auto& out_f = inference_output.singleFloat;

    // Get HSGNN score
    // ----------------
    float score = -999;
    FlavorTagInference::OutputConfig gnn_output_config = m_saltModel->getOutputConfig();

    for (const auto& outNode : gnn_output_config) {
      std::string score_name = outNode.name;

      if (outNode.name.find("_phsvertex") != std::string::npos) {
        score = out_f.at(score_name);
      }
    }
    std::feclearexcept(FE_ALL_EXCEPT);

    return score;
    
  } // end of decorate()

} // end of namespace InDetGNNHardScatterSelection

