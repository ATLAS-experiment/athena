/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/SaltModelTriton.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "FlavorTagInference/SaltModelOutput.h"
#include "CxxUtils/checker_macros.h"
#include <onnxruntime_cxx_api.h>

#include <stdexcept>
#include <tuple>
#include <set>
#include <memory>

// DType traits for Triton
template <typename T> struct TritonDType;
template <> struct TritonDType<float>    { static constexpr const char* value = "FP32"; };
template <> struct TritonDType<int64_t>  { static constexpr const char* value = "INT64"; };

template <typename T>
bool prepareInput(const std::string& name,
		  const std::vector<int64_t>& shape,
		  const std::vector<T>& data,
		  std::vector<std::shared_ptr<tc::InferInput>>& inputs)
{
  const char* dtype = TritonDType<T>::value;
  tc::InferInput* rawInputPtr = nullptr;

  // create the InferInput object with the predefined name, shape, and data type.
  tc::Error err = tc::InferInput::Create(&rawInputPtr, name, shape, dtype);
  if(!err.IsOk()) {
    std::cerr << "Unable to create input: " + name << std::endl;
    return false;
  }

  // Append tensor values for this input from a byte array.
  // Note: The vector is not copied and so it must not be modified or destroyed
  // until this input is no longer needed (that is until the Infer() call(s) that use the input have completed).
  // Multiple calls can be made to this API to keep adding tensor data for this input.
  // The data will be delivered in the order it was added.
  std::shared_ptr<tc::InferInput> input(rawInputPtr);
  err = input->AppendRaw(reinterpret_cast<const uint8_t*>(data.data()),
			 data.size() * sizeof(T));
  if(!err.IsOk()) {
    std::cerr << "Unable to set input data for: " + name << std::endl;
    return false;
  }

  inputs.push_back(std::move(input));
  return true;
}

template <typename T>
bool extractOutput(const std::string& name,
		   const std::shared_ptr<tc::InferResult>& result,
		   std::vector<T>& outputVec)
{
  const uint8_t* rawData = nullptr;
  size_t size = 0;

  // Get access to the buffer holding raw results of specified output returned by the server.
  // Note: the buffer is owned by InferResult instance.
  // Users can copy out the data if required to extend the lifetime.
  tc::Error err = result->RawData(name, &rawData, &size);
  if(!err.IsOk()) {
    std::cerr << "Unable to get raw output for: " + name << std::endl;
    return false;
  }

  outputVec.resize(size / sizeof(T));
  std::memcpy(outputVec.data(), rawData, size);
  return true;
}


namespace FlavorTagInference {

  SaltModelTriton::SaltModelTriton(const std::string& path_to_onnx
				   , const std::string& model_name
				   , float client_timeout
				   , int port
				   , const std::string& url
				   , bool useSSL)
    : m_model_name(model_name)
    , m_clientTimeout(client_timeout)
    , m_port(port)
    , m_url(url)
    , m_useSSL(useSSL)
    //load the onnx model to memory using the path m_path_to_onnx
  {
    std::unique_ptr< Ort::Env > env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_FATAL, "");

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
    std::unique_ptr< Ort::Session > session = std::make_unique<Ort::Session>(
      *env, path_to_onnx.c_str(), session_options);

    // get metadata from the onnx model
    m_metadata = loadMetadata("gnn_config", session.get());
    m_num_outputs = session->GetOutputCount();

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
    m_model_type = determineModelType(session.get());

    // iterate over output nodes and get their configuration
    for (size_t i = 0; i < m_num_outputs; i++) {
      const auto name = std::string(session->GetOutputNameAllocated(i, allocator).get());
      const auto type = session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType();
      const int rank = session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape().size();
      if (m_onnx_model_version == SaltModelVersion::V0) {
        const SaltModelOutput saltModelOutput(name, type, m_model_type);
        m_output_nodes.push_back(saltModelOutput);
      } else {
        const SaltModelOutput saltModelOutput(name, type, rank);
        m_output_nodes.push_back(saltModelOutput);
      }
    }

