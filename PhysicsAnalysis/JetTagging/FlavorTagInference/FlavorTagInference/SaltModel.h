/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This class acts as the interface to an ONNX model. It handles loading model
  the model, initializing the ORT session, and running inference. It is decoupled
  from the ATLAS EDM as much as possible. The FlavorTagInference::GNN class
  handles the interaction with the ATLAS EDM.
*/

#ifndef FLAVORTAGINFERENCE_SALTMODEL_H
#define FLAVORTAGINFERENCE_SALTMODEL_H

#include <onnxruntime_cxx_api.h>

#include "FlavorTagInference/ISaltModel.h"

namespace FlavorTagInference {

  //
  // Utility class that loads the onnx model from the given path
  // and runs inference based on the user given inputs

  class SaltModel final : public ISaltModel
  {
    public:
      SaltModel(const std::string& path_to_onnx);

      virtual InferenceOutput runInference(std::map<std::string, Inputs>& gnn_inputs) const override;

      virtual const SaltModelGraphConfig::GraphConfig getGraphConfig() const override;
      virtual const OutputConfig& getOutputConfig() const override;
      virtual SaltModelVersion getSaltModelVersion() const override;
      virtual const std::string& getModelName() const override;

    private:
      const nlohmann::json loadMetadata(const std::string& key) const;
      const std::string determineModelName() const;

      nlohmann::json m_metadata;

      std::unique_ptr< Ort::Session > m_session;
      std::unique_ptr< Ort::Env > m_env;

      size_t m_num_inputs;
      size_t m_num_outputs;
      std::string m_model_name;
      std::vector<std::string> m_input_node_names;
      OutputConfig m_output_nodes;

      SaltModelVersion m_onnx_model_version = SaltModelVersion::UNKNOWN;

  }; // Class SaltModel
} // end of FlavorTagInference namespace
#endif //FLAVORTAGDISCRIMINANTS_SALTMODEL_H

