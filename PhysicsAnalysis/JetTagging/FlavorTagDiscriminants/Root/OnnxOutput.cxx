/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

This class is used to store the configuration for a ONNX output node.
*/

#include "FlavorTagDiscriminants/OnnxOutput.h"

namespace FlavorTagDiscriminants {

/* constructor for OnnxModelVersion::V1 and higher */
OnnxOutput::OnnxOutput(const std::string& name,
                       const ONNXTensorElementDataType type,
                       int rank) 
                       : name(name),
                         name_in_model(name),
                         type(getOutputType(type, rank)),
                         target(getOutputTarget(rank)) {}

/* constructor for OnnxModelVersion::V0 */
OnnxOutput::OnnxOutput(const std::string& name,
                       const ONNXTensorElementDataType type,
                       const std::string& model_name) 
                       : name(getName(name, model_name)),
                         name_in_model(name),
                         type(getOutputType(type, 0)),
                         target(getOutputTarget(0)) {}

const std::string OnnxOutput::getName(const std::string& name, const std::string& model_name) const {
  // unfortunately, this is block is needed to support some taggers that we schedule that don't have
  // a well defined model name and rely on output remapping.
  if (model_name == "UnknownModelName") {
    return name;
  }
  return model_name + "_" + name;
}

OnnxOutput::OutputType OnnxOutput::getOutputType(ONNXTensorElementDataType type, int rank) const {
  // Determine the output node type based on the type and shape of the output tensor.
  using ORT = ONNXTensorElementDataType;
  if (type == ORT::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
    if (rank == 0) {
      return OutputType::FLOAT;
    } else if (rank == 1) {
      return OutputType::VECFLOAT;
    }
  } else if (type == ORT::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8) {
    return OutputType::VECCHAR;
  }
  return OutputType::UNKNOWN;
}

OnnxOutput::OutputTarget OnnxOutput::getOutputTarget(int rank) const {
  /* Currently the location of where an output is decorated to is
  determined based on the rank of the output tensor. In the future 
  this should be replaced with a better implementation, where the
  `target` can be obtained from the model metadata. */
  if (rank == 0) {
    return OutputTarget::JET;
  }
  else if (rank == 1) {
    return OutputTarget::TRACK;
  }
  return OutputTarget::UNKNOWN;
}

} // namespace FlavorTagDiscriminants
