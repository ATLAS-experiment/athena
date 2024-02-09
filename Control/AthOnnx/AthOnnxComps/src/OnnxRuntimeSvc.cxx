// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "OnnxRuntimeSvc.h"
#include <core/session/onnxruntime_c_api.h>

namespace AthOnnx {
  OnnxRuntimeSvc::OnnxRuntimeSvc(const std::string& name, ISvcLocator* svc) :
      asg::AsgService(name, svc)
   {
     declareServiceInterface<AthOnnx::IOnnxRuntimeSvc>();
   }
   StatusCode OnnxRuntimeSvc::initialize() {

      // Create the environment object.
      Ort::ThreadingOptions tp_options;
      tp_options.SetGlobalIntraOpNumThreads(1);
      tp_options.SetGlobalInterOpNumThreads(1);

      m_env = std::make_unique< Ort::Env >(
            tp_options, ORT_LOGGING_LEVEL_WARNING, name().c_str());
      ATH_MSG_DEBUG( "Ort::Env object created" );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode OnnxRuntimeSvc::finalize() {

      // Dekete the environment object.
      m_env.reset();
      ATH_MSG_DEBUG( "Ort::Env object deleted" );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   Ort::Env& OnnxRuntimeSvc::env() const {

      return *m_env;
   }

} // namespace AthOnnx
