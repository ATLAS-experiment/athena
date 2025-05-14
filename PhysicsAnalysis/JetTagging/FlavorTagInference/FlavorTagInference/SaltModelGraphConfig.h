/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
# pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>

namespace FlavorTagInference {
    namespace SaltModelGraphConfig {
        struct Input
        {
            std::string name;
            double offset;
            double scale;
        };

        struct InputNodeConfig
        {
            std::string name;
            std::vector<Input> variables;
            std::map<std::string, std::string> miscellaneous;
            std::map<std::string, double> defaults;
        };

        struct OutputNodeConfig
        {
            std::vector<std::string> labels;
            std::size_t node_index;
        };

        struct GraphConfig
        {
            std::vector<InputNodeConfig> inputs;
            std::vector<InputNodeConfig> input_sequences;
            std::map<std::string, OutputNodeConfig> outputs;
        };

        Input get_input(const nlohmann::json& v);
        InputNodeConfig get_input_node(const nlohmann::json& v);
        OutputNodeConfig get_output_node(const nlohmann::json& v);
        GraphConfig parse_json_graph(const nlohmann::json& metadata);
    }
}