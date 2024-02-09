/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "OnnxRuntimeSessionToolCUDA.h"

AthOnnx::OnnxRuntimeSessionToolCUDA::OnnxRuntimeSessionToolCUDA(
    const std::string& type, const std::string& name, const IInterface* parent )
    : base_class( type, name, parent )
{
  declareInterface<IOnnxRuntimeSessionTool>(this);
}

StatusCode AthOnnx::OnnxRuntimeSessionToolCUDA::initialize()
{
    // Get the Onnx Runtime service.
    ATH_CHECK(m_onnxRuntimeSvc.retrieve());

    ATH_MSG_INFO(" OnnxRuntime release: " << OrtGetApiBase()->GetVersionString());
    // Create the session options.
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetGraphOptimizationLevel( GraphOptimizationLevel::ORT_ENABLE_EXTENDED );
    sessionOptions.DisablePerSessionThreads();    // use global thread pool.

    // TODO: add more cuda options to the interface
    // https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html#cc
    // Options: https://onnxruntime.ai/docs/api/c/struct_ort_c_u_d_a_provider_options.html
    OrtCUDAProviderOptions cuda_options;
    cuda_options.device_id = m_deviceId;
    cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearch::OrtCudnnConvAlgoSearchExhaustive;
    cuda_options.gpu_mem_limit = std::numeric_limits<size_t>::max();

    // memorry arena options for cuda memory shrinkage
    // https://github.com/microsoft/onnxruntime/blob/main/onnxruntime/test/shared_lib/utils.cc#L7
    if (m_enableMemoryShrinkage) {
        Ort::ArenaCfg arena_cfg{0, 0, 1024, 0};
        // other options are not available in this release.
        // https://github.com/microsoft/onnxruntime/blob/main/onnxruntime/test/shared_lib/test_inference.cc#L2802C21-L2802C21
        // arena_cfg.max_mem = 0;   // let ORT pick default max memory
        // arena_cfg.arena_extend_strategy = 0;   // 0: kNextPowerOfTwo, 1: kSameAsRequested
        // arena_cfg.initial_chunk_size_bytes = 1024;
        // arena_cfg.max_dead_bytes_per_chunk = 0;
        // arena_cfg.initial_growth_chunk_size_bytes = 256;
        // arena_cfg.max_power_of_two_extend_bytes = 1L << 24;

        cuda_options.default_memory_arena_cfg = arena_cfg;
    }

    sessionOptions.AppendExecutionProvider_CUDA(cuda_options);

    // Create the session.
    m_session = std::make_unique<Ort::Session>(m_onnxRuntimeSvc->env(),  m_modelFileName.value().c_str(), sessionOptions);

    return StatusCode::SUCCESS;
}

StatusCode AthOnnx::OnnxRuntimeSessionToolCUDA::finalize()
{
    m_session.reset();
    ATH_MSG_DEBUG( "Ort::Session object deleted" );
    return StatusCode::SUCCESS;
}

Ort::Session& AthOnnx::OnnxRuntimeSessionToolCUDA::session() const
{
  return *m_session;
}
