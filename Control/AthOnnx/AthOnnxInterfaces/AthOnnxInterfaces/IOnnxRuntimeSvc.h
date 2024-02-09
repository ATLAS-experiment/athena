// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef ATHEXOnnxRUNTIME_IOnnxRUNTIMESVC_H
#define ATHEXOnnxRUNTIME_IOnnxRUNTIMESVC_H

// Gaudi include(s).
#include <AsgServices/IAsgService.h>

// Onnx include(s).
#include <core/session/onnxruntime_cxx_api.h>


/// Namespace holding all of the Onnx Runtime example code
namespace AthOnnx {

   //class IAsgService
   /// Service used for managing global objects used by Onnx Runtime
   ///
   /// In order to allow multiple clients to use Onnx Runtime at the same
   /// time, this service is used to manage the objects that must only
   /// be created once in the Athena process.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class IOnnxRuntimeSvc : virtual public asg::IAsgService{

   public:
      /// Virtual destructor, to make vtable happy
      virtual ~IOnnxRuntimeSvc() = default;

      /// Declare the interface that this class provides
      DeclareInterfaceID (AthOnnx::IOnnxRuntimeSvc, 1, 0);

      /// Return the Onnx Runtime environment object
      virtual Ort::Env& env() const = 0;

   }; // class IOnnxRuntimeSvc

} // namespace AthOnnx

#endif
