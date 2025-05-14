/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/SaltModelGraphConfig.h"

namespace FlavorTagInference { 
    namespace SaltModelGraphConfig {
        Input get_input(const nlohmann::json& v) {
            std::string name = v.at("name").get<std::string>();
            auto offset = v.at("offset").get<double>();
            auto scale = v.at("scale").get<double>();
            return {name, offset, scale};
        }

        InputNodeConfig get_input_node(const nlohmann::json& v) {
            InputNodeConfig cfg;
            cfg.name = v.at("name").get<std::string>();
            for (const auto& var: v.at("variables")) {
                cfg.variables.push_back(get_input(var));
                if (var.contains("default")) {
                    std::string name = var.at("name").get<std::string>();
                    cfg.defaults.emplace(name, var.at("default").get<double>());
                }
            }
            return cfg;
        }

        OutputNodeConfig get_output_node(const nlohmann::json& v) {
            OutputNodeConfig cfg;
            for (const auto& lab: v.at("labels")) {
            cfg.labels.push_back(lab.get<std::string>());
            }
            int idx = v.at("node_index").get<int>();
            if (idx < 0) throw std::logic_error("output node index is negative");
            cfg.node_index = idx;
            return cfg;
        }

        GraphConfig parse_json_graph(const nlohmann::json& metadata) {
            GraphConfig cfg;
            for (const auto& v: metadata.at("inputs")) {
            cfg.inputs.push_back(get_input_node(v));
            }
            for (const auto& v: metadata.at("input_sequences")) {
            cfg.input_sequences.push_back(get_input_node(v));
            }
            if (metadata.contains("outputs")) {
                for (const auto& v: metadata.at("outputs").items()) {
                    std::string name = v.key();
                    cfg.outputs.emplace(name, get_output_node(v.value()));
                }
            }
            return cfg;
        }
    }
}