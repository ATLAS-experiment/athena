// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHTRITONCOMPS_TRITONTOOL_H
#define ATHTRITONCOMPS_TRITONTOOL_H

// Project include(s).
#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

// System include(s).
#include <memory>

namespace AthInfer {

class TritonTool : public extends<AthAlgTool, IAthInferenceTool> {

 public:
  /// Constructor
  TritonTool(const std::string& type, const std::string& name,
             const IInterface* parent);
  /// Destructor
  virtual ~TritonTool();

  /// @name Function(s) inherited from @c AthAlgTool
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IAthInferenceTool
  /// @{

  /// Run inference with multiple inputs and multiple outputs
  virtual StatusCode inference(InputDataMap& inputData,
                               OutputDataMap& outputData) const override final;

  /// Print the tool's properties and configuration
  virtual void print() const override;

  /// @}

 private:
  /// @name Tool properties
  /// @{

  StringProperty m_modelName{this, "ModelName", "", "Model name"};
  IntegerProperty m_port{this, "Port", 8001, "Port ID for Triton server"};
  StringProperty m_modelVersion{this, "ModelVersion", "",
                                "Model version, empty for latest"};
  FloatProperty m_clientTimeout{
      this, "ClientTimeout", 0,
      "Client timeout in milliseconds, 0 for no timeout"};
  StringProperty m_url{this, "URL", "", "Triton URL"};
  BooleanProperty m_useSSL{this, "UseSSL", false,
                           "Use SSL for Triton server connection"};

  /// @}

  /// Implementation details for the tool
  struct Impl;
  /// Pointer to the implementation details
  std::unique_ptr<Impl> m_impl;

};  // class TritonTool

}  // namespace AthInfer

#endif  // ATHTRITONCOMPS_TRITONTOOL_H
