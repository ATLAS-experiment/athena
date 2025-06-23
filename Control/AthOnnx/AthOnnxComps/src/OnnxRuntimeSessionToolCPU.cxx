/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthOnnxComps/OnnxRuntimeSessionToolCPU.h"
#include "PathResolver/PathResolver.h"

AthOnnx::OnnxRuntimeSessionToolCPU::OnnxRuntimeSessionToolCPU(const std::string& name )
  : asg::AsgTool( name)
{
}

StatusCode AthOnnx::OnnxRuntimeSessionToolCPU::initialize()
{
    // Get the Onnx Runtime service.
    ATH_CHECK(m_onnxRuntimeSvc.retrieve());
    ATH_MSG_INFO(" OnnxRuntime release: " << OrtGetApiBase()->GetVersionString());
    
    // Create the session options.
    // TODO: Make this configurable.
    // other threading options: https://onnxruntime.ai/docs/performance/tune-performance/threading.html
    // 1) SetIntraOpNumThreads( 1 );
    // 2) SetInterOpNumThreads( 1 );
    // 3) SetGraphOptimizationLevel( GraphOptimizationLevel::ORT_ENABLE_EXTENDED );

    Ort::SessionOptions sessionOptions;
    sessionOptions.SetGraphOptimizationLevel( GraphOptimizationLevel::ORT_ENABLE_ALL );

    // Create the session.
    ATH_MSG_INFO("Asking model from: " << m_modelFileName.value());
    std::string modelFilePath = PathResolver::find_calib_file(m_modelFileName.value());
    ATH_MSG_INFO("Loading model from: " << modelFilePath);
    m_session = std::make_unique<Ort::Session>(m_onnxRuntimeSvc->env(), modelFilePath.c_str(), sessionOptions);

    return StatusCode::SUCCESS;
}

Ort::Session& AthOnnx::OnnxRuntimeSessionToolCPU::session() const
{
  return *m_session;
}
