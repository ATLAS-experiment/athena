///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// METNetSigHandler.cxx
// Implementation file for class METNetSigHandler
// Author: Alberto Plebani <alberto.plebani@cern.ch>, based on earlier implementation by M. Leigh
///////////////////////////////////////////////////////////////////

// METUtilities includes
#include "METUtilities/METNetSigHandler.h"
#include "PathResolver/PathResolver.h"
#include <iostream>

namespace met {

  // Dimensions cannot be changed without altering METNet, so it's OK to hard code these for the foreseeable future.
  METNetSigHandler::METNetSigHandler(const std::string& modelName) :
    m_modelName(modelName),
    m_numInputs(1),
    m_numOutputs(4),
    m_inputDims({1,77}),
    m_outputDims({1,1}), // m_outputDims({1,2}),
    m_graphInputNames({"input"}),
    m_graphOutputNames({"MET_x", "MET_y", "Sigma_x", "Sigma_y"}){}

  METNetSigHandler::~METNetSigHandler() { }

  int METNetSigHandler::initialize() {

    // Use the path resolver to find the location of the network .onnx file
    // m_modelPath = PathResolverFindCalibFile(m_modelName);
    // m_modelPath = "ggHyydAnalysis/model_mc23.onnx";
    m_modelPath = PathResolverFindCalibFile(m_modelName);
    // m_modelPath = "../source/ggHyydAnalysis/share/model_mc23.onnx";
    if (m_modelPath == "") return 1;

    // Use the default ONNX session settings for 1 CPU thread
    m_onnxSessionOptions.SetIntraOpNumThreads(1);
    m_onnxSessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

    // Initialise the ONNX environment and session using the above options and the model name
    m_onnxSession = std::make_unique<Ort::Session>(m_onnxEnv, m_modelPath.c_str(), m_onnxSessionOptions);

    return 0;
  }

  int METNetSigHandler::getReqSize() const {
    // Returns the required size of the inputs for the network
    return static_cast<int>(m_inputDims[1]);
  }

  std::vector<float> METNetSigHandler::predict(std::vector<float> inputs) const {
    // This method passes a input vector through the neural network and returns its estimate
    // It requires conversions to onnx type tensors and back

    // Create a CPU tensor to be used as input
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>( memory_info,
                                                               inputs.data(),
                                                               inputs.size(),
                                                               m_inputDims.data(),
                                                               m_inputDims.size() );
                                                               

    std::vector<Ort::Value> ort_outputs = m_onnxSession->Run(
      Ort::RunOptions{nullptr},
      m_graphInputNames.data(), &input_tensor, m_numInputs,
      m_graphOutputNames.data(), m_numOutputs
    );

    // Extract output values and convert from GeV to MeV
    std::vector<float> outputs;
    for (size_t i = 0; i < ort_outputs.size(); ++i) {
      const float* data = ort_outputs[i].GetTensorData<float>();
      outputs.push_back(data[0] * 1000.0f);
    }
    return outputs;

  }

}