/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingXrtAlgorithm.h"

EFTrackingXrtAlgorithm::EFTrackingXrtAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingXrtAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());

  for (const auto& [kernelName, kernelDefinition] : m_kernelDefinitions) {
    for (const auto& interfaceDefinition : kernelDefinition) {
      const std::string& storeGateKey = interfaceDefinition.at("storeGateKey");

      // Todo: Reference the enum in the python to avoid painful string 
      // comparisons here.
      if (interfaceDefinition.at("interfaceMode") == "INPUT") {
        m_inputDataStreamKeys.push_back({storeGateKey});
        ATH_CHECK(m_inputDataStreamKeys.back().initialize());
      }
      else if (interfaceDefinition.at("interfaceMode") == "OUTPUT") {
        m_outputDataStreamKeys.push_back({storeGateKey});
        ATH_CHECK(m_outputDataStreamKeys.back().initialize());
      }
      else {
        ATH_MSG_ERROR("Failed to map kernel definitions to xrt objects.");     

        return StatusCode::FAILURE;
      }
    }
  }
  
  return StatusCode::SUCCESS;
}

StatusCode EFTrackingXrtAlgorithm::execute(const EventContext& ctx) const
{
  // Keep xrt variables in execute scope for now as there are non-const 
  // methods that are needed.
  xrt::device device = xrt::device(0);
  xrt::uuid uuid = device.load_xclbin(m_xclbinPath);

  std::vector<xrt::run> runs{};
  std::vector<xrt::kernel> kernels{};
  std::vector<xrt::bo> inputBuffers{};
  std::vector<xrt::bo> outputBuffers{};
  
  for (const auto& [kernelName, kernelDefinition] : m_kernelDefinitions) {
    kernels.emplace_back(device, uuid, kernelName, xrt::kernel::cu_access_mode::exclusive);
    runs.emplace_back(kernels.back());

    for (const auto& interfaceDefinition : kernelDefinition) {
      const int argumentIndex = std::stoi(interfaceDefinition.at("argumentIndex"));

      // Todo: Reference an enum in the python to avoid painful string 
      // comparisons here.
      if (interfaceDefinition.at("interfaceMode") == "INPUT") {
        ATH_MSG_INFO("Mapping: " << kernelName << ",\tINPUT,\t" << argumentIndex);

        inputBuffers.emplace_back(device, 
                                  sizeof(unsigned long) * m_bufferSize, 
                                  kernels.back().group_id(argumentIndex));

        runs.back().set_arg(argumentIndex, inputBuffers.back());
      }
      else if (interfaceDefinition.at("interfaceMode") == "OUTPUT") {
        ATH_MSG_INFO("Mapping: " << kernelName << ",\tOUTPUT,\t" << argumentIndex);

        outputBuffers.emplace_back(device, 
                                   sizeof(unsigned long) * m_bufferSize, 
                                   kernels.back().group_id(argumentIndex));

        runs.back().set_arg(argumentIndex, outputBuffers.back());
      }
      else {
        ATH_MSG_ERROR("Failed to map kernel definitions to xrt objects.");     

        return StatusCode::FAILURE;
      }
    }
  }

  ATH_MSG_DEBUG("Writing Inputs");
  std::size_t handleIndex = 0;
  for (
    const SG::ReadHandleKey<std::vector<unsigned long>>& inputDataStreamKey : 
    m_inputDataStreamKeys
  ) {
    SG::ReadHandle<std::vector<unsigned long>> inputDataStream(inputDataStreamKey, ctx);
    inputBuffers[handleIndex].write(inputDataStream->data(), 
                                    sizeof(unsigned long) * m_bufferSize,
                                    0);

    inputBuffers[handleIndex].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    handleIndex++;
  }

  ATH_MSG_DEBUG("Starting Kernels");
  for (xrt::run& run : runs) {
    run.start();
  }

  ATH_MSG_DEBUG("Waiting for Kernels");
  for (xrt::run& run : runs) {
    run.wait();
  }

  ATH_MSG_DEBUG("Reading Outputs");
  handleIndex = 0;
  for (
    const SG::WriteHandleKey<std::vector<unsigned long>>& outputDataStreamKey : 
    m_outputDataStreamKeys
  ) {
    SG::WriteHandle<std::vector<unsigned long>> outputDataStream(outputDataStreamKey, ctx);
    ATH_CHECK(outputDataStream.record(std::make_unique<std::vector<unsigned long>>(m_bufferSize)));

    outputBuffers[handleIndex].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    outputBuffers[handleIndex].read(outputDataStream->data());

    handleIndex++;
  }

  return StatusCode::SUCCESS;
}

