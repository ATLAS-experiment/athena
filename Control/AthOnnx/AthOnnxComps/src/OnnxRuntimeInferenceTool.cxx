/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthOnnxComps/OnnxRuntimeInferenceTool.h"
#include "AthOnnxUtils/OnnxUtils.h"

#ifndef XAOD_STANDALONE
// AthAsynchronousAlgorithm for pointer
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// Gaudi include to figure out if something is an algorithm, a service, or a tool
#include "GaudiKernel/IAlgTool.h"
#endif // !XAOD_STANDALONE

AthOnnx::OnnxRuntimeInferenceTool::OnnxRuntimeInferenceTool( const std::string& name)
  : asg::AsgTool ( name )
{
}

StatusCode AthOnnx::OnnxRuntimeInferenceTool::initialize()
{
    // Get the Onnx Runtime service.
    ATH_CHECK(m_onnxRuntimeSvc.retrieve());

    // Create the session.
    ATH_CHECK(m_onnxSessionTool.retrieve());

    ATH_CHECK(getNodeInfo());

    #ifndef XAOD_STANDALONE
    // If session doesn't support asynchronous inference we don't need to find our parent
    if (!m_onnxSessionTool->supportsAsync())
    {
        ATH_MSG_INFO("Session does not support asynchronous inference");
        m_parentAsyncAlg = nullptr;
        return StatusCode::SUCCESS;
    }
    // Figure out if parent is an AthAsynchronousAlgorithm, and set pointer if it is
    const IAlgTool* p = dynamic_cast<const IAlgTool*>(this);
    // Follow chain of parents up until we hit one that can't be converted to an IAlgTool
    const IInterface* myParent = nullptr;
    while (p != nullptr) {
        myParent = p->parent();
        p = dynamic_cast<const IAlgTool*>(myParent);
    }
    // If this ultimate ancestor can be converted to an AthAsynchronousAlgorithm, set the member variable
    m_parentAsyncAlg = dynamic_cast<const AthAsynchronousAlgorithm*>(myParent);
    if (m_parentAsyncAlg != nullptr) {
        ATH_MSG_INFO("Owned by an AthAsynchronousAlgorithm, using asynchronous inference");
    }
    else {
        ATH_MSG_INFO("Not owned by an AthAsynchronousAlgorithm, not using asynchronous inference");
    }
    #endif // !XAOD_STANDALONE

    return StatusCode::SUCCESS;
}

StatusCode AthOnnx::OnnxRuntimeInferenceTool::getNodeInfo()
{
    auto& session = m_onnxSessionTool->session();
    // obtain the model information
    m_numInputs = session.GetInputCount();
    m_numOutputs = session.GetOutputCount();

    AthOnnxUtils::getInputNodeInfo(session, m_inputShapes, m_inputNodeNames);
    AthOnnxUtils::getOutputNodeInfo(session, m_outputShapes, m_outputNodeNames);

    return StatusCode::SUCCESS;
}


void AthOnnx::OnnxRuntimeInferenceTool::setBatchSize(int64_t batchSize)
{
    if (batchSize <= 0) {
        ATH_MSG_ERROR("Batch size should be positive");
        return;
    }

    for (auto& shape : m_inputShapes) {
        if (shape[0] == -1) {
            shape[0] = batchSize;
        }
    }
    
    for (auto& shape : m_outputShapes) {
        if (shape[0] == -1) {
            shape[0] = batchSize;
        }
    }
}

int64_t AthOnnx::OnnxRuntimeInferenceTool::getBatchSize(int64_t inputDataSize, int idx) const
{
    auto tensorSize = AthOnnxUtils::getTensorSize(m_inputShapes[idx]);
    if (tensorSize < 0) {
        return inputDataSize / abs(tensorSize);
    } else {
        return -1;
    }
}

