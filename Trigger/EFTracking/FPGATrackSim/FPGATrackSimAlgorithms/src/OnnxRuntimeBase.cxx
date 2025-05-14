#include "FPGATrackSimAlgorithms/OnnxRuntimeBase.h"

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <Eigen/Dense>


OnnxRuntimeBase::OnnxRuntimeBase(TString fileName)
{
    initialize(std::move(fileName));
}

OnnxRuntimeBase::OnnxRuntimeBase() {}

void OnnxRuntimeBase::initialize(TString fileName) 
{
  m_fileName = std::move(fileName);
  //load the onnx model to memory using the path m_path_to_onnx
  m_env = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_WARNING, "");

  // Set the ONNX runtime session options
  Ort::SessionOptions session_options;
  // Set graph optimization level
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
  // Create the Ort session
  m_session = std::make_unique< Ort::Session >(*m_env, m_fileName.Data(), session_options);
  // Default allocator
  Ort::AllocatorWithDefaultOptions allocator;
  // Get the names of the input nodes of the model
  size_t numInputNodes = m_session->GetInputCount();
  // Iterate over all input nodes and get the name
  for (size_t i = 0; i < numInputNodes; i++) 
  {
    auto name = m_session->GetInputNameAllocated(i, allocator);
    char* input_name = new char[strlen(name.get()) + 1];
    strcpy(input_name, name.get());

    m_inputNodeNames.push_back(input_name);
    // Get the dimensions of the input nodes,
    // here we assume that all input nodes have the same dimensions
    Ort::TypeInfo inputTypeInfo = m_session->GetInputTypeInfo(i);
    auto tensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();

    m_inputNodeDims = tensorInfo.GetShape();
  }
  // Get the names of the output nodes
  size_t numOutputNodes = m_session->GetOutputCount();
  // Iterate over all output nodes and get the name
  for (size_t i = 0; i < numOutputNodes; i++) 
  {
    auto name = m_session->GetOutputNameAllocated(i, allocator);
    char* output_name = new char[strlen(name.get()) + 1];
    strcpy(output_name, name.get());
    m_outputNodeNames.push_back(output_name);
    // Get the dimensions of the output nodes
    // here we assume that all output nodes have the dimensions
    Ort::TypeInfo outputTypeInfo = m_session->GetOutputTypeInfo(i);
    auto tensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
    m_outputNodeDims = tensorInfo.GetShape();
  }
}

// Inference function using ONNX runtime for multiple entries
std::vector<std::vector<float> >OnnxRuntimeBase::runONNXInference(std::vector<std::vector<float> >& inputTensorValues) const 
{
  std::vector<std::vector<float> > output;
  if (inputTensorValues.size() == 0) return output;

  NetworkBatchInput vectorInput(inputTensorValues.size(), inputTensorValues[0].size());
  for (size_t i = 0; i < inputTensorValues.size(); i++) { 
    for (size_t j = 0; j < inputTensorValues[i].size(); j++) {
      vectorInput(i,j) = inputTensorValues[i][j];
    }
  }
  output = runONNXInference(vectorInput);
  return output;
}



// Inference function using ONNX runtime for one single entry
std::vector<float> OnnxRuntimeBase::runONNXInference(std::vector<float>& inputTensorValues) const 
{
  NetworkBatchInput vectorInput(1, inputTensorValues.size());
  for (size_t i = 0; i < inputTensorValues.size(); i++) {
    vectorInput(0, i) = inputTensorValues[i];
  }
  auto vectorOutput = runONNXInference(vectorInput);
  return vectorOutput[0];
}

