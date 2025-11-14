/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "CxxUtils/checker_macros.h"

#include <stdexcept>
#include <tuple>
#include <set>

namespace FlavorTagInference {

  SaltModel::SaltModel(const std::string& path_to_onnx)
    //load the onnx model to memory using the path m_path_to_onnx
    : m_env (std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_FATAL, ""))
  {
    // initialize session options
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);

    // Ignore all non-fatal errors. This isn't a good idea, but it's
    // what we get for uploading semi-working graphs.
    session_options.SetLogSeverityLevel(4);
    session_options.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    // this should reduce memory use while slowing things down slightly
    // see
    //
    // https://github.com/microsoft/onnxruntime/issues/11627#issuecomment-1137668551
    //
    // and also https://its.cern.ch/jira/browse/AFT-818
    //
    session_options.DisableCpuMemArena();

    // declare an allocator with default options
    Ort::AllocatorWithDefaultOptions allocator;

    // create session and load model into memory
    m_session = std::make_unique<Ort::Session>(
      *m_env, path_to_onnx.c_str(), session_options);

    // get metadata from the onnx model
    m_metadata = loadMetadata("gnn_config");
    m_num_inputs = m_session->GetInputCount();
    m_num_outputs = m_session->GetOutputCount();

    // get the onnx model version
    if (m_metadata.contains("onnx_model_version")) { // metadata version is explicitly set
      m_onnx_model_version = m_metadata["onnx_model_version"].get<SaltModelVersion>();
      if (m_onnx_model_version == SaltModelVersion::UNKNOWN){
        throw std::runtime_error("Unknown Onnx model version!");
      }
    } else { // metadata version is not set, infer from the presence of "outputs" key
      if (m_metadata.contains("outputs")){
        m_onnx_model_version = SaltModelVersion::V0;
      } else {
        throw std::runtime_error("Onnx model version not found in metadata");
      }
    }

    // get the model name
    m_model_name = determineModelName();

    // iterate over input nodes and get their names
    for (size_t i = 0; i < m_num_inputs; i++) {
      m_input_node_names.push_back(m_session->GetInputNameAllocated(i, allocator).get());
     }

    // iterate over output nodes and get their configuration
    for (size_t i = 0; i < m_num_outputs; i++) {
      const auto name = std::string(m_session->GetOutputNameAllocated(i, allocator).get());
      const auto type = m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType();
      const int rank = m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape().size();
      if (m_onnx_model_version == SaltModelVersion::V0) {
        m_output_nodes.emplace_back(name, type, m_model_name);
      } else {
        m_output_nodes.emplace_back(name, type, rank);
      }
    }
  }

  const nlohmann::json SaltModel::loadMetadata(const std::string& key) const {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata modelMetadata = m_session->GetModelMetadata();
    std::string metadataString(modelMetadata.LookupCustomMetadataMapAllocated(key.c_str(), allocator).get());
    return nlohmann::json::parse(metadataString);
  }

  const std::string SaltModel::determineModelName() const {
    Ort::AllocatorWithDefaultOptions allocator;
    if (m_onnx_model_version == SaltModelVersion::V0) {
      // get the model name directly from the metadata
      return std::string(m_metadata["outputs"].begin().key());
    } else {
      // get the model name from the output node names
      // each output node name is of the form "<model_name>_<output_name>"
      std::set<std::string> model_names;
      for (size_t i = 0; i < m_num_outputs; i++) {
        const auto name = std::string(m_session->GetOutputNameAllocated(i, allocator).get());
        size_t underscore_pos = name.find('_');
        if (underscore_pos != std::string::npos) {
          model_names.insert(name.substr(0, underscore_pos));
        } else {
          return std::string("UnknownModelName");
        }
      }
      if (model_names.size() != 1) {
        throw std::runtime_error("SaltModel: model names are not consistent between outputs");
      }
      return *model_names.begin();
    }

  }

  const SaltModelGraphConfig::GraphConfig SaltModel::getGraphConfig() const {
    return SaltModelGraphConfig::parse_json_graph(m_metadata);
  }

  const nlohmann::json& SaltModel::getMetadata() const {
    return m_metadata;
  }

  const OutputConfig& SaltModel::getOutputConfig() const {
    return m_output_nodes;
  }

  SaltModelVersion SaltModel::getSaltModelVersion() const {
    return m_onnx_model_version;
  }

  const std::string& SaltModel::getModelName() const {
    return m_model_name;
  }


  InferenceOutput SaltModel::runInference(
    std::map<std::string, Inputs>& gnn_inputs) const {

    std::vector<float> input_tensor_values;

    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(
      OrtArenaAllocator, OrtMemTypeDefault
    );
    std::vector<Ort::Value> input_tensors;
    for (auto& node_name : m_input_node_names) {
      input_tensors.push_back(Ort::Value::CreateTensor<float>(
        memory_info, gnn_inputs.at(node_name).first.data(), gnn_inputs.at(node_name).first.size(),
        gnn_inputs.at(node_name).second.data(), gnn_inputs.at(node_name).second.size())
      );
    }

    // casting vector<string> to vector<const char*>. this is what ORT expects
    std::vector<const char*> input_node_names;
    input_node_names.reserve(m_input_node_names.size());
    for (const auto& name : m_input_node_names) {
      input_node_names.push_back(name.c_str());
    }
    std::vector<const char*> output_node_names;
    output_node_names.reserve(m_output_nodes.size());
    for (const auto& node : m_output_nodes) {
      output_node_names.push_back(node.name_in_model.c_str());
    }

    // score model & input tensor, get back output tensor
    // Although Session::Run is non-const, the onnx authors say
    // it is safe to call from multiple threads:
    //  https://github.com/microsoft/onnxruntime/discussions/10107
    Ort::Session& session ATLAS_THREAD_SAFE = *m_session;
    auto output_tensors = session.Run(Ort::RunOptions{nullptr},
      input_node_names.data(), input_tensors.data(), input_node_names.size(),
      output_node_names.data(), output_node_names.size()
    );

    // Extract outputs with improved clarity and structure
    InferenceOutput output;
    for (size_t node_idx = 0; node_idx < m_output_nodes.size(); ++node_idx) {
      const auto& output_node = m_output_nodes[node_idx];
      const auto& tensor = output_tensors[node_idx];
      auto tensor_type = tensor.GetTypeInfo().GetTensorTypeAndShapeInfo().GetElementType();
      auto tensor_shape = tensor.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
      int length = tensor.GetTensorTypeAndShapeInfo().GetElementCount();
      if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        if (tensor_shape.size() == 0) {
          output.singleFloat[output_node.name] = *tensor.GetTensorData<float>();
        } else if (tensor_shape.size() == 1) {
          const float* data = tensor.GetTensorData<float>();
          output.vecFloat[output_node.name] = std::vector<float>(data, data + length);
        } else {
          throw std::runtime_error("Unsupported tensor shape for FLOAT type");
        }
      } else if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8) {
        if (tensor_shape.size() == 1) {
          const char* data = tensor.GetTensorData<char>();
          output.vecChar[output_node.name] = std::vector<char>(data, data + length);
        } else {
          throw std::runtime_error("Unsupported tensor shape for INT8 type");
        }
      } else {
        throw std::runtime_error("Unsupported tensor type");
      }
    }

    return output;
  }

} // end of FlavorTagInference namespace
