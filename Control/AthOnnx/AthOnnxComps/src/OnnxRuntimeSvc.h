// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef ATHONNXRUNTIMESERVICE_ONNXRUNTIMESVC_H
#define ATHONNXRUNTIMESERVICE_ONNXRUNTIMESVC_H

// Local include(s).
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"

// Framework include(s).
#include <AsgServices/AsgService.h>

// ONNX include(s).
#include <onnxruntime_cxx_api.h>

// System include(s).
#include <memory>

namespace AthOnnx {

   /// Service implementing @c AthOnnx::IOnnxRuntimeSvc
   ///
   /// This is a very simple implementation, just managing the lifetime
   /// of some Onnx Runtime C++ objects.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class OnnxRuntimeSvc : public asg::AsgService, virtual public IOnnxRuntimeSvc {

   public:

      /// @name Function(s) inherited from @c Service
      /// @{
      OnnxRuntimeSvc (const std::string& name, ISvcLocator* svc);

      /// Function initialising the service
      virtual StatusCode initialize() override;
      /// Function finalising the service
      virtual StatusCode finalize() override;

      /// @}

      /// @name Function(s) inherited from @c AthOnnx::IOnnxRuntimeSvc
      /// @{

      /// Return the Onnx Runtime environment object
      virtual Ort::Env& env() const override;

      /// @}

   private:
      /// Global runtime environment for Onnx Runtime
      std::unique_ptr< Ort::Env > m_env;

   }; // class OnnxRuntimeSvc

} // namespace AthOnnx

#endif // ATHONNXRUNTIMESERVICE_ONNXRUNTIMESVC_H
