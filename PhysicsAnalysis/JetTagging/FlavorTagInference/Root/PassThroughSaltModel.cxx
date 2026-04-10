/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/PassThroughSaltModel.h"
#include <stdexcept>

namespace FlavorTagInference {

  PassThroughSaltModel::PassThroughSaltModel(const nlohmann::json& config)
    : m_model_name(config.value("model_name", "PassThrough")) {

    // ── Scalar jet variables ──
    // Support both "variables" (legacy) and "jet_variables" keys
    const char* jet_key = config.contains("jet_variables")
      ? "jet_variables" : "variables";

    if (config.contains(jet_key) && config[jet_key].is_array()) {
      SaltModelGraphConfig::InputNodeConfig input_node;
      input_node.name = "jets";

      for (const auto& var : config[jet_key]) {
        std::string input_name = var.at("input").get<std::string>();
        std::string output_name = var.at("output").get<std::string>();

        m_jet_input_names.push_back(input_name);
        m_jet_output_names.push_back(output_name);

        // Graph config: identity normalisation (offset=0, scale=1)
        input_node.variables.emplace_back(input_name,0.0,1.0);

        // Output config: scalar float (rank 0)
        m_output_config.emplace_back(
          output_name,
          ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
          0  // rank 0 = scalar float
        );
      }

      if (!input_node.variables.empty()) {
        m_graph_config.inputs.push_back(std::move(input_node));
      }
    }

    // ── Constituent variables (tracks, electrons, muons, flows) ──
    if (config.contains("constituents") && config["constituents"].is_array()) {
      for (const auto& cnode : config["constituents"]) {
        std::string node_name = cnode.at("node_name").get<std::string>();

        if (!cnode.contains("variables") || !cnode["variables"].is_array()) {
          throw std::runtime_error(
            "PassThroughSaltModel: constituent node '" + node_name
            + "' must have a 'variables' array");
        }

        ConstituentNode cn;
        cn.node_name = node_name;

        // Build graph config input_sequence for this constituent type
        SaltModelGraphConfig::InputNodeConfig seq_node;
        seq_node.name = node_name;

        for (const auto& var : cnode["variables"]) {
          std::string input_name = var.at("input").get<std::string>();
          std::string output_name = var.at("output").get<std::string>();
          std::string var_type = var.value("type", "float");
          std::string cast = var.value("cast", "");
          float fp16_scale = var.value("scale", 1.0f);

          cn.output_names.push_back(output_name);
          cn.var_types.push_back(var_type);

          // Graph config: identity normalisation
          seq_node.variables.emplace_back(input_name, 0.0, 1.0);

          // Output config: vector type based on JSON "type"/"cast" fields
          if (cast == "fp16") {
            m_output_config.push_back(
              SaltModelOutput(output_name, SaltModelOutput::OutputType::VECFP16, fp16_scale));
          } else if (cast == "bf16") {
            m_output_config.push_back(
              SaltModelOutput(output_name, SaltModelOutput::OutputType::VECBF16, fp16_scale));
          } else {
            ONNXTensorElementDataType onnx_type;
            if (var_type == "int") {
              onnx_type = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
            } else if (var_type == "char") {
              onnx_type = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
            } else {
              onnx_type = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
            }
            m_output_config.emplace_back(output_name, onnx_type, 1);
          }
        }

        cn.num_vars = cn.output_names.size();
        cn.input_key = deriveInputKey(node_name);
        m_constituent_nodes.push_back(std::move(cn));
        m_graph_config.input_sequences.push_back(std::move(seq_node));
      }
    }

    if (m_jet_input_names.empty() && m_constituent_nodes.empty()) {
      throw std::runtime_error(
        "PassThroughSaltModel: JSON config must contain "
        "'jet_variables'/'variables' and/or 'constituents'");
    }
  }

