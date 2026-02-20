/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGINFERENCE_SALTMODELTRITON_H
#define FLAVORTAGINFERENCE_SALTMODELTRITON_H

/**
 * @class SaltModelTriton
 *
 * @brief This class implements ISaltModel interface and allows for running
 *        inference by leveraging the IaaS (Triton) mechanism
 */

#include "FlavorTagInference/ISaltModel.h"

#include "grpc_client.h"
#include "grpc_service.pb.h"

namespace Ort {
  class Session;
}

namespace tc = triton::client;

namespace FlavorTagInference {

  class SaltModelTriton final : public ISaltModel
  {
  public:
    SaltModelTriton(const std::string& path_to_onnx
		    , const std::string& model_name
		    , float client_timeout
		    , int port
		    , const std::string& url
		    , bool useSSL);

    virtual InferenceOutput runInference(std::map<std::string, Inputs>& gnn_inputs) const override;

    virtual const SaltModelGraphConfig::GraphConfig getGraphConfig() const override;
    virtual const OutputConfig& getOutputConfig() const override;
    virtual SaltModelVersion getSaltModelVersion() const override;
    virtual const std::string& getModelName() const override;

  private:
    const nlohmann::json loadMetadata(const std::string& key, const Ort::Session* session) const;
    const std::string determineModelType(const Ort::Session* session) const;
    tc::InferenceServerGrpcClient* getClient() const;

    nlohmann::json m_metadata;

    size_t m_num_outputs;
    std::string m_model_name;
    std::string m_model_type;
    OutputConfig m_output_nodes;

    SaltModelVersion m_onnx_model_version = SaltModelVersion::UNKNOWN;

    std::unique_ptr<tc::InferOptions> m_options;
    float                             m_clientTimeout{0.f};
    int                               m_port{8001};
    std::string                       m_url{};
    bool                              m_useSSL{false};
  }; // Class SaltModelnTriton
} // end of FlavorTagInference namespace

#endif

