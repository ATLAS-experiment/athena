// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#pragma once

#include "AthTritonInterfaces/ITritonTool.h"
#include "grpc_client.h"
#include "grpc_service.pb.h"

#include <string>
#include <vector>
#include <memory>


#include "AthenaBaseComps/AthAlgTool.h"

namespace tc = triton::client;


namespace AthInfer {

#define FAIL_IF_ERR(X, MSG)                                        \
{                                                                \
    tc::Error err = (X);                                          \
    if (!err.IsOk()) {                                             \
      ATH_MSG_ERROR(MSG);                                          \
      return StatusCode::FAILURE;                                  \
    }                                                              \
}

class TritonTool: public extends<AthAlgTool, ITritonTool>
{

  public:
    TritonTool(const std::string& type, const std::string& name, const IInterface* parent);

    StatusCode initialize() override final;

    virtual StatusCode inference(InputDataMap& inputData, OutputDataMap& outputData) const override final;

  protected:
    TritonTool() = delete;
    TritonTool(const TritonTool&) =delete;
    TritonTool &operator=(const TritonTool&) = delete;

    StringProperty m_modelName{this, "ModelName", "", "Model name"};
    IntegerProperty m_port{this, "Port", 8001, "Port ID for Triton server"};
    StringProperty m_modelVersion{this, "ModelVersion", "", "Model version, empty for latest"};
    FloatProperty m_clientTimeout{this, "ClientTimeout", 0, "Client timeout in milliseconds, 0 for no timeout"};
    StringProperty m_url{this, "URL", "", "Triton URL"};
    BooleanProperty m_useSSL{this, "UseSSL", false, "Use SSL for Triton server connection"};

  private:
    tc::InferenceServerGrpcClient* getClient() const;
    std::unique_ptr<tc::InferOptions> m_options;

    template <typename T>
    StatusCode prepareInput(const std::string& name,
                            const std::vector<int64_t>& shape,
                            const std::vector<T>& data,
                            std::vector<std::shared_ptr<tc::InferInput>>& inputs) const;

    template <typename T>
    StatusCode extractOutput(const std::string& name,
                            const std::shared_ptr<tc::InferResult>& result,
                            std::vector<T>& outputVec) const;

};

  #include "TritonTool.icc"

}
