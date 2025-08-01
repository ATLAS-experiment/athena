// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#pragma once

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthOnnxInterfaces/IAthInferenceTool.h"

// System include(s).
#include <string>
#include <vector>

namespace AthInfer {

   /// Algorithm demonstrating the usage of the Triton Client API
   ///
   ///
   /// @author Xiangyang Ju <xju@lbl.gov>
   ///
   class ExampleMLInferenceWithTriton: public AthReentrantAlgorithm {

   public:
      /// Inherit the base class's constructor
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      /// @name Function(s) inherited from @c AthAlgorithm
      /// @{

      /// Function initialising the algorithm
      virtual StatusCode initialize() override;
      /// Function executing the algorithm for a single event
      virtual StatusCode execute( const EventContext& ctx ) const override;

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

      /// Tool handle for the Triton client
      ToolHandle< AthInfer::IAthInferenceTool >  m_tritonTool{
         this, "InferenceTool", "AthInfer::TritonTool", "Triton client tool"
      };

      std::vector<std::vector<std::vector<float>>> m_input_tensor_values_notFlat;

   private:
      std::vector<std::vector<std::vector<float>>> read_mnist_pixel_notFlat(const std::string &full_path) const;
      std::vector<float> flattenNestedVectors(const std::vector<std::vector<float>>& nestedVector) const;
   };

} // namespace AthInfer
