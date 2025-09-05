/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauGNN.h"
#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "PathResolver/PathResolver.h"

#include <algorithm>
#include <fstream>

#include "tauRecTools/TauGNNUtils.h"

TauGNN::TauGNN(const std::string &nnFile, const Config &config):
    asg::AsgMessaging("TauGNN"),
    m_saltModel(std::make_shared<FlavorTagInference::SaltModel>(nnFile)),
    m_config{config}
  {
    //==================================================//
    // This part is ported from FTagDiscriminant GNN.cxx//
    //==================================================//

    // get the configuration of the model outputs
    FlavorTagInference::SaltModel::OutputConfig gnn_output_config = m_saltModel->getOutputConfig();
    
    //Let's see the output!
    for (const auto& out_node: gnn_output_config) {
        if(out_node.type==FlavorTagInference::SaltModelOutput::OutputType::FLOAT) ATH_MSG_INFO("Found output FLOAT node named:" << out_node.name);
        if(out_node.type==FlavorTagInference::SaltModelOutput::OutputType::VECCHAR) ATH_MSG_INFO("Found output VECCHAR node named:" << out_node.name);
        if(out_node.type==FlavorTagInference::SaltModelOutput::OutputType::VECFLOAT) ATH_MSG_INFO("Found output VECFLOAT node named:" << out_node.name);
    }

    //Get model config (for inputs)
    auto graph_config = m_saltModel->getGraphConfig();
    
    //===================================================//
    // This part is ported from tauRecTools TauJetRNN.cxx//
    //===================================================//

    // Search for input layer names specified in 'config'
    auto node_is_scalar = [&config](const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig &in_node) {
        return in_node.name == config.input_layer_scalar;
    };
    auto node_is_track = [&config](const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig &in_node) {
        return in_node.name == config.input_layer_tracks;
    };
    auto node_is_cluster = [&config](const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig &in_node) {
        return in_node.name == config.input_layer_clusters;
    };

    auto scalar_node = std::find_if(graph_config.inputs.cbegin(),
                                    graph_config.inputs.cend(),
                                    node_is_scalar);

    auto track_node = std::find_if(graph_config.input_sequences.cbegin(),
                                   graph_config.input_sequences.cend(),
                                   node_is_track);

    auto cluster_node = std::find_if(graph_config.input_sequences.cbegin(),
                                     graph_config.input_sequences.cend(),
                                     node_is_cluster);

    // Check which input layers were found
    auto has_scalar_node = scalar_node != graph_config.inputs.cend();
    auto has_track_node = track_node != graph_config.input_sequences.cend();
    auto has_cluster_node = cluster_node != graph_config.input_sequences.cend();
    if(!has_scalar_node) ATH_MSG_WARNING("No scalar node with name "<<config.input_layer_scalar<<" found!");
    if(!has_track_node) ATH_MSG_WARNING("No track node with name "<<config.input_layer_tracks<<" found!");
    if(!has_cluster_node) ATH_MSG_WARNING("No cluster node with name "<<config.input_layer_clusters<<" found!");
    
    // Fill the variable names of each input layer into the corresponding vector
    if (has_scalar_node) {
        for (const auto &in : scalar_node->variables) {
            std::string name = in.name;
            m_scalarCalc_inputs.push_back(name);
        }
    }

    if (has_track_node) {
        for (const auto &in : track_node->variables) {
            std::string name = in.name;
            m_trackCalc_inputs.push_back(name);
        }
    }

    if (has_cluster_node) {
        for (const auto &in : cluster_node->variables) {
            std::string name = in.name;
            m_clusterCalc_inputs.push_back(name);
        }
    }
    // Load the variable calculator
    m_var_calc = std::make_unique<TauGNNUtils::GNNVarCalc>();
    ATH_MSG_INFO("TauGNN object initialized successfully!");
}

TauGNN::~TauGNN() {}

std::tuple<
    std::map<std::string, float>,
    std::map<std::string, std::vector<char>>,
    std::map<std::string, std::vector<float>> >
TauGNN::compute(const xAOD::TauJet &tau,
		const std::vector<const xAOD::TauTrack *> &tracks,
		const std::vector<xAOD::CaloVertexedTopoCluster> &clusters) const {
    std::map<std::string, Inputs> gnn_input;
    ATH_MSG_DEBUG("Starting compute...");
    //Prepare input variables
    auto [tau_feats, trk_feats, cls_feats] = calculateInputVariables(tau, tracks, clusters);

    std::vector<int64_t> tau_feats_dim = {static_cast<int64_t>(1),               static_cast<int64_t>(tau_feats.size())};
    std::vector<int64_t> trk_feats_dim = {static_cast<int64_t>(tracks.size()),   static_cast<int64_t>(m_trackCalc_inputs.size())};
    std::vector<int64_t> cls_feats_dim = {static_cast<int64_t>(clusters.size()), static_cast<int64_t>(m_clusterCalc_inputs.size())};

    Inputs tau_info (tau_feats, tau_feats_dim);
    Inputs trk_info (trk_feats, trk_feats_dim);
    Inputs cls_info (cls_feats, cls_feats_dim);

    gnn_input.insert({"tau_vars", tau_info});
    gnn_input.insert({"track_vars", trk_info});
    gnn_input.insert({"cluster_vars", cls_info}); 

    //RUN THE INFERENCE!!!
    ATH_MSG_DEBUG("Prepared inputs, running inference...");
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input);
    ATH_MSG_DEBUG("Finished compute!");
    return std::make_tuple(out_f, out_vc, out_vf);
}

std::tuple<std::vector<float>, std::vector<float>, std::vector<float>>
    TauGNN::calculateInputVariables(
        const xAOD::TauJet &tau,
        const std::vector<const xAOD::TauTrack *> &tracks,
        const std::vector<xAOD::CaloVertexedTopoCluster> &clusters
    ) const {
    // Populate input (sequence) map with input variables
    std::vector<float> tau_feats;
    std::vector<std::vector<float>> track_feats_2d, cluster_feats_2d;
    for (const auto &varname : m_scalarCalc_inputs) {
        tau_feats.push_back(m_var_calc->compute(varname, tau));
    }
    for (const auto &varname : m_trackCalc_inputs) {
        track_feats_2d.push_back(m_var_calc->compute(varname, tau, tracks));
    }
    for (const auto &varname : m_clusterCalc_inputs) {
        cluster_feats_2d.push_back(m_var_calc->compute(varname, tau, clusters));
    }
    //transposing the 2d feature arrays
    std::vector<float> track_feats   = flatten(track_feats_2d);
    std::vector<float> cluster_feats = flatten(cluster_feats_2d);
    return std::make_tuple(tau_feats, track_feats, cluster_feats);
}