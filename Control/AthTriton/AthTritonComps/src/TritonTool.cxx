// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "AthTritonComps/TritonTool.h"

namespace tc = triton::client;

AthInfer::TritonTool::TritonTool( const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent)
    : base_class(type, name, parent)
{
    declareInterface<ITritonTool>(this);
}

StatusCode AthInfer::TritonTool::initialize() {

    m_options = std::make_unique<tc::InferOptions>(m_modelName.value());
    m_options->model_version_ = m_modelVersion;
    m_options->client_timeout_ = m_clientTimeout;

    return getClient()? StatusCode::SUCCESS : StatusCode::FAILURE;
}

tc::InferenceServerGrpcClient* AthInfer::TritonTool::getClient() const {
    thread_local std::unique_ptr<tc::InferenceServerGrpcClient> threadClient;
    if (!threadClient) {
        std::string url = m_url.value() + ":" + std::to_string(m_port); // always use the gRPC port

        bool verbose = false;

        tc::Error err = tc::InferenceServerGrpcClient::Create(&threadClient, url, verbose, m_useSSL);
        if (!err.IsOk()) {
            ATH_MSG_ERROR("Failed to create Triton gRPC client for model: " + m_modelName.value() + " at url: " + url);
            ATH_MSG_ERROR("Error message: " + err.Message());
            return nullptr;
        }

        ATH_MSG_INFO("Triton client created for model: "+ m_modelName.value() + " at url: "+ url);

    }
    return threadClient.get();
}

StatusCode AthInfer::TritonTool::inference(InputDataMap& inputData, OutputDataMap& outputData) const {

    // Create the tensor for the input data.
    // Use shared_ptr to manage the memory of the InferInput objects.
    std::vector<std::shared_ptr<tc::InferInput> > inputs_;
    inputs_.reserve(inputData.size());

    for (auto& [inputName, inputInfo]: inputData) {
        const std::vector<int64_t>& inputShape = inputInfo.first;
        const auto& variant = inputInfo.second;

        const auto status = std::visit([&](const auto& dataVec) {
            using T = std::decay_t<decltype(dataVec[0])>;
            return prepareInput<T>(inputName, inputShape, dataVec, inputs_);
        }, variant);

        if (status != StatusCode::SUCCESS) return status;
    }

   // construct raw points for inference
    std::vector<tc::InferInput*> rawInputs;
    for (auto& input: inputs_) {
        rawInputs.push_back(input.get());
    }

    // perform the inference.
    tc::InferResult* rawResultPtr = nullptr;
    tc::Headers http_headers;
    grpc_compression_algorithm compression_algorithm =
        grpc_compression_algorithm::GRPC_COMPRESS_NONE;

    FAIL_IF_ERR(
     getClient()->Infer(
        &rawResultPtr, *m_options, rawInputs, {}, http_headers, compression_algorithm),
        "unable to run model "+ m_modelName.value() + " error: " + err.Message()
    );

    std::shared_ptr<tc::InferResult> results(rawResultPtr);

    // Get the result of the inference.
    for (auto& [outputName, outputInfo]: outputData) {
        auto& variant = outputInfo.second;

        const auto status = std::visit([&](auto& dataVec) {
            using T = std::decay_t<decltype(dataVec[0])>;
            return extractOutput<T>(outputName, results, dataVec);
        }, variant);

        if (status != StatusCode::SUCCESS) return status;
    }
    return StatusCode::SUCCESS;
}
