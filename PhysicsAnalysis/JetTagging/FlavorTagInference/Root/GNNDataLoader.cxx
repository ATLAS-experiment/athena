/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/GNNDataLoader.h"

FlavorTagInference::GNNDataLoader::GNNDataLoader(std::shared_ptr<const SaltModel> saltModel, const GNNOptions& gnn_options) : 
  SaltModelEDMLoaderBase(saltModel),
  m_gnn_options(gnn_options)
  {
    // Create configuration objects for data preprocessing.
    auto [inputs_config, constituents_configs, fo] = 
        dataprep::createGetterConfig<
            SaltModelGraphConfig::GraphConfig, 
            SaltModelGraphConfig::OutputNodeConfig
        > (
            graph_config, 
            m_gnn_options.flip_config, 
            m_gnn_options.variable_remapping
        );
    auto salt_model_version = salt_model->getSaltModelVersion();

    for (auto config : constituents_configs){
      switch (config.type){
      using enum ConstituentsType;
      case TRACK:
        addVectorLoader(getVecInputName(salt_model_version, config), std::make_shared<TracksLoader>(config, fo));
        break;
      case FLOW_ELEMENT:
        addVectorLoader(getVecInputName(salt_model_version, config), std::make_shared<FlowElementsLoader>(config, fo));
        break;
      case HIT:
        addVectorLoader(getVecInputName(salt_model_version, config), std::make_shared<HitsLoader>(config, fo));
        break;
      case ELECTRON:
        addVectorLoader(getVecInputName(salt_model_version, config), std::make_shared<ElectronsLoader>(config, fo));
        break;
      default:
        throw std::runtime_error("Unknown constituent type");
      }
    }
    // Initialize jet and b-tagging input getters.
    scalarInputName = (salt_model_version == SaltModelVersion::V2 ? "jets" : "jet_features");
    auto [vars_from_jet, ds] = dataprep::createBvarGetters(inputs_config);
    data_dependency_names = std::move(ds);
    ftag_options = std::move(fo);
    for (const auto& [name, getter]: vars_from_jet) {
      addScalarLoader(
        name,
        [getter](const xAOD::IParticle* p) { 
          auto jet = dynamic_cast<const xAOD::Jet*>(p);
          return getter(*jet).second; 
      });
    }
  }

std::string FlavorTagInference::GNNDataLoader::getVecInputName(const SaltModelVersion salt_model_version, const ConstituentsInputConfig& constituents_config) const {
  if (salt_model_version == SaltModelVersion::V2){
    return constituents_config.output_name;
  } else {
    auto out = constituents_config.output_name;
    out.pop_back();
    return out + "_features";
  }
}