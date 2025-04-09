/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthOnnxComps/OnnxRuntimeInferenceTool.h"
#include "AthOnnxUtils/OnnxUtils.h"

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
    AthOnnxUtils::inferenceWithIOBinding(
            m_onnxSessionTool->session(), 
            m_inputNodeNames, inputTensors, 
            m_outputNodeNames, outputTensors);

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
    for (auto& [outputName, outputInfo] : outputData) {
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
