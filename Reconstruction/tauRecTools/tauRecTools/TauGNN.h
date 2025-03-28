/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNN_H
#define TAURECTOOLS_TAUGNN_H

#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include "AsgMessaging/AsgMessaging.h"

#include "FlavorTagInference/SaltModel.h"

#include <memory>
#include <string>
#include <map>

namespace TauGNNUtils {
    class GNNVarCalc;
}

namespace FlavorTagInference{
    class SaltModel;
}

/**
 * @brief Wrapper around SaltModel to compute the output score of a model
 *
 *   Configures the network and computes the network outputs given the input
 *   objects. Retrieval of input variables is handled internally.
 *
 * @author N.M. Tamir
 *
 */
class TauGNN : public asg::AsgMessaging {
public:
    // Configuration of the weight file structure
    struct Config {
        std::string input_layer_scalar;
        std::string input_layer_tracks;
        std::string input_layer_clusters;
        std::string output_node_tau;
        std::string output_node_jet;
    };
    std::shared_ptr<const FlavorTagInference::SaltModel> m_saltModel;
public:
    TauGNN(const std::string &nnFile, const Config &config);
    ~TauGNN();

    // Output the SaltModel tuple 
    std::tuple<
        std::map<std::string, float>,
        std::map<std::string, std::vector<char>>,
        std::map<std::string, std::vector<float>> > 
    compute(const xAOD::TauJet &tau,
                  const std::vector<const xAOD::TauTrack *> &tracks,
                  const std::vector<xAOD::CaloVertexedTopoCluster> &clusters) const;

    // Compute all input variables and store them in the maps that are passed by reference
    bool calculateInputVariables(const xAOD::TauJet &tau,
                  const std::vector<const xAOD::TauTrack *> &tracks,
                  const std::vector<xAOD::CaloVertexedTopoCluster> &clusters,
                  std::map<std::string, std::map<std::string, double>>& scalarInputs,
                  std::map<std::string, std::map<std::string, std::vector<double>>>& vectorInputs) const;

    // Getter for the variable calculator
    const TauGNNUtils::GNNVarCalc* variable_calculator() const {
        return m_var_calc.get();
    }

    //Make the output config transparent to external tools
    FlavorTagInference::SaltModel::OutputConfig gnn_output_config;

private:
    using Inputs = FlavorTagInference::Inputs;
    // Abbreviations for lwtnn
    using VariableMap = std::map<std::string, double>;
    using VectorMap = std::map<std::string, std::vector<double>>;

    using InputMap = std::map<std::string, VariableMap>;
    using InputSequenceMap = std::map<std::string, VectorMap>;

private:
    const Config m_config;

    // Names of the input variables
    std::vector<std::string> m_scalar_inputs;
    std::vector<std::string> m_track_inputs;
    std::vector<std::string> m_cluster_inputs;
    // Names passed to the variable calculator
    std::vector<std::string> m_scalarCalc_inputs;
    std::vector<std::string> m_trackCalc_inputs;
    std::vector<std::string> m_clusterCalc_inputs;

    // Variable calculator to calculate input variables on the fly
    std::unique_ptr<TauGNNUtils::GNNVarCalc> m_var_calc;
};

#endif // TAURECTOOLS_TAUGNN_H
