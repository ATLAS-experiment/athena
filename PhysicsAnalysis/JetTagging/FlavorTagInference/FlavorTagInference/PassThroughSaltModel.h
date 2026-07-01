/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGINFERENCE_PASSTHROUGHSALTMODEL_H
#define FLAVORTAGINFERENCE_PASSTHROUGHSALTMODEL_H

#include "FlavorTagInference/ISaltModel.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "nlohmann/json.hpp"

#include <string>
#include <vector>
#include <map>

namespace FlavorTagInference {

  /// A pass-through "model" that copies jet and constituent inputs
  /// directly to named outputs. Configured via JSON specifying which
  /// variables to read and what to name the outputs.
  ///
  /// For scalar jet variables, copies input → singleFloat output.
  /// For constituent variables (tracks, electrons, muons, flows),
  /// unpacks the flat input tensor into per-variable vecFloat outputs.
  class PassThroughSaltModel : public ISaltModel {
  public:
    /// Construct from a JSON config with the format:
    /// {
    ///   "model_name": "PassThrough",
    ///   "jet_variables": [
    ///     {"input": "pt_calibrated", "output": "pt_calibrated"}
    ///   ],
    ///   "constituents": [
    ///     {
    ///       "node_name": "tracks_r22loose_sd0sort",
    ///       "input_key": "tracks",
    ///       "variables": [
    ///         {"input": "d0", "output": "trk_d0"},
    ///         ...
    ///       ]
    ///     }
    ///   ]
    /// }
    /// "input_key" must be one of: tracks, flows, hits, electrons, muons,
    /// clusters, towers — it is the key used to look up the pre-assembled
    /// constituent tensor produced by the matching *Loader.
    PassThroughSaltModel(const nlohmann::json& config);
    virtual ~PassThroughSaltModel() = default;

    InferenceOutput runInference(InputMap& gnn_inputs) const override;

    const SaltModelGraphConfig::GraphConfig getGraphConfig() const override;
    const OutputConfig& getOutputConfig() const override;
    SaltModelVersion getSaltModelVersion() const override;
    const std::string& getModelName() const override;

  private:
    std::string m_model_name;
    SaltModelGraphConfig::GraphConfig m_graph_config;
    OutputConfig m_output_config;

    /// Scalar jet variables: input names (graph config order)
    std::vector<std::string> m_jet_input_names;
    /// Scalar jet variables: output names (matching by index)
    std::vector<std::string> m_jet_output_names;

    /// Per-constituent node config
    struct ConstituentNode {
      std::string node_name;       ///< e.g. "tracks_r22loose_sd0sort" (drives ConstituentsLoader sort/select regex)
      std::string input_key;       ///< key in gnn_inputs, read from JSON "input_key" (e.g. "tracks", "flows")
      size_t num_vars = 0;         ///< number of variables in this node
      std::vector<std::string> output_names; ///< per-variable output names
      std::vector<std::string> var_types;  ///< per-variable: "float", "int", or "char"
    };
    std::vector<ConstituentNode> m_constituent_nodes;
  };

} // namespace FlavorTagInference

#endif
