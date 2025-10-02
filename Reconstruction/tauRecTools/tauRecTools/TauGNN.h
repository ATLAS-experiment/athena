/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
public:
    TauGNN(const std::string &nnFile, const Config &config, bool useTRT);
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
    std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> calculateInputVariables(
        const xAOD::TauJet &tau,
        const std::vector<const xAOD::TauTrack *> &tracks,
        const std::vector<xAOD::CaloVertexedTopoCluster> &clusters
    ) const;

    // Getter for the variable calculator
    const TauGNNUtils::GNNVarCalc* variable_calculator() const {
        return m_var_calc.get();
    }

    //Make the output config transparent to external tools
    FlavorTagInference::OutputConfig gnn_output_config;

private:
    using Inputs = FlavorTagInference::Inputs;
    // Abbreviations for lwtnn
    using VariableMap = std::map<std::string, double>;
    using VectorMap = std::map<std::string, std::vector<double>>;

    using InputMap = std::map<std::string, VariableMap>;
    using InputSequenceMap = std::map<std::string, VectorMap>;

private:
    std::shared_ptr<const FlavorTagInference::SaltModel> m_saltModel;
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
    bool m_useTRT = true;

    std::vector<float> flatten(const std::vector<std::vector<float>>& mat) const {
        std::vector<float> flat;
        for (size_t col = 0; col < mat[0].size(); col++){
            for (size_t row = 0; row < mat.size(); row++){
                flat.push_back(mat[row][col]);
            }
        }
        return flat;
    };
};

#endif // TAURECTOOLS_TAUGNN_H