    m_options = std::make_unique<tc::InferOptions>(m_model_name);
    m_options->model_version_ = ""; // Fixme: find a proper solution for this
    m_options->client_timeout_ = m_clientTimeout;
  }

  const nlohmann::json SaltModelTriton::loadMetadata(const std::string& key, const Ort::Session* session) const {
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata modelMetadata = session->GetModelMetadata();
    std::string metadataString(modelMetadata.LookupCustomMetadataMapAllocated(key.c_str(), allocator).get());
    return nlohmann::json::parse(metadataString);
  }

  const std::string SaltModelTriton::determineModelType(const Ort::Session* session) const {
    Ort::AllocatorWithDefaultOptions allocator;
    if (m_onnx_model_version == SaltModelVersion::V0) {
      // get the model name directly from the metadata
      return std::string(m_metadata["outputs"].begin().key());
    }
    else {
      // get the model name from the output node names
      // each output node name is of the form "<model_type>_<output_name>"
      std::set<std::string> model_types;
      for (size_t i = 0; i < m_num_outputs; i++) {
        const auto name = std::string(session->GetOutputNameAllocated(i, allocator).get());
        size_t underscore_pos = name.find('_');
        if (underscore_pos != std::string::npos) {
          std::string substring = name.substr(0, underscore_pos);
          model_types.insert(substring);
        }
	else {
          return std::string("UnknownModelName");
        }
      }
      if (model_types.size() != 1) {
        throw std::runtime_error("SaltModelTriton: model names are not consistent between outputs");
      }
      return *model_types.begin();
    }
  }

  const SaltModelGraphConfig::GraphConfig SaltModelTriton::getGraphConfig() const {
    return SaltModelGraphConfig::parse_json_graph(m_metadata);
  }

  const OutputConfig& SaltModelTriton::getOutputConfig() const {
    return m_output_nodes;
  }

  SaltModelVersion SaltModelTriton::getSaltModelVersion() const {
    return m_onnx_model_version;
  }

  const std::string& SaltModelTriton::getModelName() const {
    return m_model_type;
  }


  InferenceOutput SaltModelTriton::runInference(
    std::map<std::string, Inputs>& gnn_inputs) const {

    // Create tensor for the input data
    std::vector<std::shared_ptr<tc::InferInput> > inputs_;
    inputs_.reserve(gnn_inputs.size());

    for (auto& [inputName, inputInfo]: gnn_inputs) {
      const std::vector<float>& inputData  = inputInfo.first; // ? good name ?
      const std::vector<int64_t>& inputShape = inputInfo.second;
      if(!prepareInput<float>(inputName, inputShape, inputData, inputs_)) {
	throw std::runtime_error("Failed to prepare input for inference"); // ? more informative error message ?
      }
    }

    // construct raw points for inference
    std::vector<tc::InferInput*> rawInputs;
    for(auto& input : inputs_) {
      rawInputs.push_back(input.get());
    }

    // perform the inference
    tc::InferResult* rawResultPtr = nullptr;
    tc::Headers http_headers;
    grpc_compression_algorithm compression_algorithm = grpc_compression_algorithm::GRPC_COMPRESS_NONE;

    auto client = getClient();
    if(client) {
      tc::Error err = client->Infer(&rawResultPtr
				    , *m_options
				    , rawInputs
				    , {}
				    , http_headers
				    , compression_algorithm);
      if(!err.IsOk()) {
	throw std::runtime_error("unable to run model "+ m_model_name + " error: " + err.Message());
      }
    }
    else {
      throw std::runtime_error("Failed to create Triton gRPC client");
    }

    // Get the result of the inference
    InferenceOutput output;
    std::shared_ptr<tc::InferResult> results(rawResultPtr);
    for (size_t node_idx = 0; node_idx < m_output_nodes.size(); ++node_idx) {
      const auto& output_node = m_output_nodes[node_idx];
      switch(output_node.type) {
      case SaltModelOutput::OutputType::VECFLOAT:
	{
	  std::vector<float> outputVecFloat;
	  extractOutput<float>(output_node.name, results, outputVecFloat);
	  output.vecFloat[output_node.name] = std::move(outputVecFloat);
	}
	break;
      case SaltModelOutput::OutputType::FLOAT:
	{
	  std::vector<float> outputFloat;
	  extractOutput<float>(output_node.name, results, outputFloat);
	  if(outputFloat.size()==1) {
	    output.singleFloat[output_node.name] = outputFloat[0];
	  }
	  else {
	    throw std::runtime_error("Vector of floats returned instead of a single float for " + output_node.name);
	  }
	}
	break;
      case SaltModelOutput::OutputType::VECCHAR:
	{
	  std::vector<int8_t> outputVecInt;
	  extractOutput<int8_t>(output_node.name, results, outputVecInt);
	  // convert int8_t vector to char vector
	  std::vector<char> outputVecChar(outputVecInt.begin(), outputVecInt.end());
	  output.vecChar[output_node.name] = std::move(outputVecChar);
	}
	break;
      case SaltModelOutput::OutputType::UNKNOWN:
	[[fallthrough]];
      default:
	throw std::runtime_error("Unknown output type for the node " + output_node.name);
      }
    }

    return output;
  }

  tc::InferenceServerGrpcClient* SaltModelTriton::getClient() const
  {
    thread_local std::unique_ptr<tc::InferenceServerGrpcClient> threadClient;
    if(!threadClient) {
      std::string url = m_url + ":" + std::to_string(m_port);
      tc::Error err = tc::InferenceServerGrpcClient::Create(&threadClient, url, false, m_useSSL);
      if (!err.IsOk()) {
	std::cerr << "SaltModelTriton ERROR: Failed to create Triton gRPC client for model: " << m_model_name
		  << " at URL: " << url << std::endl;
	std::cerr << err.Message() << std::endl;
	return nullptr;
      }
    }
    return threadClient.get();
  }
} // end of FlavorTagInference namespace
