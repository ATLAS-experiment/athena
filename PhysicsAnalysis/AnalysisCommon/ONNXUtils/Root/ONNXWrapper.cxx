/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <ONNXUtils/ONNXWrapper.h>

// Constructor:
// Use the path resolver to find the location of the network .onnx file
// initialise onnx environment
ONNXWrapper::ONNXWrapper(const std::string & model_path):
  m_modelPath(PathResolverFindCalibFile(model_path)),
  m_onnxEnv(std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_WARNING, "")) {
   
    // initialize session options if needed
    m_session_options.SetIntraOpNumThreads(1);
    m_session_options.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_EXTENDED);


    // Initialise the ONNX environment and session using the above options and the model name
    m_onnxSession = std::make_unique<Ort::Session>(*m_onnxEnv,
                                                   m_modelPath.c_str(),
                                                   m_session_options);

    // get the input nodes
    m_nr_inputs = m_onnxSession->GetInputCount();

    // get the output nodes
    m_nr_output = m_onnxSession->GetOutputCount();
     

    // iterate over all input nodes
    for (std::size_t i = 0; i < m_nr_inputs; i++) {
      const char* input_name = m_onnxSession->GetInputNameAllocated(i, m_allocator).release();

      m_input_names.push_back(input_name);

      m_input_dims[input_name] = getShape(m_onnxSession->GetInputTypeInfo(i));
    }

    // iterate over all output nodes
    for(std::size_t i = 0; i < m_nr_output; i++ ) {
      const char* output_name = m_onnxSession->GetOutputNameAllocated(i, m_allocator).release();

      m_output_names.push_back(output_name);

      m_output_dims[output_name] = getShape(m_onnxSession->GetOutputTypeInfo(i));

    }
}

std::map<std::string, std::vector<float>> ONNXWrapper::Run(
  std::map<std::string, std::vector<float>> inputs, int n_batches) {
    for ( const auto &p : inputs ) // check for valid dimensions between batches and inputs
    {
      uint64_t n=1;
      for (uint64_t i:m_input_dims[p.first])
        {
          n*=i;
        }
    if ( (p.second.size() % n) != 0){
      
      throw std::invalid_argument("For input '"+p.first+"' length not compatible with model. Expect a multiple of "+std::to_string(n)+", got "+std::to_string(p.second.size()));
    }
    if (  p.second.size()!=(n_batches*n)){
      throw std::invalid_argument("Number of batches not compatible with length of vector");
    }
    }
    // Create a CPU tensor to be used as input
    Ort::MemoryInfo memory_info("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault);

    // define input tensor
    std::vector<Ort::Value> output_tensor;
    std::vector<Ort::Value> input_tensor;

    // add the inputs to vector
    for ( const auto &p : m_input_dims )   
    {        
      std::vector<int64_t> in_dims = p.second;
      in_dims.at(0) = n_batches;
      input_tensor.push_back(Ort::Value::CreateTensor<float>(memory_info,
                                                            inputs[p.first].data(),
                                                            inputs[p.first].size(),
                                                            in_dims.data(),
                                                            in_dims.size()));
    }

    // init output tensor and fill with zeros
    std::map<std::string, std::vector<float>> outputs;
    for ( const auto &p : m_output_dims ) {
      std::vector<int64_t> out_dims = p.second;
      out_dims.at(0) = n_batches;
      // init output
      int length = 1;
      for(auto i : out_dims){ length*=i; }
      std::vector<float> output(length,0);
      // std::vector<float> output(m_output_dims[i][1], 0.0);
      outputs[p.first] = std::move(output);
      output_tensor.push_back(Ort::Value::CreateTensor<float>(memory_info,
                                                            outputs[p.first].data(),
                                                            outputs[p.first].size(),
                                                            out_dims.data(),
                                                            out_dims.size()));
    }

    Ort::Session& session = *m_onnxSession;

    // run the model
    session.Run(Ort::RunOptions{nullptr},
                m_input_names.data(),
                input_tensor.data(),
                2,
                m_output_names.data(),
                output_tensor.data(),
                2); 

    return outputs;
    }


const std::map<std::string, std::vector<int64_t>> ONNXWrapper::GetModelInputs() {
  std::map<std::string, std::vector<int64_t>> ModelInputINFO_map;

  for(std::size_t i = 0; i < m_nr_inputs; i++ ) {
    ModelInputINFO_map[m_input_names.at(i)] = m_input_dims[m_input_names.at(i)];
  }
  return ModelInputINFO_map;
}

const std::map<std::string, std::vector<int64_t>> ONNXWrapper::GetModelOutputs() {
  std::map<std::string, std::vector<int64_t>> ModelOutputINFO_map;

  for(std::size_t i = 0; i < m_nr_output; i++ ) {
    ModelOutputINFO_map[m_output_names.at(i)] = m_output_dims[m_output_names.at(i)];
  }
  return ModelOutputINFO_map;
}

const std::map<std::string, std::string> ONNXWrapper::GetMETAData() {
  std::map<std::string, std::string> METAData_map;
  auto metadata = m_onnxSession->GetModelMetadata();

  auto keys = metadata.GetCustomMetadataMapKeysAllocated(m_allocator);
  // auto status = OrtApi::ModelMetadataGetCustomMetadataMapKeys(metadata, m_allocator, keys, nkeys)
  for (size_t i = 0; i < keys.size(); i++) {
    METAData_map[keys[i].get()]=this->GetMETADataByKey(keys[i].get());
  }

  return METAData_map;
}

std::string ONNXWrapper::GetMETADataByKey(const char * key){
  auto metadata = m_onnxSession->GetModelMetadata();
  return metadata.LookupCustomMetadataMapAllocated(key, m_allocator).get();
}

const std::vector<const char*>& ONNXWrapper::getInputNames(){
  //put the model access for input here
  return m_input_names;
}

const std::vector<const char*>& ONNXWrapper::getOutputNames(){
  //put the model access for outputs here
  return m_output_names;
}

const std::vector<int64_t>& ONNXWrapper::getInputShape(int input_nr=0){
  //put the model access for input here
  std::vector<const char*> names = getInputNames();
  return m_input_dims[names.at(input_nr)];
}

const std::vector<int64_t>& ONNXWrapper::getOutputShape(int output_nr=0){
  //put the model access for outputs here
  std::vector<const char*> names = getOutputNames();
  return m_output_dims[names.at(output_nr)];
}

int ONNXWrapper::getNumInputs() const { return m_input_names.size(); }
int ONNXWrapper::getNumOutputs() const { return m_output_names.size(); }

const std::vector<int64_t> ONNXWrapper::getShape(Ort::TypeInfo model_info) {
      auto tensor_info = model_info.GetTensorTypeAndShapeInfo();
      std::vector<int64_t> dims = tensor_info.GetShape();
      dims[0]=1;
      return dims;
    }
