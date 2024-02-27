/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "OnnxRuntimeSessionToolCPU.h"

AthOnnx::OnnxRuntimeSessionToolCPU::OnnxRuntimeSessionToolCPU(
  const std::string& type, const std::string& name, const IInterface* parent )
  : base_class( type, name, parent )
{
  declareInterface<IOnnxRuntimeSessionTool>(this);
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
    m_session = std::make_unique<Ort::Session>(m_onnxRuntimeSvc->env(), m_modelFileName.value().c_str(), sessionOptions);

    return StatusCode::SUCCESS;
}

StatusCode AthOnnx::OnnxRuntimeSessionToolCPU::finalize()
{
    m_session.reset();
    ATH_MSG_DEBUG( "Ort::Session object deleted" );
    return StatusCode::SUCCESS;
}

Ort::Session& AthOnnx::OnnxRuntimeSessionToolCPU::session() const
{
  return *m_session;
}
