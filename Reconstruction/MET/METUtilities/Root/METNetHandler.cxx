///////////////////////// -*- C++ -*- /////////////////////////////
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Author: Bill Balunas <balunas@cern.ch>, based on earlier implementation by M. Leigh
///////////////////////////////////////////////////////////////////

// METUtilities includes
#include "METUtilities/METNetHandler.h"
#include "PathResolver/PathResolver.h"

namespace met {

  // Dimensions cannot be changed without altering METNet, so it's OK to hard code these for the foreseeable future.
  METNetHandler::METNetHandler(const std::string& modelName) :
    m_modelName(modelName),
    m_numInputs(1),
    m_numOutputs(1),
    m_inputDims({1,77}),
    m_outputDims({1,2}),
    m_graphInputNames({"inputs"}),
    m_graphOutputNames({"outputs"}){}

  int METNetHandler::initialize() {

    // Use the path resolver to find the location of the network .onnx file
    m_modelPath = PathResolverFindCalibFile(m_modelName);
    if (m_modelPath == "") return 1;

    // Use the default ONNX session settings for 1 CPU thread
    m_onnxSessionOptions.SetIntraOpNumThreads(1);
    m_onnxSessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

    // Initialise the ONNX environment and session using the above options and the model name
    m_onnxSession = std::make_unique<Ort::Session>(m_onnxEnv, m_modelPath.c_str(), m_onnxSessionOptions);

    return 0;
  }

  unsigned int METNetHandler::getReqSize() const{
    // Returns the required size of the inputs for the network
    return static_cast<unsigned int>(m_inputDims[1]);
  }

  void METNetHandler::predict(std::vector<float>& outputs, std::vector<float>& inputs) const {
    // This method passes a input vector through the neural network and returns its estimate
    // It requires conversions to onnx type tensors and back

    // Create a CPU tensor to be used as input
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>( memory_info,
                                                               inputs.data(),
                                                               inputs.size(),
                                                               m_inputDims.data(),
                                                               m_inputDims.size() );

    outputs = {0, 0};
    Ort::Value output_tensor = Ort::Value::CreateTensor<float>( memory_info,
                                                                outputs.data(),
                                                                outputs.size(),
                                                                m_outputDims.data(),
                                                                m_outputDims.size() );

    // Pass the input through the network, getting a vector of outputs
    std::lock_guard<std::mutex> lock(m_onnxMutex);
    m_onnxSession->Run( Ort::RunOptions{nullptr},
                        m_graphInputNames.data(),  &input_tensor,  m_numInputs,
                        m_graphOutputNames.data(), &output_tensor, m_numOutputs );
  }

}
