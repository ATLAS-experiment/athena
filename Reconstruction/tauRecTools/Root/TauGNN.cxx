/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    auto node_is_hit = [&config](const FlavorTagInference::SaltModelGraphConfig::InputNodeConfig &in_node) {
        return in_node.name == config.input_layer_hits;
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

    auto hit_node = std::find_if(graph_config.input_sequences.cbegin(),
                                 graph_config.input_sequences.cend(),
                                 node_is_hit);

    // Check which input layers were found
    auto has_scalar_node = scalar_node != graph_config.inputs.cend();
    auto has_track_node = track_node != graph_config.input_sequences.cend();
    auto has_cluster_node = cluster_node != graph_config.input_sequences.cend();
    auto has_hit_node = hit_node != graph_config.input_sequences.cend();
    if(!has_scalar_node) ATH_MSG_DEBUG("No scalar node with name "<<config.input_layer_scalar<<" found!");
    if(!has_track_node) ATH_MSG_DEBUG("No track node with name "<<config.input_layer_tracks<<" found!");
    if(!has_cluster_node) ATH_MSG_DEBUG("No cluster node with name "<<config.input_layer_clusters<<" found!");
    if(!has_hit_node) ATH_MSG_DEBUG("No hit node with name "<<config.input_layer_hits<<" found!");
    
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

    if (has_hit_node) {
        for (const auto &in : hit_node->variables) {
            std::string name = in.name;
            m_hitCalc_inputs.push_back(name);
        }
    }

    // Load the variable calculator
    m_var_calc = TauGNNUtils::get_calculator(m_scalarCalc_inputs, m_trackCalc_inputs, m_clusterCalc_inputs, m_hitCalc_inputs);
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
    const std::vector<const xAOD::TrackMeasurementValidation*> empty_hits;
    return compute(tau, tracks, clusters, empty_hits);
}

std::tuple<
    std::map<std::string, float>,
    std::map<std::string, std::vector<char>>,
    std::map<std::string, std::vector<float>> >
TauGNN::compute(const xAOD::TauJet &tau,
		const std::vector<const xAOD::TauTrack *> &tracks,
		const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
        const std::vector<const xAOD::TrackMeasurementValidation*> &hits) const {
    InputMap scalarInputs;
    InputSequenceMap vectorInputs;
    std::map<std::string, Inputs> gnn_input;
    ATH_MSG_DEBUG("Starting compute...");
    //Prepare input variables
    if (!calculateInputVariables(tau, tracks, clusters, hits, scalarInputs, vectorInputs)) {
        ATH_MSG_FATAL("Failed calculateInputVariables");
        throw StatusCode::FAILURE;
    }

    // Add TauJet-level features to the input
    std::vector<float> tau_feats;
    for (const auto &varname : m_scalarCalc_inputs) {
        tau_feats.push_back(static_cast<float>(scalarInputs[m_config.input_layer_scalar][varname]));
    }
    std::vector<int64_t> tau_feats_dim = {1, static_cast<int64_t>(tau_feats.size())};
    Inputs tau_info (tau_feats, tau_feats_dim);
    gnn_input.insert({m_config.input_layer_scalar, tau_info});

    //Add track-level features to the input
    std::vector<float> trk_feats;
    int num_nodes=m_trackCalc_inputs.empty() ? 0 : static_cast<int>(vectorInputs[m_config.input_layer_tracks][m_trackCalc_inputs.at(0)].size());
    int num_node_vars=static_cast<int>(m_trackCalc_inputs.size());
    trk_feats.resize(num_nodes * num_node_vars);
    int var_idx=0;
    for (const auto &varname : m_trackCalc_inputs) {
        for (int node_idx=0; node_idx<num_nodes; node_idx++){    
            trk_feats.at(node_idx*num_node_vars + var_idx)
              = static_cast<float>(vectorInputs[m_config.input_layer_tracks][varname].at(node_idx));
        }
        var_idx++;
    }
    std::vector<int64_t> trk_feats_dim = {num_nodes, num_node_vars};
    Inputs trk_info (trk_feats, trk_feats_dim);
    gnn_input.insert({m_config.input_layer_tracks, trk_info});
    
    //Add cluster-level features to the input
    std::vector<float> cls_feats;
    num_nodes=m_clusterCalc_inputs.empty() ? 0 : static_cast<int>(vectorInputs[m_config.input_layer_clusters][m_clusterCalc_inputs.at(0)].size());
    num_node_vars=static_cast<int>(m_clusterCalc_inputs.size());
    cls_feats.resize(num_nodes * num_node_vars);
    var_idx=0;
    for (const auto &varname : m_clusterCalc_inputs) {
        for (int node_idx=0; node_idx<num_nodes; node_idx++){    
            cls_feats.at(node_idx*num_node_vars + var_idx)
              = static_cast<float>(vectorInputs[m_config.input_layer_clusters][varname].at(node_idx));
        }
        var_idx++;
    }
    std::vector<int64_t> cls_feats_dim = {num_nodes, num_node_vars};
    Inputs cls_info (cls_feats, cls_feats_dim);
    gnn_input.insert({m_config.input_layer_clusters, cls_info});

    //Add hit-level features to the input
    std::vector<float> hit_feats;
    num_nodes=m_hitCalc_inputs.empty() ? 0 : static_cast<int>(vectorInputs[m_config.input_layer_hits][m_hitCalc_inputs.at(0)].size());
    num_node_vars=static_cast<int>(m_hitCalc_inputs.size());
    hit_feats.resize(num_nodes * num_node_vars);
    var_idx=0;
    for (const auto &varname : m_hitCalc_inputs) {
        for (int node_idx=0; node_idx<num_nodes; node_idx++){    
            hit_feats.at(node_idx*num_node_vars + var_idx)
              = static_cast<float>(vectorInputs[m_config.input_layer_hits][varname].at(node_idx));
        }
        var_idx++;
    }
    std::vector<int64_t> hit_feats_dim = {num_nodes, num_node_vars};
    Inputs hit_info (hit_feats, hit_feats_dim);
    gnn_input.insert({m_config.input_layer_hits, hit_info});

    //RUN THE INFERENCE!!!
    ATH_MSG_DEBUG("Prepared inputs, running inference...");
    auto [out_f, out_vc, out_vf] = m_saltModel->runInference(gnn_input);
    ATH_MSG_DEBUG("Finished compute!");
    return std::make_tuple(out_f, out_vc, out_vf);
}

