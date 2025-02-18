/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This class acts as the interface to an ONNX model. It handles loading model
  the model, initializing the ORT session, and running inference. It is decoupled
  from the ATLAS EDM as much as possible. The FlavorTagDiscriminants::GNN class
  handles the interaction with the ATLAS EDM.
*/

#ifndef FLAVORTAGDISCRIMINANTS_SALTMODEL_H
#define FLAVORTAGDISCRIMINANTS_SALTMODEL_H

#include <onnxruntime_cxx_api.h>

#include "nlohmann/json.hpp"
#include "lwtnn/parse_json.hh"

#include "FlavorTagDiscriminants/SaltModelOutput.h"

#include <map> //also has std::pair
#include <vector>
#include <string>
#include <memory>

namespace FlavorTagDiscriminants {

  // the first element is the input data, the second is the shape
  using Inputs = std::pair<std::vector<float>, std::vector<int64_t>>;

  enum class SaltModelVersion{UNKNOWN, V0, V1, V2};

  NLOHMANN_JSON_SERIALIZE_ENUM( SaltModelVersion , {
    { SaltModelVersion::UNKNOWN, "" },
    { SaltModelVersion::V0, "v0" },
    { SaltModelVersion::V1, "v1" },
    { SaltModelVersion::V2, "v2" },
  })

  //
  // Utility class that loads the onnx model from the given path
  // and runs inference based on the user given inputs

  class SaltModel final{

    public:
      using OutputConfig = std::vector<SaltModelOutput>;

      SaltModel(const std::string& path_to_onnx);

      void initialize();

      struct InferenceOutput {
        std::map<std::string, float> singleFloat;
        std::map<std::string, std::vector<char>> vecChar;
        std::map<std::string, std::vector<float>> vecFloat;
      };

      InferenceOutput runInference(std::map<std::string, Inputs>& gnn_inputs) const;

      const lwt::GraphConfig getLwtConfig() const;
      const nlohmann::json& getMetadata() const;
      const OutputConfig& getOutputConfig() const;
      SaltModelVersion getSaltModelVersion() const;
      const std::string& getModelName() const;

    private:
      const nlohmann::json loadMetadata(const std::string& key) const;
      const std::string determineModelName() const;

      nlohmann::json m_metadata;
      std::string m_path_to_onnx;

      std::unique_ptr< Ort::Session > m_session;
      std::unique_ptr< Ort::Env > m_env;

      size_t m_num_inputs;
      size_t m_num_outputs;
      std::string m_model_name;
      std::vector<std::string> m_input_node_names;
      OutputConfig m_output_nodes;

      SaltModelVersion m_onnx_model_version = SaltModelVersion::UNKNOWN;

  }; // Class SaltModel
} // end of FlavorTagDiscriminants namespace
#endif //FLAVORTAGDISCRIMINANTS_SALTMODEL_H