  InferenceOutput PassThroughSaltModel::runInference(
    std::map<std::string, Inputs>& gnn_inputs) const
  {
    InferenceOutput output;

    // ── Scalar jet variables ──
    if (!m_jet_input_names.empty()) {
      auto it = gnn_inputs.find("jets");
      if (it == gnn_inputs.end()) {
        throw std::runtime_error(
          "PassThroughSaltModel: expected 'jets' in gnn_inputs");
      }

      const auto& data = it->second.first;   // vector<float>
      const auto& shape = it->second.second;  // vector<int64_t>

      if (shape.size() != 2 || shape[0] != 1) {
        throw std::runtime_error(
          "PassThroughSaltModel: unexpected jets shape");
      }

      size_t num_vars = static_cast<size_t>(shape[1]);
      if (num_vars != m_jet_input_names.size()) {
        throw std::runtime_error(
          "PassThroughSaltModel: jets size mismatch: got "
          + std::to_string(num_vars) + " expected "
          + std::to_string(m_jet_input_names.size()));
      }

      for (size_t i = 0; i < num_vars; ++i) {
        output.singleFloat[m_jet_output_names[i]] = data[i];
      }
    }

    // ── Constituent variables ──
    // The loaders produce flat tensors with shape [N_constituents, N_vars]
    // in row-major order: element [c, v] = data[c * N_vars + v].
    // We transpose back to per-variable vectors for the output.
    for (const auto& cn : m_constituent_nodes) {
      auto writeEmpty = [&](const ConstituentNode& node) {
        for (size_t v = 0; v < node.output_names.size(); ++v) {
          if (node.var_types[v] == "char") {
            output.vecChar[node.output_names[v]] = {};
          } else {
            output.vecFloat[node.output_names[v]] = {};
          }
        }
      };

      auto it = gnn_inputs.find(cn.input_key);
      if (it == gnn_inputs.end()) {
        writeEmpty(cn);
        continue;
      }

      const auto& data = it->second.first;
      const auto& shape = it->second.second;

      // shape = [N_constituents, N_vars]
      int64_t n_constituents = (shape.size() >= 1) ? shape[0] : 0;
      int64_t n_vars = (shape.size() >= 2) ? shape[1] : 0;

      if (n_constituents == 0 || n_vars == 0) {
        writeEmpty(cn);
        continue;
      }

      if (static_cast<size_t>(n_vars) != cn.num_vars) {
        throw std::runtime_error(
          "PassThroughSaltModel: constituent '" + cn.node_name
          + "' vars mismatch: got " + std::to_string(n_vars)
          + " expected " + std::to_string(cn.num_vars));
      }

      // Transpose: extract column v from the flat row-major tensor.
      // Float and int variables go to vecFloat (GNN converts int at
      // decoration time).  Char variables go to vecChar for 1-byte
      // storage (hit counts, quality flags).
      for (size_t v = 0; v < cn.num_vars; ++v) {
        if (cn.var_types[v] == "char") {
          std::vector<char> col(n_constituents);
          for (int64_t c = 0; c < n_constituents; ++c) {
            col[c] = static_cast<char>(data[c * n_vars + v]);
          }
          output.vecChar[cn.output_names[v]] = std::move(col);
        } else {
          std::vector<float> col(n_constituents);
          for (int64_t c = 0; c < n_constituents; ++c) {
            col[c] = data[c * n_vars + v];
          }
          output.vecFloat[cn.output_names[v]] = std::move(col);
        }
      }
    }

    return output;
  }

  const SaltModelGraphConfig::GraphConfig
  PassThroughSaltModel::getGraphConfig() const {
    return m_graph_config;
  }

  const OutputConfig& PassThroughSaltModel::getOutputConfig() const {
    return m_output_config;
  }

  SaltModelVersion PassThroughSaltModel::getSaltModelVersion() const {
    return SaltModelVersion::V2;
  }

  const std::string& PassThroughSaltModel::getModelName() const {
    return m_model_name;
  }

  std::string PassThroughSaltModel::deriveInputKey(
    const std::string& node_name)
  {
    // V2 naming: GNNDataLoader::getVecInputName() returns the raw
    // output_name from ConstituentsLoader (e.g. "tracks", "flows").
    // We match the node_name substring to the same base names.
    static const std::pair<const char*, const char*> types[] = {
      {"tracks",    "tracks"},
      {"flows",     "flows"},
      {"hits",      "hits"},
      {"electrons", "electrons"},
      {"muons",     "muons"},
      {"clusters",  "clusters"},
      {"towers",    "towers"},
    };
    for (const auto& [substr, key] : types) {
      if (node_name.find(substr) != std::string::npos) {
        return key;
      }
    }
    throw std::runtime_error(
      "PassThroughSaltModel: cannot determine constituent type from "
      "node_name '" + node_name + "'. Must contain one of: "
      "tracks, flows, hits, electrons, muons, clusters, towers");
  }

} // namespace FlavorTagInference
