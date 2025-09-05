/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/GNNDataLoader.h"

FlavorTagInference::GNNDataLoader::GNNDataLoader(std::shared_ptr<const SaltModel> saltModel, const GNNOptions& gnn_options) : 
  salt_model(saltModel),
  graph_config(saltModel->getGraphConfig()),
  m_gnn_options(gnn_options)
  {
    // Create configuration objects for data preprocessing.
    auto [inputs, constituents_configs, fo] = 
        dataprep::createGetterConfig<
            SaltModelGraphConfig::GraphConfig, 
            SaltModelGraphConfig::OutputNodeConfig
        > (
            graph_config, 
            m_gnn_options.flip_config, 
            m_gnn_options.variable_remapping
        );

    for (auto config : constituents_configs){
      switch (config.type){
      using enum ConstituentsType;
      case TRACK:
        constituents_loaders.push_back(std::make_shared<TracksLoader>(config, fo));
        break;
      case FLOW_ELEMENT:
        constituents_loaders.push_back(std::make_shared<FlowElementsLoader>(config, fo));
        break;
      case HIT:
        constituents_loaders.push_back(std::make_shared<HitsLoader>(config, fo));
        break;
      case ELECTRON:
        constituents_loaders.push_back(std::make_shared<ElectronsLoader>(config, fo));
        break;
      default:
        throw std::runtime_error("Unknown constituent type");
      }
    }
    // Initialize jet and b-tagging input getters.
    auto [vb, vj, ds] = dataprep::createBvarGetters(inputs);
    vars_from_jet = vj;
    data_dependency_names = ds;
    ftag_options = std::move(fo);
  }

FlavorTagInference::SaltModelData FlavorTagInference::GNNDataLoader::loadInputs(const xAOD::IParticle* p) const{
    auto jet = dynamic_cast<const xAOD::Jet*>(p);

    SaltModelData salt_model_data;
    // jet level inputs
    std::vector<float> jet_feat;
    for (const auto& getter: vars_from_jet) {
      jet_feat.push_back(getter(*jet).second);
    }
    std::vector<int64_t> jet_feat_dim = {1, static_cast<int64_t>(jet_feat.size())};
    Inputs jet_info(jet_feat, jet_feat_dim);
    if (salt_model->getSaltModelVersion() == SaltModelVersion::V2) {
      salt_model_data.gnn_inputs.insert({"jets", jet_info});
    } else {
      salt_model_data.gnn_inputs.insert({"jet_features", jet_info});
    }

    // constituent level inputs
    for (const auto& loader : constituents_loaders){
      auto [input_name, input_data, input_objects] = loader->getData(*jet);
      if (salt_model->getSaltModelVersion() != SaltModelVersion::V2) {
        input_name.pop_back();
        input_name.append("_features");
      }
      salt_model_data.gnn_inputs.insert({input_name, input_data});
      salt_model_data.num_inputs += input_data.first.size();
      salt_model_data.constituents[input_name] = input_objects;
    }
    return salt_model_data;
}