/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

#include <atomic>
#include <map>

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
		    , bool useSSL
		    , const std::string& bearer = ""
		    , const std::map<std::string, int>& padded_inputs = {}
		    , const std::map<std::string, std::string>& output_renames = {});

    virtual InferenceOutput runInference(InputMap& gnn_inputs) const override;

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
    std::string                       m_bearer{};

    // Batched-model mode (salt --batched-standalone export served with dynamic
    // batching): sequence inputs listed here are sent as [1, N, F], zero padded
    // to N objects, with a boolean <input>_mask [1, N] (true = padding).
    // Empty map = standard per-jet interface.
    std::map<std::string, int>         m_paddedInputs;
    // Athena output name -> output name of the Triton model, where they differ
    std::map<std::string, std::string> m_outputRenames;
    mutable std::atomic<size_t>        m_nTruncated{0};
  }; // Class SaltModelnTriton
} // end of FlavorTagInference namespace

#endif

