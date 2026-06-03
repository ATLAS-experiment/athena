// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "AthTritonComps/TritonTool.h"

// Project include(s).
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"
#include "AthenaBaseComps/AthMessaging.h"

// External include(s).
#include <grpc_client.h>
#include <grpc_service.pb.h>

// System include(s).
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

/// Shorthand for the Triton client namespace
namespace tc = triton::client;

/// Helper macro to check Triton client return codes
#define TRITON_CHECK(EXP)                           \
  do {                                              \
    const tc::Error err = EXP;                      \
    if (!err.IsOk()) {                              \
      ATH_MSG_ERROR("Failed to execute: " << #EXP); \
      return StatusCode::FAILURE;                   \
    }                                               \
  } while (false)

namespace AthInfer {

/// DType traits for Triton
template <typename T>
struct TritonDType;
template <>
struct TritonDType<float> {
  static constexpr const char* value = "FP32";
};
template <>
struct TritonDType<int64_t> {
  static constexpr const char* value = "INT64";
};

struct TritonTool::Impl : public AthMessaging {

  // Inherit the constructor(s) from AthMessaging
  using AthMessaging::AthMessaging;

  StatusCode getClient(tc::InferenceServerGrpcClient*& client,
                       const std::string& url, int port, bool useSSL) const {

    thread_local std::unique_ptr<tc::InferenceServerGrpcClient> threadClient;
    if (!threadClient) {

      const std::string urlAndPort =
          url + ":" + std::to_string(port);  // always use the gRPC port

      constexpr bool verbose = false;
      TRITON_CHECK(tc::InferenceServerGrpcClient::Create(
          &threadClient, urlAndPort, verbose, useSSL));

      ATH_MSG_INFO("Triton client created for url: " << urlAndPort);
    }
    client = threadClient.get();

    return StatusCode::SUCCESS;
  }

  template <typename T>
  StatusCode prepareInput(
      const std::string& name, const std::vector<int64_t>& shape,
      const std::vector<T>& data,
      std::vector<std::unique_ptr<tc::InferInput>>& inputs) const {

    const char* dtype = TritonDType<T>::value;
    tc::InferInput* rawInputPtr = nullptr;

    // create the InferInput object with the predefined name, shape, and data
    // type.
    TRITON_CHECK(tc::InferInput::Create(&rawInputPtr, name, shape, dtype));
    assert(rawInputPtr != nullptr);

    // Append tensor values for this input from a byte array.
    // Note: The vector is not copied and so it must not be modified or
    // destroyed until this input is no longer needed (that is until the Infer()
    // call(s) that use the input have completed). Multiple calls can be made to
    // this API to keep adding tensor data for this input. The data will be
    // delivered in the order it was added.
    std::unique_ptr<tc::InferInput> input{rawInputPtr};
    TRITON_CHECK(input->AppendRaw(reinterpret_cast<const uint8_t*>(data.data()),
                                  data.size() * sizeof(T)));

    inputs.push_back(std::move(input));
    return StatusCode::SUCCESS;
  }

  template <typename T>
  StatusCode extractOutput(const std::string& name,
                           const tc::InferResult& result,
                           std::vector<T>& outputVec) const {

    const uint8_t* rawData = nullptr;
    size_t size = 0;

    // Get access to the buffer holding raw results of specified output returned
    // by the server. Note: the buffer is owned by InferResult instance. Users
    // can copy out the data if required to extend the lifetime.
    TRITON_CHECK(result.RawData(name, &rawData, &size));

    outputVec.resize(size / sizeof(T));
    std::memcpy(outputVec.data(), rawData, size);
    return StatusCode::SUCCESS;
  }

  const AthAsynchronousAlgorithm* m_parentAsyncAlg = nullptr;
  std::unique_ptr<tc::InferOptions> m_options;

};  // struct TritonTool::Impl

TritonTool::TritonTool(const std::string& type, const std::string& name,
                       const IInterface* parent)
    : base_class(type, name, parent) {}

TritonTool::~TritonTool() = default;

StatusCode TritonTool::initialize() {

  // Set up the implementation object.
  m_impl = std::make_unique<Impl>(name() + "::Impl");
  m_impl->m_options = std::make_unique<tc::InferOptions>(m_modelName.value());
  m_impl->m_options->model_version_ = m_modelVersion;
  m_impl->m_options->client_timeout_ = m_clientTimeout;

  // Figure out if parent is an AthAsynchronousAlgorithm, and set pointer if it
  // is
  const IAlgTool* p = dynamic_cast<const IAlgTool*>(this);
  // Follow chain of parents up until we hit one that can't be converted to an
  // IAlgTool
  const IInterface* myParent = nullptr;
  while (p != nullptr) {
    myParent = p->parent();
    p = dynamic_cast<const IAlgTool*>(myParent);
  }
  // If this ultimate ancestor can be converted to an AthAsynchronousAlgorithm,
  // set the member variable
  m_impl->m_parentAsyncAlg =
      dynamic_cast<const AthAsynchronousAlgorithm*>(myParent);
  if (m_impl->m_parentAsyncAlg != nullptr) {
    ATH_MSG_INFO(
        "Owned by an AthAsynchronousAlgorithm, using asynchronous inference");
  } else {
    ATH_MSG_INFO(
        "Not owned by an AthAsynchronousAlgorithm, not using asynchronous "
        "inference");
  }

  // Make sure already during initialization that a client can be created.
  tc::InferenceServerGrpcClient* dummyClient = nullptr;
  ATH_CHECK(m_impl->getClient(dummyClient, m_url, m_port, m_useSSL));

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode AthInfer::TritonTool::inference(InputDataMap& inputData,
                                           OutputDataMap& outputData) const {

  assert(m_impl);

  // Create the tensor for the input data.
  // Use shared_ptr to manage the memory of the InferInput objects.
  std::vector<std::unique_ptr<tc::InferInput>> inputs;
  inputs.reserve(inputData.size());

  for (auto& [inputName, inputInfo] : inputData) {

    const std::vector<int64_t>& inputShape = inputInfo.first;
    const DataVariant& variant = inputInfo.second;

    ATH_CHECK(std::visit(
        [&](const auto& dataVec) {
          using T = std::decay_t<decltype(dataVec[0])>;
          return m_impl->prepareInput<T>(inputName, inputShape, dataVec,
                                         inputs);
        },
        variant));
  }

  // construct raw points for inference
  std::vector<tc::InferInput*> rawInputs;
  for (auto& input : inputs) {
    rawInputs.push_back(input.get());
  }

  // Get the triton client object.
  tc::InferenceServerGrpcClient* client = nullptr;
  ATH_CHECK(m_impl->getClient(client, m_url, m_port, m_useSSL));
  assert(client != nullptr);

  // perform the inference.
  std::shared_ptr<tc::InferResult> results;
  tc::Headers http_headers;
  grpc_compression_algorithm compression_algorithm =
      grpc_compression_algorithm::GRPC_COMPRESS_NONE;

  if (m_impl->m_parentAsyncAlg == nullptr) {
    tc::InferResult* rawResultPtr = nullptr;
    TRITON_CHECK(client->Infer(&rawResultPtr, *(m_impl->m_options), rawInputs,
                               {}, http_headers, compression_algorithm));
    assert(rawResultPtr != nullptr);
    results.reset(rawResultPtr);
  } else {
    // If m_impl->m_parentAsyncAlg is set, use asynchronous inference
    using Promise_t = boost::fibers::promise<tc::InferResult*>;
    using Future_t = boost::fibers::future<tc::InferResult*>;
    Promise_t promise{};
    Future_t future = promise.get_future();
    auto callback = [&promise](tc::InferResult* resultPtr) {
      assert(resultPtr != nullptr);
      promise.set_value(resultPtr);
    };
    TRITON_CHECK(client->AsyncInfer(callback, *(m_impl->m_options), rawInputs,
                                    {}, http_headers, compression_algorithm));
    results.reset(future.get());
    ATH_CHECK(m_impl->m_parentAsyncAlg->restoreAfterSuspend());
  }

  // Get the result of the inference.
  for (auto& [outputName, outputInfo] : outputData) {

    DataVariant& variant = outputInfo.second;

    ATH_CHECK(std::visit(
        [&](auto& dataVec) {
          using T = std::decay_t<decltype(dataVec[0])>;
          return m_impl->extractOutput<T>(outputName, *results, dataVec);
        },
        variant));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

void TritonTool::print() const {}

}  // namespace AthInfer