StatusCode AthOnnx::OnnxRuntimeInferenceTool::inference(std::vector<Ort::Value>& inputTensors, std::vector<Ort::Value>& outputTensors) const
{
    assert (inputTensors.size() == m_numInputs);
    assert (outputTensors.size() == m_numOutputs);

    // Run the model.
    // If we're in Athena and the parent is an asynchronous algorithm we do the inference asynchronously
    #ifndef XAOD_STANDALONE
    if (m_parentAsyncAlg == nullptr) {
    #endif // !XAOD_STANDALONE

    AthOnnxUtils::inferenceWithIOBinding(
            m_onnxSessionTool->session(), 
            m_inputNodeNames, inputTensors, 
            m_outputNodeNames, outputTensors);

    #ifndef XAOD_STANDALONE
    }
    else {
        // Asynchronous version
        std::string errorMsg = AthOnnxUtils::asyncInference(
            m_onnxSessionTool->session(), m_inputNodeNames, inputTensors,
            m_outputNodeNames, outputTensors, m_parentAsyncAlg);
        if (!errorMsg.empty()) {
            ATH_MSG_ERROR("ONNX Runtime Error: " << errorMsg);
            return StatusCode::FAILURE;
        }
    }
    #endif // !XAOD_STANDALONE

    return StatusCode::SUCCESS;
}

void AthOnnx::OnnxRuntimeInferenceTool::printModelInfo() const
{
    ATH_MSG_INFO("Number of inputs: " << m_numInputs);
    ATH_MSG_INFO("Number of outputs: " << m_numOutputs);

    ATH_MSG_INFO("Input node names: ");
    for (const auto& name : m_inputNodeNames) {
        ATH_MSG_INFO("\t" << name);
    }

    ATH_MSG_INFO("Output node names: ");
    for (const auto& name : m_outputNodeNames) {
        ATH_MSG_INFO("\t" << name);
    }

    ATH_MSG_INFO("Input shapes: ");
    for (const auto& shape : m_inputShapes) {
        std::string shapeStr = "\t";
        for (const auto& dim : shape) {
            shapeStr += std::to_string(dim) + " ";
        }
        ATH_MSG_INFO(shapeStr);
    }

    ATH_MSG_INFO("Output shapes: ");
    for (const auto& shape : m_outputShapes) {
        std::string shapeStr = "\t";
        for (const auto& dim : shape) {
            shapeStr += std::to_string(dim) + " ";
        }
        ATH_MSG_INFO(shapeStr);
    }
}

StatusCode AthOnnx::OnnxRuntimeInferenceTool::inference(AthInfer::InputDataMap& inputData, AthInfer::OutputDataMap& outputData) const
{
    // Create input tensors.
    std::vector<Ort::Value> inputTensors;
    for (auto& [inputName, inputInfo] : inputData) {
        const std::vector<int64_t>& shape = inputInfo.first;
        if (std::holds_alternative<std::vector<float>>(inputInfo.second)) {
            auto& data = std::get<std::vector<float>>(inputInfo.second);
            inputTensors.push_back(AthOnnxUtils::createTensor(data, shape));
        } else if (std::holds_alternative<std::vector<int64_t>>(inputInfo.second)) {
            auto& data = std::get<std::vector<int64_t>>(inputInfo.second);
            inputTensors.push_back(AthOnnxUtils::createTensor(data, shape));
        } else {
            ATH_MSG_ERROR("Unsupported data type");
            return StatusCode::FAILURE;
        }
    }

    // Create output tensors.
    std::vector<Ort::Value> outputTensors;
    outputTensors.reserve(inputData.size());
    for (auto& outName : m_outputNodeNames) {
        if (outputData.find(outName) == outputData.end()) {
            ATH_MSG_ERROR("Output name " << outName << " not found in output data map");
            return StatusCode::FAILURE;
        }
        auto& outputInfo = outputData.at(outName);
        auto& shape = outputInfo.first;
        auto tensorSize = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<int64_t>());

        if (std::holds_alternative<std::vector<float>>(outputInfo.second)) {
            auto& data = std::get<std::vector<float>>(outputInfo.second);
            data.resize(tensorSize);
            outputTensors.push_back(AthOnnxUtils::createTensor(data, shape));
        } else if (std::holds_alternative<std::vector<int64_t>>(outputInfo.second)) {
            auto& data = std::get<std::vector<int64_t>>(outputInfo.second);
            data.resize(tensorSize);
            outputTensors.push_back(AthOnnxUtils::createTensor(data, shape));
        } else {
            ATH_MSG_ERROR("Unsupported data type");
            return StatusCode::FAILURE;
        }
    }

    ATH_CHECK(inference(inputTensors, outputTensors));

    return StatusCode::SUCCESS;
}
