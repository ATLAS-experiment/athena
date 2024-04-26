/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

This class is used to store the configuration for a ONNX output node.
*/

#ifndef OUTPUTNODE_H
#define OUTPUTNODE_H

#include <onnxruntime_cxx_api.h>
#include "nlohmann/json.hpp"
#include <string>

namespace FlavorTagDiscriminants {

class OnnxOutput {

  public:
    enum class OutputType {UNKNOWN, FLOAT, VECCHAR, VECFLOAT};
    enum class OutputTarget {UNKNOWN, JET, TRACK};

    /* constructor for OnnxModelVersion::V1 and higher */
    OnnxOutput(const std::string& name,
               ONNXTensorElementDataType type,
               int rank);

    /* constructor for OnnxModelVersion::V0 */
    OnnxOutput(const std::string& name,
               ONNXTensorElementDataType type,
               const std::string& name_in_model);

    const std::string name;
    const std::string name_in_model;
    const OutputType type;
    const OutputTarget target;

  private:
    OutputType getOutputType(ONNXTensorElementDataType type, int rank) const;
    OutputTarget getOutputTarget(int rank) const;
    const std::string getName(const std::string& name, const std::string& model_name) const;

}; // class OnnxOutput

} // namespace FlavorTagDiscriminants

#endif // OUTPUTNODE_H
