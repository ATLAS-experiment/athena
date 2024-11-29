/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ONNXUtils_h
#define ONNXUtils_h

// STL includes
#include <string>
#include <vector>
#include <map>

// Asg tool includes

#include "PathResolver/PathResolver.h"

// ONNX Library
#include <onnxruntime_cxx_api.h>


class ONNXWrapper {

  private:

    // Class properties
    std::string m_modelName; // Path to the onnx file
    std::string m_modelPath; // Output of the path resolver

    // Features of the network structure

    // input and output nodes
    size_t m_nr_inputs;
    size_t m_nr_output;

    // dimensions of the input and output
    std::map<std::string, std::vector<int64_t>> m_input_dims;
    std::map<std::string, std::vector<int64_t>> m_output_dims;


    // ONNX session objects
    std::unique_ptr<Ort::Session> m_onnxSession;
    std::unique_ptr< Ort::Env > m_onnxEnv;

    // onnx session options
    Ort::SessionOptions m_session_options;
    Ort::AllocatorWithDefaultOptions m_allocator;

    // allocate memory

    // name of the outputs
    std::vector<const char*> m_output_names;
    std::vector<const char*> m_input_names;
    const std::vector<int64_t> getShape(Ort::TypeInfo model_info);

  public:
    // Constructor with parameters

    ONNXWrapper(const std::string & model_path);

    std::map<std::string, std::vector<float>> Run(
      std::map<std::string,
      std::vector<float>> inputs,
      int n_batches=1);

    const std::map<std::string, std::vector<int64_t>> GetModelInputs();
    const std::map<std::string, std::vector<int64_t>> GetModelOutputs();

    const std::map<std::string, std::string> GetMETAData();
    std::string GetMETADataByKey(const char * key);
    const std::vector<int64_t>& getInputShape(int input_nr);
    const std::vector<int64_t>& getOutputShape(int output_nr);
    const std::vector<const char*>& getInputNames();
    const std::vector<const char*>& getOutputNames();
    int getNumInputs() const;
    int getNumOutputs() const;
};

#endif
