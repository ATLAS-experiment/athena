/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNN.h"



TauGNN::TauGNN(const TauGNNDataLoader::Config &config):
        asg::AsgMessaging("TauGNN"),
        m_saltModel(std::make_shared<FlavorTagInference::SaltModel>(config.nnFile)),
        m_dataloader(TauGNNDataLoader(m_saltModel, config))
{
    ATH_MSG_INFO("TauGNN object initialized successfully!");
}

std::tuple<
        std::map<std::string, float>,
        std::map<std::string, std::vector<char>>,
        std::map<std::string, std::vector<float>> > 
TauGNN::compute(const xAOD::TauJet &tau) const {
    ATH_MSG_DEBUG("Computing TauGNN features...");
    auto salt_model_input_data = m_dataloader.loadInputs(&tau);
    // m_dataloader.DumpGnnInputs(salt_model_input_data.gnn_inputs);
    FlavorTagInference::InputMap input_with_aliases = salt_model_input_data.gnn_inputs;
    // Support both legacy loader keys and ONNX-hard-coded input tensor names.
    const std::map<std::string, std::string> alias_map = {
      {"jet_var", "jet_features"},
      {"tracks_r22default_sd0sort", "track_features"},
      {"cells_var", "cell_features"}
    };
    for (const auto& [legacy_name, onnx_name] : alias_map) {
      auto it = salt_model_input_data.gnn_inputs.find(legacy_name);
      if (it != salt_model_input_data.gnn_inputs.end()) {
        input_with_aliases[onnx_name] = it->second;
      }
    }
    ATH_MSG_DEBUG("Running inference...");
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(input_with_aliases);
    ATH_MSG_DEBUG("Inference done.");
    return std::make_tuple(out_f, out_vc, out_vf);
}
