// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include "AthOnnxUtils/OnnxUtils.h"
#include <cassert>
#include <string>

#ifndef XAOD_STANDALONE
// AthAsynchronousAlgorithm to resume in asyncInference
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// Explicit include of boost fiber
#include <boost/fiber/all.hpp>
#endif // !XAOD_STANDALONE

namespace AthOnnxUtils {

void getNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape, 
    std::vector<std::string>& nodeNames,
    bool isInput
){
    dataShape.clear();
    nodeNames.clear();

    size_t numNodes = isInput? session.GetInputCount(): session.GetOutputCount();
    dataShape.reserve(numNodes);
    nodeNames.reserve(numNodes);

    Ort::AllocatorWithDefaultOptions allocator;
    for( std::size_t i = 0; i < numNodes; i++ ) {
        Ort::TypeInfo typeInfo = isInput? session.GetInputTypeInfo(i): session.GetOutputTypeInfo(i);
        auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
        dataShape.emplace_back(tensorInfo.GetShape());

        auto nodeName = isInput? session.GetInputNameAllocated(i, allocator) : session.GetOutputNameAllocated(i, allocator);
        nodeNames.emplace_back(nodeName.get());
     }
}

void getInputNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape, 
    std::vector<std::string>& nodeNames
){
    getNodeInfo(session, dataShape, nodeNames, true);
}

void getOutputNodeInfo(
    const Ort::Session& session,
    std::vector<std::vector<int64_t> >& dataShape,
    std::vector<std::string>& nodeNames
) {
    getNodeInfo(session, dataShape, nodeNames, false);
}

void inferenceWithIOBinding(Ort::Session& session, 
    const std::vector<std::string>& inputNames,
    const std::vector<Ort::Value>& inputData,
    const std::vector<std::string>& outputNames,
    const std::vector<Ort::Value>& outputData){
    
    if (inputNames.empty()) {
        throw std::runtime_error("Onnxruntime input data maping cannot be empty");
    }
    assert(inputNames.size() == inputData.size());

    Ort::IoBinding iobinding(session);
    for(size_t idx = 0; idx < inputNames.size(); ++idx){
        iobinding.BindInput(inputNames[idx].data(), inputData[idx]);
    }


    for(size_t idx = 0; idx < outputNames.size(); ++idx){
        iobinding.BindOutput(outputNames[idx].data(), outputData[idx]);
    }

    session.Run(Ort::RunOptions{nullptr}, iobinding);
}

#ifndef XAOD_STANDALONE
std::string asyncInference(Ort::Session& session,
                           const std::vector<std::string>& inputNames,
                           const std::vector<Ort::Value>& inputData,
                           const std::vector<std::string>& outputNames,
                           std::vector<Ort::Value>& outputData,
                           const AthAsynchronousAlgorithm* parentAlg) {
  if (inputNames.empty()) {
    throw std::runtime_error("Onnxruntime input data mapping cannot be empty");
  }
  assert(inputNames.size() == inputData.size());
  assert(outputNames.size() == outputData.size());

  Ort::RunOptions runOptions{};

  // Transform names into formats required by ORT
  std::vector<const char*> inputNamesArray{};
  std::vector<const char*> outputNamesArray{};
  inputNamesArray.reserve(inputNames.size());
  outputNamesArray.reserve(outputNames.size());
  for (const auto& name : inputNames) {
    inputNamesArray.push_back(name.c_str());
  }
  for (const auto& name : outputNames) {
    outputNamesArray.push_back(name.c_str());
  }

  // Setup for async
  using Promise_t = boost::fibers::promise<std::string>;
  Promise_t promise{};
  boost::fibers::future<std::string> future{promise.get_future()};

  // callback in format required by ORT
  const auto callback = [](void* promise, OrtValue**, std::size_t,
                           OrtStatusPtr statusPtr) mutable {
    std::string errorMsg{};
    if (statusPtr != nullptr) {
      Ort::Status status{statusPtr};
      if (!status.IsOK()) {
        errorMsg = status.GetErrorMessage();
      }
    }
    static_cast<Promise_t*>(promise)->set_value(errorMsg);
  };

  // Run inference
  session.RunAsync(runOptions, inputNamesArray.data(), inputData.data(),
                   inputData.size(), outputNamesArray.data(), outputData.data(),
                   outputData.size(), callback, static_cast<void*>(&promise));
  // Suspends fiber while waiting
  std::string errorMsg = future.get();
  parentAlg->restoreAfterSuspend().orThrow("Failed to restore after suspension", "AsyncAlg");
  return errorMsg;
}
#endif

int64_t getTensorSize(const std::vector<int64_t>& dataShape){
    int64_t size = 1;
    for (const auto& dim : dataShape) {
            size *= dim;
    }
    return size;
}


} // namespace AthOnnx
