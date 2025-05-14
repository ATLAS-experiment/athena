/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

#include <fstream>

namespace InDetGNNHardScatterSelection {

  GNN::GNN(const std::string& nn_file):
    m_saltModel(nullptr)
  {

    // Load and initialize the neural network model from the given file path.
    std::string fullPathToOnnxFile = PathResolverFindCalibFile(nn_file);
    m_saltModel = std::make_shared<FlavorTagInference::SaltModel>(fullPathToOnnxFile);

    // Extract metadata from the ONNX file, primarily about the model's inputs.
    auto graph_config = m_saltModel->getGraphConfig();

    // Create configuration objects for data preprocessing.
    auto [inputs, constituents_configs] = dataprep::createGetterConfig(graph_config);
    
    for (auto config : constituents_configs){
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
    FlavorTagInference::SaltModel::OutputConfig gnn_output_config = m_saltModel->getOutputConfig();

    for (const auto& outNode : gnn_output_config) {
      // the node's output name will be used to define the decoration name
      std::string dec_name = outNode.name;
      m_decorators.vertexFloat.emplace_back(outNode.name, Dec<float>(dec_name));
    }
  }

  GNN::GNN(GNN&&) = default;
  GNN::GNN(const GNN&) = default;
  GNN::~GNN() = default;

  void GNN::decorate(const xAOD::Vertex& vertex) const {
    /* Main function for decorating a vertex with GNN outputs. */
    using namespace internal;

    // prepare input
    // -------------
    std::map<std::string, FlavorTagInference::Inputs> gnn_input;

    std::vector<float> vertex_feat;
    for (const auto& getter: m_varsFromVertex) {
      vertex_feat.push_back(getter(vertex).second);
    }
    std::vector<int64_t> vertexfeat_dim = {1, static_cast<int64_t>(vertex_feat.size())};

    FlavorTagInference::Inputs vertex_info (vertex_feat, vertexfeat_dim);
    gnn_input.insert({"vertex_features", vertex_info});

    for (auto loader : m_constituentsLoaders){
      auto [sequence_name, sequence_data, sequence_constituents] = loader->getData(vertex);
      gnn_input.insert({sequence_name, sequence_data});
    }

    // run inference
    // -------------
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input);

    // decorate outputs
    // ----------------
    for (const auto& dec: m_decorators.vertexFloat) {
      dec.second(vertex) = out_f.at(dec.first);
    }
  } // end of decorate()

} // end of namespace InDetGNNHardScatterSelection

