/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include <set>
#include <regex>

#include "AthenaKernel/Chrono.h"

#include "EFTrackingFPGAPipeline/EFTrackingXrtAlgorithm.h"
#include "EFTrackingFPGAPipeline/EFTrackingXrtParameters.h"

namespace {
bool deviceHasKernel(
  const std::shared_ptr<xrt::device>& device,
  const std::vector<std::shared_ptr<xrt::device>>& devices 
) {
  for (const std::shared_ptr<xrt::device>& otherDevice : devices) {
    if (*device == *otherDevice) {
      return true;
    }
  }

  return false;
}

bool deviceHasKernels(
  const nlohmann::json& kernelDefinitionsJson,
  const ServiceHandle<AthXRT::IDeviceMgmtSvc>& deviceMgmtSvc,
  const std::shared_ptr<xrt::device>& device
) {
  for (const auto& [kernelName, dummy] : kernelDefinitionsJson.items()) {
    if (!deviceHasKernel(device, deviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName))) {
      return false;
    }
  }
  
  return true;
}

std::shared_ptr<xrt::device> getDevice(
  const nlohmann::json& kernelDefinitionsJson,
  const ServiceHandle<AthXRT::IDeviceMgmtSvc>& deviceMgmtSvc
) {
  std::set<std::shared_ptr<xrt::device>> devicesSet{};

  for (const auto& [kernelName, dummy] : kernelDefinitionsJson.items()) {
    std::vector<std::shared_ptr<xrt::device>> devices = 
      deviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    devicesSet.insert(devices.begin(), devices.end());
  }

  for (const std::shared_ptr<xrt::device>& device : devicesSet) {
    if (!deviceHasKernels(kernelDefinitionsJson, deviceMgmtSvc, device)) {
      continue;
    }

    return device;
  }

  return {};
}
}

