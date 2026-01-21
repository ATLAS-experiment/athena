///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// METNetSigHandler.h
// Header file for class METNetSigHandler
// Author: Alberto Plebani <alberto.plebani@cern.ch>, based on previous implementation by M.Leigh<mleigh@cern.ch>
///////////////////////////////////////////////////////////////////

#ifndef METUTILITIES_MET_METNetSigHandler_H
#define METUTILITIES_MET_METNetSigHandler_H

// STL includes
#include <string>


// ONNX Library
#include <onnxruntime_cxx_api.h>

namespace met {

  class METNetSigHandler {

  public:

    // Constructor with parameters
    METNetSigHandler(const std::string& modelName);

    // Destructor
    virtual ~METNetSigHandler();

    // Public methods
    int initialize();
    int getReqSize() const;
    std::vector<float> predict(std::vector<float> inputs) const;

  private:

    // Default constructor
    METNetSigHandler();

    // Class properties
    std::string m_modelName; // Path to the onnx file
    std::string m_modelPath; // Output of the path resolver

    // Features of the network structure
    size_t m_numInputs;
    size_t m_numOutputs;
    std::vector<int64_t> m_inputDims;
    std::vector<int64_t> m_outputDims;
    std::vector<const char *> m_graphInputNames;
    std::vector<const char *> m_graphOutputNames;

    // ONNX session objects
    Ort::Env m_onnxEnv;
    Ort::SessionOptions m_onnxSessionOptions;
    Ort::AllocatorWithDefaultOptions m_onnxAllocator;
    std::unique_ptr<Ort::Session> m_onnxSession;
  };

}

#endif