// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef ATHEXONNXRUNTIME_EVALUATEMODELWithAsyncInfer_H
#define ATHEXONNXRUNTIME_EVALUATEMODELWithAsyncInfer_H

// Local include(s).
#include "AthOnnxInterfaces/IAthInferenceTool.h"

// Framework include(s).
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

// Onnx Runtime include(s).
#include <onnxruntime_cxx_api.h>

// System include(s).
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
class EvaluateModelWithAsyncInfer : public AthAsynchronousAlgorithm {

 public:
  /// Inherit the base class's constructor
  using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

  /// @name Function(s) inherited from @c AthAlgorithm
  /// @{

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm for a single event
  virtual StatusCode execute(const EventContext& ctx) const override;

  /// @}

 private:
  /// @name Algorithm properties
  /// @{

  /// Name of the input file to load
  Gaudi::Property<std::string> m_pixelFileName{
      this, "InputDataPixel", "dev/MLTest/2020-03-31/t10k-images-idx3-ubyte",
      "Name of the input pixel file to load"};

  /// Following properties needed to be consdered if the .onnx model is
  /// evaluated in batch mode
  Gaudi::Property<int> m_batchSize{this, "BatchSize", 1,
                                   "No. of elements/example in a batch"};

  /// Tool handler for onnx inference session
  ToolHandle<AthInfer::IAthInferenceTool> m_onnxTool{
      this, "ORTInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"};

  std::vector<std::vector<std::vector<float>>> m_input_tensor_values_notFlat;

};  // class EvaluateModel

}  // namespace AthOnnx

#endif  // ATHEXONNXRUNTIME_EVALUATEMODELWithAsyncInfer_H
