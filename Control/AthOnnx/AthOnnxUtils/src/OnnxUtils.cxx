// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "AthOnnxUtils/OnnxUtils.h"
#include <cassert>
#include <string>

namespace AthOnnx {

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

int64_t getTensorSize(const std::vector<int64_t>& dataShape){
    int64_t size = 1;
    for (const auto& dim : dataShape) {
            size *= dim;
    }
    return size;
}

Ort::Value createTensor(std::vector<float>& data, const std::vector<int64_t>& dataShape)
{
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault); 

    return Ort::Value::CreateTensor<float>(
                                memoryInfo, 
                                data.data(), 
                                data.size(),  
                                dataShape.data(), 
                                dataShape.size());
};


} // namespace AthOnnx