bool TauGNN::calculateInputVariables(const xAOD::TauJet &tau,
                  const std::vector<const xAOD::TauTrack *> &tracks,
                  const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
                  std::map<std::string, std::map<std::string, double>>& scalarInputs,
                  std::map<std::string, std::map<std::string, std::vector<double>>>& vectorInputs) const {
    const std::vector<const xAOD::TrackMeasurementValidation*> empty_hits;
    return calculateInputVariables(tau, tracks, clusters, empty_hits, scalarInputs, vectorInputs);
}

bool TauGNN::calculateInputVariables(const xAOD::TauJet &tau,
                  const std::vector<const xAOD::TauTrack *> &tracks,
                  const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
                  const std::vector<const xAOD::TrackMeasurementValidation*> &hits,
                  std::map<std::string, std::map<std::string, double>>& scalarInputs,
                  std::map<std::string, std::map<std::string, std::vector<double>>>& vectorInputs) const {
    scalarInputs.clear();
    vectorInputs.clear();
    // Populate input (sequence) map with input variables
    for (const auto &varname : m_scalarCalc_inputs) {
        if (!m_var_calc->compute(varname, tau,
                                 scalarInputs[m_config.input_layer_scalar][varname])) {
            ATH_MSG_WARNING("Error computing '" << varname
                            << "' returning default");
            return false;
        }
    }

    for (const auto &varname : m_trackCalc_inputs) {
        if (!m_var_calc->compute(varname, tau, tracks,
                                 vectorInputs[m_config.input_layer_tracks][varname])) {
            ATH_MSG_WARNING("Error computing '" << varname
                            << "' returning default");
            return false;
        }
    }

    for (const auto &varname : m_clusterCalc_inputs) {
        if (!m_var_calc->compute(varname, tau, clusters,
                                 vectorInputs[m_config.input_layer_clusters][varname])) {
            ATH_MSG_WARNING("Error computing '" << varname
                            << "' returning default");
            return false;
        }
    }

    for (const auto &varname : m_hitCalc_inputs) {
        if (!m_var_calc->compute(varname, tau, hits,
                                 vectorInputs[m_config.input_layer_hits][varname])) {
            ATH_MSG_WARNING("Error computing '" << varname
                            << "' returning default");
            return false;
        }
    }

    return true;
}
