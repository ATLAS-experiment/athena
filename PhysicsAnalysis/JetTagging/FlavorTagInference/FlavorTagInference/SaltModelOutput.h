/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

This class is used to store the configuration for a ONNX output node.
*/

#ifndef FLAVORTAGDISCRIMINANTS_SALTMODELOUTPUT_H
#define FLAVORTAGDISCRIMINANTS_SALTMODELOUTPUT_H

#include <onnxruntime_cxx_api.h>
#include "nlohmann/json.hpp"
#include <string>

namespace FlavorTagInference {

class SaltModelOutput {

  public:
    enum class OutputType {UNKNOWN, FLOAT, VECCHAR, VECFLOAT};

    /* constructor for SaltModelVersion::V1 and higher */
    SaltModelOutput(const std::string& name,
               ONNXTensorElementDataType type,
               int rank);

    /* constructor for SaltModelVersion::V0 */
    SaltModelOutput(const std::string& name,
               ONNXTensorElementDataType type,
               const std::string& name_in_model);

    const std::string name;
    const std::string name_in_model;
    const OutputType type;

  private:
    OutputType getOutputType(ONNXTensorElementDataType type, int rank) const;
    static const std::string getName(const std::string& name, const std::string& model_name);

}; // class SaltModelOutput

} // namespace FlavorTagInference

#endif // FLAVORTAGDISCRIMINANTS_SALTMODELOUTPUT_H
