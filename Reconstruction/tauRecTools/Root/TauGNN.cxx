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
    ATH_MSG_DEBUG("Running inference...");
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(salt_model_input_data.gnn_inputs);
    ATH_MSG_DEBUG("Inference done.");
    return std::make_tuple(out_f, out_vc, out_vf);
}
