/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "OnnxRuntimeInferenceTool.h"
#include "AthOnnxUtils/OnnxUtils.h"

AthOnnx::OnnxRuntimeInferenceTool::OnnxRuntimeInferenceTool(
  const std::string& type, const std::string& name, const IInterface* parent )
  : base_class( type, name, parent )
{
  declareInterface<IOnnxRuntimeInferenceTool>(this);
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

StatusCode AthOnnx::OnnxRuntimeInferenceTool::finalize()
{
    return StatusCode::SUCCESS;
}

StatusCode AthOnnx::OnnxRuntimeInferenceTool::getNodeInfo()
{
    auto& session = m_onnxSessionTool->session();
    // obtain the model information
    m_numInputs = session.GetInputCount();
    m_numOutputs = session.GetOutputCount();

    AthOnnx::getInputNodeInfo(session, m_inputShapes, m_inputNodeNames);
    AthOnnx::getOutputNodeInfo(session, m_outputShapes, m_outputNodeNames);

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
    auto tensorSize = AthOnnx::getTensorSize(m_inputShapes[idx]);
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
    AthOnnx::inferenceWithIOBinding(
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
