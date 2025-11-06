///////////////////////// -*- C++ -*- /////////////////////////////
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Author: Bill Balunas <balunas@cern.ch>, based on earlier implementation by M. Leigh
///////////////////////////////////////////////////////////////////

#ifndef METUTILITIES_MET_METNETHANDLER_H
#define METUTILITIES_MET_METNETHANDLER_H

// STL includes
#include <string>

// ONNX Library
#include <onnxruntime_cxx_api.h>

// For ATLAS_THREAD_SAFE
#include "CxxUtils/checker_macros.h"

namespace met {

  class METNetHandler {

  public:

    // Constructor with parameters
    METNetHandler(const std::string& modelName);

    // Destructor
    virtual ~METNetHandler() = default;

    // Public methods
    int initialize();
    unsigned int getReqSize() const;
    void predict(std::vector<float>& outputs, std::vector<float>& inputs) const;

  private:

    // Default constructor
    METNetHandler() = delete;

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
    mutable std::unique_ptr<Ort::Session> m_onnxSession ATLAS_THREAD_SAFE {nullptr};
    mutable std::mutex m_onnxMutex ATLAS_THREAD_SAFE;
  };

}

#endif