// Inference function using ONNX runtime
// the function assumes that the model has 1 input node and 1 output node
std::vector<std::vector<float>> OnnxRuntimeBase::runONNXInference(NetworkBatchInput& inputTensorValues) const 
{
  int batchSize = inputTensorValues.rows();
  std::vector<int64_t> inputNodeDims = m_inputNodeDims;
  std::vector<int64_t> outputNodeDims = m_outputNodeDims; //bad. Assumes they all have the same number of nodes.

  // The first dim node should correspond to the batch size
  // If it is -1, it is dynamic and should be set to the input size
  if (inputNodeDims[0] == -1) 
  {
    inputNodeDims[0] = batchSize;
  }
  if (outputNodeDims[0] == -1) 
  {
    outputNodeDims[0] = batchSize;
  }

  if(inputNodeDims[1]*inputNodeDims[2] != inputTensorValues.cols() && inputNodeDims[1] != inputTensorValues.cols())
  {
    throw std::runtime_error("runONNXInference: feature size doesn't match the input size: inputSize required: " + std::to_string(inputNodeDims[1]*inputNodeDims[2]) + " inputSize provided: " + std::to_string(inputTensorValues.cols()));
  }

  if (batchSize != 1 && (inputNodeDims[0] != batchSize || outputNodeDims[0] != batchSize)) 
  {
    throw std::runtime_error("runONNXInference: batch size doesn't match the input or output node size");
  }

  // Create input tensor object from data values
  // note: this assumes the model has only 1 input node
  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorValues.size(),inputNodeDims.data(), inputNodeDims.size());
  // Double-check that inputTensor is a Tensor
  if (!inputTensor.IsTensor()) 
  {
    throw std::runtime_error("runONNXInference: conversion of input to Tensor failed. ");
  }
  // Score model on input tensors, get back output tensors
  Ort::RunOptions run_options;
  std::vector<Ort::Value> outputTensors =
      m_session->Run(run_options, m_inputNodeNames.data(), &inputTensor,
                     m_inputNodeNames.size(), m_outputNodeNames.data(),
                     m_outputNodeNames.size());
  // Double-check that outputTensors contains Tensors and that the count matches
  // that of output nodes
  if (!outputTensors[0].IsTensor() || (outputTensors.size() != m_outputNodeNames.size())) {
    throw std::runtime_error("runONNXInference: calculation of output failed. ");
  }
  // Get pointer to output tensor float values
  // note: this assumes the model has only 1 output value
  float* outputTensor = outputTensors.front().GetTensorMutableData<float>();
  // Get the output values
  std::vector<std::vector<float>> outputTensorValues(batchSize, std::vector<float>(outputNodeDims[1], -1));
  for (int i = 0; i < outputNodeDims[0]; i++) 
  {
    for (int j = 0; j < ((outputNodeDims.size() > 1) ? outputNodeDims[1] : 1); j++) 
    {
      outputTensorValues[i][j] = outputTensor[i * outputNodeDims[1] + j];
    }
  }

  return outputTensorValues;
}

// Inference function using ONNX runtime
// the function assumes that the model has two output layers
// should return a map of <int, Eigen::Matrix>
std::map<int, Eigen::MatrixXf> OnnxRuntimeBase::runONNXInferenceMultilayerOutput(NetworkBatchInput& inputTensorValues) const 
{
  const int batchSize = inputTensorValues.rows();
  std::vector<int64_t> inputNodeDims = m_inputNodeDims;
  std::vector<int64_t> outputNodeDims = m_outputNodeDims;

  // The first dim node should correspond to the batch size
  // If it is -1, it is dynamic and should be set to the input size
  if (inputNodeDims[0] == -1) 
  {
    inputNodeDims[0] = batchSize;
  }
  if (outputNodeDims[0] == -1) 
  {
    outputNodeDims[0] = batchSize;
  }

  if(inputNodeDims[1] != inputTensorValues.cols())
  {
    throw std::runtime_error("runONNXInference: feature size doesn't match the input size: inputSize required: " + std::to_string(inputNodeDims[1]) + " inputSize provided: " + std::to_string(inputTensorValues.cols()));
  }

  if (batchSize != 1 &&(inputNodeDims[0] != batchSize || outputNodeDims[0] != batchSize)) 
  {
    throw std::runtime_error("runONNXInference: batch size doesn't match the input or output node size");
  }
  // Create input tensor object from data values
  // note: this assumes the model has only 1 input node
  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputNodeDims.data(), inputNodeDims.size());
  // Double-check that inputTensor is a Tensor
  if (!inputTensor.IsTensor()) 
  {
    throw std::runtime_error("runONNXInference: conversion of input to Tensor failed. ");
  }
  // Score model on input tensors, get back output tensors
  Ort::RunOptions run_options;
  std::vector<Ort::Value> outputTensors =
      m_session->Run(run_options, m_inputNodeNames.data(), &inputTensor,
                     m_inputNodeNames.size(), m_outputNodeNames.data(),
                     m_outputNodeNames.size());
  // Double-check that outputTensors contains Tensors and that the count matches
  // that of output nodes
  if (!outputTensors[0].IsTensor() || (outputTensors.size() != m_outputNodeNames.size())) {
    throw std::runtime_error("runONNXInference: calculation of output failed. ");
  }
  // Get pointers to output tensor float values
  // note: this assumes the model has multiple output layers
  std::map<int, Eigen::MatrixXf> outputTensorMap;
  size_t numOutputNodes = m_session->GetOutputCount();
  for (size_t i=0; i<numOutputNodes; i++){ // two output layers

    // retrieve pointer to output float tenor
    float* output = outputTensors.at(i).GetTensorMutableData<float>();
    Ort::TypeInfo outputTypeInfo = m_session->GetOutputTypeInfo(i);
    auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
    // Not all outputNodes have the same shape. Get the new shape.
    // First dimension should be batch size
    outputNodeDims = outputTensorInfo.GetShape();

    int nNodes = outputNodeDims.size() > 1 ? outputNodeDims[1] : 1;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> batchMatrix(batchSize, nNodes);
    for (int j = 0; j < batchSize; j++) 
    {
      Eigen::VectorXf vec(nNodes);
      for (int k = 0; k<nNodes; k++) 
      {
        float val = output[j * outputNodeDims[1] + k];
        vec(k) = val;
      }
      batchMatrix.row(j) = vec;
    } // batch
    outputTensorMap[i] = std::move(batchMatrix);
  } // output layers
  return outputTensorMap;
}
