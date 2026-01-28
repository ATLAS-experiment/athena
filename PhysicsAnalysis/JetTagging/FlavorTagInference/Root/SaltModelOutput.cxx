/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

This class is used to store the configuration for a ONNX output node.
*/

#include "FlavorTagInference/SaltModelOutput.h"

namespace FlavorTagInference {

/* constructor for SaltModelVersion::V1 and higher */
SaltModelOutput::SaltModelOutput(const std::string& name,
                       const ONNXTensorElementDataType type,
                       int rank) 
                       : name(name),
                         name_in_model(name),
                         type(getOutputType(type, rank)){}

/* constructor for SaltModelVersion::V0 */
SaltModelOutput::SaltModelOutput(const std::string& name,
                       const ONNXTensorElementDataType type,
                       const std::string& model_name) 
                       : name(getName(name, model_name)),
                         name_in_model(name),
                         type(getOutputType(type, 0)){}

const std::string SaltModelOutput::getName(const std::string& name, const std::string& model_name) {
  // unfortunately, this is block is needed to support some taggers that we schedule that don't have
  // a well defined model name and rely on output remapping.
  if (model_name == "UnknownModelName") {
    return name;
  }
  return model_name + "_" + name;
}

SaltModelOutput::OutputType SaltModelOutput::getOutputType(ONNXTensorElementDataType type, int rank) const {
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

} // namespace FlavorTagInference
