/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGINFERENCE_ISALTMODEL_H
#define FLAVORTAGINFERENCE_ISALTMODEL_H

/**
 *  @class ISaltModel
 *
 *  @brief Abstract base class for SaltModel implementations
 *         local (ONNX) and remote (Triton)
 */

#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "FlavorTagInference/SaltModelOutput.h"
#include "nlohmann/json.hpp"
#include <memory>
#include <map>
#include <vector>
#include <string>

namespace FlavorTagInference {

  enum class SaltModelVersion{UNKNOWN, V0, V1, V2};

  NLOHMANN_JSON_SERIALIZE_ENUM( SaltModelVersion , {
    { SaltModelVersion::UNKNOWN, "" },
    { SaltModelVersion::V0, "v0" },
    { SaltModelVersion::V1, "v1" },
    { SaltModelVersion::V2, "v2" },
  })

  // Inputs: the first element is the input data, the second is the shape
  using Inputs = std::pair<std::vector<float>, std::vector<int64_t>>;
  using OutputConfig = std::vector<SaltModelOutput>;

  struct InferenceOutput {
    std::map<std::string, float> singleFloat;
    std::map<std::string, std::vector<char>> vecChar;
    std::map<std::string, std::vector<float>> vecFloat;
  };
  
  class ISaltModel
  {
  public:
    virtual InferenceOutput runInference(std::map<std::string, Inputs>& gnn_inputs) const =0;
    virtual const SaltModelGraphConfig::GraphConfig getGraphConfig() const = 0;
    virtual const OutputConfig& getOutputConfig() const = 0;
    virtual SaltModelVersion getSaltModelVersion() const = 0;
    virtual const std::string& getModelName() const = 0;
  };

  using ISaltModelPtr = std::shared_ptr<const ISaltModel>;
  
} // End namespace

#endif
