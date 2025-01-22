/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include <set>

#include "EFTrackingXrtAlgorithm.h"

EFTrackingXrtAlgorithm::EFTrackingXrtAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingXrtAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());

  // Retrieve the necessary component(s).
  ATH_CHECK(m_DeviceMgmtSvc.retrieve());

  // Todo: Fix this disgusting mess (probably just add a new method to the 
  //       AthXrt service to enumerate devices and a method to check if a 
  //       device has a particular kernel.
  m_device = getDevice();

  if (!m_device) {
    ATH_MSG_ERROR("No XRT device provides all kernels.");

    return StatusCode::FAILURE;
  }

  for (const auto& [kernelName, kernelDefinition] : m_kernelDefinitions) {
    m_kernels.push_back(std::make_unique<xrt::kernel>(*m_device, 
                                                      m_device->get_xclbin_uuid(), 
                                                      kernelName,
                                                      xrt::kernel::cu_access_mode::exclusive));

    m_runs.push_back(std::make_unique<xrt::run>(*m_kernels.back()));

    for (const auto& interfaceDefinition : kernelDefinition) {
      const std::string& storeGateKey = interfaceDefinition.at("storeGateKey");
      const int argumentIndex = std::stoi(interfaceDefinition.at("argumentIndex"));

      // Todo: Reference the enum in the python to avoid painful string 
      // comparisons here.
      if (interfaceDefinition.at("interfaceMode") == "INPUT") {
        m_inputDataStreamKeys.push_back({storeGateKey});
        ATH_CHECK(m_inputDataStreamKeys.back().initialize());

        m_inputBuffers.emplace_back(
          *m_device, 
          sizeof(unsigned long) * m_bufferSize, 
          xrt::bo::flags::normal, 
          m_kernels.back()->group_id(argumentIndex)
        );

        m_runs.back()->set_arg(argumentIndex, m_inputBuffers.back());
      }
      else if (interfaceDefinition.at("interfaceMode") == "OUTPUT") {
        m_outputDataStreamKeys.push_back({storeGateKey});
        ATH_CHECK(m_outputDataStreamKeys.back().initialize());

        m_outputBuffers.emplace_back(
          *m_device, 
          sizeof(unsigned long) * m_bufferSize, 
          xrt::bo::flags::normal, 
          m_kernels.back()->group_id(argumentIndex)
        );

        m_runs.back()->set_arg(argumentIndex, m_outputBuffers.back());
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
  ATH_MSG_DEBUG("Writing Inputs");
  std::size_t handleIndex = 0;
  for (
    const SG::ReadHandleKey<std::vector<unsigned long>>& inputDataStreamKey : 
    m_inputDataStreamKeys
  ) {
    SG::ReadHandle<std::vector<unsigned long>> inputDataStream(inputDataStreamKey, ctx);
    //m_inputBuffers.at(handleIndex)->write(inputDataStream->data(), 
    //                                       sizeof(unsigned long) * m_bufferSize,
    //                                       0);

    unsigned long* inputMap = m_inputBuffers.at(handleIndex).map<unsigned long*>();

    if (inputDataStream->size() > m_bufferSize) {
      ATH_MSG_ERROR("Input data stream too large for buffer (buffer: " << 
                    m_bufferSize << 
                    ", data stream: " << 
                    inputDataStream->size() << 
                    "), consider increasing buffer size.");

      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Test: " << inputDataStream->size());

    for (std::size_t index = 0; index < inputDataStream->size(); index++) {
      inputMap[index] = inputDataStream->at(index);
      ATH_MSG_INFO("Test: " << std::hex << inputDataStream->at(index));
    }
    
    m_inputBuffers.at(handleIndex).sync(XCL_BO_SYNC_BO_TO_DEVICE);
    handleIndex++;
  }

  ATH_MSG_DEBUG("Starting Kernels");
  for (std::size_t index = 0; index < m_runs.size(); index++) {
    m_runs.at(index)->start();
  }

  ATH_MSG_DEBUG("Waiting for Kernels");
  for (std::size_t index = 0; index < m_runs.size(); index++) {
    m_runs.at(index)->wait();
  }

  ATH_MSG_DEBUG("Reading Outputs");
  handleIndex = 0;
  for (
    const SG::WriteHandleKey<std::vector<unsigned long>>& outputDataStreamKey : 
    m_outputDataStreamKeys
  ) {
    SG::WriteHandle<std::vector<unsigned long>> outputDataStream(outputDataStreamKey, ctx);
    ATH_CHECK(outputDataStream.record(std::make_unique<std::vector<unsigned long>>(m_bufferSize)));

    m_outputBuffers.at(handleIndex).sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    //m_outputBuffers.at(handleIndex)->read(outputDataStream->data());

    const unsigned long* outputMap = m_outputBuffers.at(handleIndex).map<unsigned long*>();

    for (std::size_t index = 0; index < outputDataStream->size(); index++) {
      outputDataStream->at(index) = outputMap[index];
    }
    

    handleIndex++;
  }

  return StatusCode::SUCCESS;
}

std::shared_ptr<xrt::device> EFTrackingXrtAlgorithm::getDevice() {
  std::set<std::shared_ptr<xrt::device>> devicesSet{};
  std::vector<std::shared_ptr<xrt::device>> finalDevices{};

  for (const auto& [kernelName, _] : m_kernelDefinitions) {
    std::vector<std::shared_ptr<xrt::device>> devices = 
      m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    devicesSet.insert(devices.begin(), devices.end());
  }

  for (const std::shared_ptr<xrt::device>& device : devicesSet) {
    if (!deviceHasKernels(device)) {
      continue;
    }

    return device;
  }

  return {};
}

bool EFTrackingXrtAlgorithm::deviceHasKernels(
  const std::shared_ptr<xrt::device>& device
) const {
  for (const auto& [kernelName, _] : m_kernelDefinitions) {
    if (!deviceHasKernel(device, m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName))) {
      return false;
    }
  }
  
  return true;
}

bool EFTrackingXrtAlgorithm::deviceHasKernel(
  const std::shared_ptr<xrt::device>& device,
  const std::vector<std::shared_ptr<xrt::device>>& devices 
) const {
  for (const std::shared_ptr<xrt::device>& otherDevice : devices) {
    if (*device == *otherDevice) {
      return true;
    }
  }

  return false;
}

