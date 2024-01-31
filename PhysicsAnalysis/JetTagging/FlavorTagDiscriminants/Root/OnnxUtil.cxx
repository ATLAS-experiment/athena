/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "CxxUtils/checker_macros.h"
#include <stdexcept>
#include <tuple> //for std::make_tuple
#include "lwtnn/parse_json.hh"

namespace FlavorTagDiscriminants {

  OnnxUtil::OnnxUtil(const std::string& path_to_onnx)
    //load the onnx model to memory using the path m_path_to_onnx
    : m_env (std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_FATAL, ""))
  {
    // initialize session options
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);

    // Ignore all non-fatal errors. This isn't a good idea, but it's
    // what we get for uploading semi-working graphs.
    session_options.SetLogSeverityLevel(4);
    session_options.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // create session and load model into memory
    m_session = std::make_unique< Ort::Session >(*m_env, path_to_onnx.c_str(),
                                                 session_options);

    // get metadata from the onnx model
    m_metadata = loadMetadata("gnn_config");

    // get the onnx model version
    if (m_metadata.contains("onnx_model_version")) { // metadata version is explicitly set
      m_onnx_model_version = m_metadata["onnx_model_version"].get<OnnxModelVersion>();
      if (m_onnx_model_version == OnnxModelVersion::UNKNOWN){
        throw std::runtime_error("Unknown Onnx model version!");
      }
    } else { // metadata version is not set, infer from the presence of "outputs" key
      if (m_metadata.contains("outputs")){
        m_onnx_model_version = OnnxModelVersion::V0;
      } else {
        throw std::runtime_error("Onnx model version not found in metadata");
      }
    }

    // iterate over input nodes and get their names
    size_t num_input_nodes = m_session->GetInputCount();
    for (std::size_t i = 0; i < num_input_nodes; i++) {
      auto input_name = m_session->GetInputNameAllocated(i, allocator);
      m_input_node_names.emplace_back(input_name.get());
     }

    // iterate over output nodes and get their configuration
    size_t num_output_nodes = m_session->GetOutputCount();
    for (std::size_t i = 0; i < num_output_nodes; i++) {
      const auto name = std::string(m_session->GetOutputNameAllocated(i, allocator).get());
      const auto type = m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType();
      const int rank = m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape().size();
      if (m_onnx_model_version == OnnxModelVersion::V0) {
        const OnnxOutput onnxOutput(name, type, m_metadata["outputs"].begin().key());
        m_output_nodes.push_back(onnxOutput);
      } else {
        const OnnxOutput onnxOutput(name, type, rank);
        m_output_nodes.push_back(onnxOutput);
      }
    }
  }

  nlohmann::json OnnxUtil::loadMetadata(const std::string& key) const {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata modelMetadata = m_session->GetModelMetadata();
    std::string metadataString(modelMetadata.LookupCustomMetadataMapAllocated(key.c_str(), allocator).get());
    return nlohmann::json::parse(metadataString);
  }

  const lwt::GraphConfig OnnxUtil::getLwtConfig() const {
    /* for the new metadata format (>V0), the outputs are inferred directly from
    the model graph, rather than being configured as json metadata.
    however we still need to add an empty "outputs" key to the config so that
    lwt::parse_json_graph doesn't throw an exception */

    // deep copy the metadata by round tripping through a string stream
    nlohmann::json metadataCopy = nlohmann::json::parse(m_metadata.dump());
    if (getOnnxModelVersion() != OnnxModelVersion::V0){
      metadataCopy["outputs"] = nlohmann::json::object();
    }
    std::stringstream metadataStream;
    metadataStream << metadataCopy.dump();
    return lwt::parse_json_graph(metadataStream);
  }

  const nlohmann::json& OnnxUtil::getMetadata() const {
    return m_metadata;
  }

  const OnnxUtil::OutputConfig& OnnxUtil::getOutputConfig() const {
    return m_output_nodes;
  }

  OnnxModelVersion OnnxUtil::getOnnxModelVersion() const {
    return m_onnx_model_version;
  }

  std::tuple<
    std::map<std::string, float>,
    std::map<std::string, std::vector<char>>,
    std::map<std::string, std::vector<float>> >
  OnnxUtil::runInference(
    std::map<std::string, input_pair>& gnn_inputs) const {
    // Args:
    //    gnn_inputs : {string: input_pair}

    std::vector<float> input_tensor_values;

    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(
      OrtArenaAllocator, OrtMemTypeDefault
    );
    std::vector<Ort::Value> input_tensors;
    for (auto const &node_name : m_input_node_names){
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

    // extract outputs
    std::map<std::string, float> output_f;
    std::map<std::string, std::vector<char>> output_vc;
    std::map<std::string, std::vector<float>> output_vf;
    for (unsigned int node_idx=0; node_idx<m_output_nodes.size(); node_idx++){

      auto tensor_type = 
        output_tensors.at(node_idx).GetTypeInfo().GetTensorTypeAndShapeInfo().GetElementType();
      auto tensor_shape = 
        output_tensors.at(node_idx).GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();

      if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT){
        if (tensor_shape.size() == 0){
          output_f.insert({m_output_nodes[node_idx].name,
            output_tensors.at(node_idx).GetTensorData<float>()[0]});
        }
        else if (tensor_shape.size() == 1){
          const float *float_ptr = output_tensors.at(node_idx).GetTensorData<float>();
          int float_ptr_len = 
            output_tensors[node_idx].GetTensorTypeAndShapeInfo().GetElementCount();

          output_vf.insert({m_output_nodes[node_idx].name,
            {float_ptr, float_ptr + float_ptr_len}});
        }
        else{
          throw std::runtime_error("OnnxUtil::runInference: unsupported tensor shape");
        }
      } 
      else if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8){
        if (tensor_shape.size() == 1){
          const char *char_ptr = output_tensors.at(node_idx).GetTensorMutableData<char>();
          int char_ptr_len = 
            output_tensors[node_idx].GetTensorTypeAndShapeInfo().GetElementCount();

          output_vc.insert({m_output_nodes[node_idx].name,
            {char_ptr, char_ptr + char_ptr_len}});
        }
        else{
          throw std::runtime_error("OnnxUtil::runInference: unsupported tensor shape");
        }
      } else{
        throw std::runtime_error("OnnxUtil::runInference: unsupported tensor type");
      }
    }

    return std::make_tuple(output_f, output_vc, output_vf);
  }

} // end of FlavorTagDiscriminants namespace