EFTrackingXrtAlgorithm::EFTrackingXrtAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingXrtAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());

  // Too complicated to implement as a Gaudi::Property (would require a new 
  // grammar) so get a string and make the nlohmann::json in initialize. 
  //
  // This should probably just be a std container e.g. vector of map etc.
  const std::optional<nlohmann::json> kernelDefinitionsJson {
    [](const std::string& kernelDefinitionsJsonString)->std::optional<nlohmann::json> {
      try {
        const nlohmann::json kernelDefinitionsJson = 
          nlohmann::json::parse(kernelDefinitionsJsonString);
        return kernelDefinitionsJson;

      }
      catch (const nlohmann::json::exception& [[maybe_unused]]exception) {
        return std::nullopt;
      }

      // c++23 std::unreachable()
      return std::nullopt;
    }(m_kernelDefinitionsJsonString)
  };

  if (!kernelDefinitionsJson) {
    return StatusCode::FAILURE; 
  }

  ATH_CHECK(m_DeviceMgmtSvc.retrieve());
  ATH_CHECK(m_chronoSvc.retrieve());

  // Todo: Fix this disgusting mess (probably just add a new method to the 
  //       AthXrt service to enumerate devices and a method to check if a 
  //       device has a particular kernel.
  m_device = getDevice(*kernelDefinitionsJson, m_DeviceMgmtSvc);
  if (!m_device) {
    ATH_MSG_ERROR("No XRT device provides all kernels.");

    return StatusCode::FAILURE;
  }

  for (const auto& [kernelName, kernelDefinition] : kernelDefinitionsJson->items()) {
    m_kernels.push_back(std::make_unique<xrt::kernel>(*m_device, 
                                                      m_device->get_xclbin_uuid(), 
                                                      kernelName,
                                                      xrt::kernel::cu_access_mode::exclusive));

    m_runs.push_back(std::make_unique<xrt::run>(*m_kernels.back()));

    for (const auto& interfaceDefinition : kernelDefinition) {
      const std::string& storeGateKey = 
        interfaceDefinition.at("storeGateKey").get<const std::string>();

      const int argumentIndex = 
        std::stoi(interfaceDefinition.at("argumentIndex").get<const std::string>());

      const int interfaceMode = 
        std::stoi(interfaceDefinition.at("interfaceMode").get<const std::string>());

      switch (interfaceMode) {
        case EFTrackingXrtParameters::InterfaceMode::INPUT: {
          m_inputDataStreamKeys.emplace_back(storeGateKey);
          ATH_CHECK(m_inputDataStreamKeys.back().initialize());

          m_inputBuffers.emplace_back(
            *m_device, 
            sizeof(unsigned long) * m_bufferSize, 
            xrt::bo::flags::normal, 
            m_kernels.back()->group_id(argumentIndex)
          );

          m_runs.back()->set_arg(argumentIndex, m_inputBuffers.back());

          break;
        }
        case EFTrackingXrtParameters::InterfaceMode::OUTPUT: {
          m_outputDataStreamKeys.emplace_back(storeGateKey);
          ATH_CHECK(m_outputDataStreamKeys.back().initialize());

          m_outputBuffers.emplace_back(
            *m_device, 
            sizeof(unsigned long) * m_bufferSize, 
            xrt::bo::flags::normal, 
            m_kernels.back()->group_id(argumentIndex)
          );

          m_runs.back()->set_arg(argumentIndex, m_outputBuffers.back());

          break;
        }
        case EFTrackingXrtParameters::InterfaceMode::VSIZE: {
          m_vsizes.push_back({.runIndex = static_cast<int>(m_runs.size()) - 1,
                              .argumentIndex = argumentIndex,
                              .storeGateKey = SG::ReadHandleKey<std::vector<long unsigned int>>(storeGateKey)});

          ATH_CHECK(m_vsizes.back().storeGateKey.initialize());

          break;
        }
        default: {
          ATH_MSG_ERROR("Failed to map kernel definitions to xrt objects.");     

          return StatusCode::FAILURE;
        }
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
    unsigned long* inputMap = m_inputBuffers.at(handleIndex).map<unsigned long*>();

    if (inputDataStream->size() > m_bufferSize) {
      ATH_MSG_ERROR("Input data stream too large for buffer (buffer: " << 
                    m_bufferSize << 
                    ", data stream: " << 
                    inputDataStream->size() << 
                    "), consider increasing buffer size.");

      return StatusCode::FAILURE;
    }

    {
      Athena::Chrono  chrono(
        "Copy " + inputDataStream.name() + " from storegate to host side map",
        m_chronoSvc.get()
      );

      for (std::size_t index = 0; index < inputDataStream->size(); index++) {
        inputMap[index] = inputDataStream->at(index);
      }
    }
    
    {
      Athena::Chrono  chrono(
        "Copy " + inputDataStream.name() + " from host side map to device",
        m_chronoSvc.get()
      );

      m_inputBuffers.at(handleIndex).sync(XCL_BO_SYNC_BO_TO_DEVICE);
    }

    handleIndex++;
  }

  ATH_MSG_DEBUG("Writing VSizes");
  for (const EFTrackingFPGAIntegration::VSize& vsize : m_vsizes) {
    SG::ReadHandle<std::vector<unsigned long>> inputDataStream(vsize.storeGateKey, ctx);

    m_runs.at(vsize.runIndex)->set_arg(vsize.argumentIndex, inputDataStream->size());
  }

  {
    Athena::Chrono  chrono("Run accelerated algorithms", m_chronoSvc.get());

    ATH_MSG_DEBUG("Starting Kernels");
    for (std::size_t index = 0; index < m_runs.size(); index++) {
      m_runs.at(index)->start();
    }

    ATH_MSG_DEBUG("Waiting for Kernels");
    for (std::size_t index = 0; index < m_runs.size(); index++) {
      m_runs.at(index)->wait();
    }
  }

  ATH_MSG_DEBUG("Reading Outputs");
  handleIndex = 0;
  for (
    const SG::WriteHandleKey<std::vector<unsigned long>>& outputDataStreamKey : 
    m_outputDataStreamKeys
  ) {
    SG::WriteHandle<std::vector<unsigned long>> outputDataStream(outputDataStreamKey, ctx);
    ATH_CHECK(outputDataStream.record(std::make_unique<std::vector<unsigned long>>(m_bufferSize)));

    {
      Athena::Chrono  chrono(
        "Copy " + outputDataStream.name() + " from device to host side map",
        m_chronoSvc.get()
      );

      m_outputBuffers.at(handleIndex).sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    }

    const unsigned long* outputMap = m_outputBuffers.at(handleIndex).map<unsigned long*>();

    {
      Athena::Chrono  chrono(
        "Copy " + outputDataStream.name() + " from host side map to storegate",
        m_chronoSvc.get()
      );

      for (std::size_t index = 0; index < outputDataStream->size(); index++) {
        outputDataStream->at(index) = outputMap[index];
      }
    };

    handleIndex++;
  }

  return StatusCode::SUCCESS;
}

