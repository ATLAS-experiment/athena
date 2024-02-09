// Dear emacs, this is -*- c++ -*-
// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#ifndef ATHEXONNXRUNTIME_EVALUATEMODEL_H
#define ATHEXONNXRUNTIME_EVALUATEMODEL_H

// Local include(s).
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

// Onnx Runtime include(s).
#include <core/session/onnxruntime_cxx_api.h>

// System include(s).
#include <memory>
#include <string>
#include <vector>

namespace AthOnnx {

   /// Algorithm demonstrating the usage of the ONNX Runtime C++ API
   ///
   /// In most cases this should be preferred over the C API...
   ///
   /// @author Debottam Bakshi Gupta <Debottam.Bakshi.Gupta@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class EvaluateModel: public AthReentrantAlgorithm {

   public:
      /// Inherit the base class's constructor
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      /// @name Function(s) inherited from @c AthAlgorithm
      /// @{

      /// Function initialising the algorithm
      virtual StatusCode initialize() override;
      /// Function executing the algorithm for a single event
      virtual StatusCode execute( const EventContext& ctx ) const override;
      /// Function finalising the algorithm
      virtual StatusCode finalize() override;

      /// @}

   private:
      /// @name Algorithm properties
      /// @{

      /// Name of the model file to load
      Gaudi::Property< std::string > m_pixelFileName{ this, "InputDataPixel",
         "dev/MLTest/2020-03-31/t10k-images-idx3-ubyte",
         "Name of the input pixel file to load" };

      /// Following properties needed to be consdered if the .onnx model is evaluated in batch mode
      Gaudi::Property<int> m_batchSize {this, "BatchSize", 1, "No. of elements/example in a batch"};

      /// Tool handler for onnx inference session
      ToolHandle< IOnnxRuntimeInferenceTool >  m_onnxTool{
         this, "ORTInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"
      };
      
      std::vector<std::vector<std::vector<float>>> m_input_tensor_values_notFlat;

   }; // class EvaluateModel

} // namespace AthOnnx

#endif // ATHEXONNXRUNTIME_EVALUATEMODEL_H
