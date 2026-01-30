/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "AthenaKernel/Chrono.h"

#include "EFTrackingFPGAPipeline/EFTrackingXrtAlgorithm.h"

EFTrackingXrtAlgorithm::EFTrackingXrtAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingXrtAlgorithm::initialize() {
  ATH_MSG_INFO("Initializing " << name());

  ATH_CHECK(m_DeviceMgmtSvc.retrieve());
  ATH_CHECK(m_chronoSvc.retrieve());
  ATH_CHECK(m_inputDataStreamKeys.initialize());
  ATH_CHECK(m_vSizeDataStreamKeys.initialize());
  ATH_CHECK(m_outputDataStreamKeys.initialize());

  for (const auto& [kernelName, storeGateKey, argumentIndex] : m_inputInterfaces) {
    ATH_MSG_DEBUG(
      "Setting up " << 
      kernelName << 
      " to read " << 
      storeGateKey << 
      " into argument " << 
      argumentIndex
    );

    const std::vector<std::shared_ptr<xrt::device>> devices = 
      m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    ATH_CHECK(devices.size() != 0);

    if (!m_kernels.contains(kernelName)) {
      m_kernels[kernelName] = std::make_unique<xrt::kernel>(
        *(devices[0]),
        (devices[0])->get_xclbin_uuid(),
        kernelName,
        xrt::kernel::cu_access_mode::exclusive
      );
    }

    ATH_CHECK(m_kernels[kernelName].get() != nullptr);

    const std::optional<xrt::bo::flags> mem_flags = 
      determine_mem_flags(m_kernels[kernelName], argumentIndex);

    if (!mem_flags.has_value()) {
      ATH_MSG_WARNING(
          "Unable to determine mem_flags for argument with index " << 
          argumentIndex <<
          " in kernel named " <<
          kernelName << 
          ". Defaulting to xrt::bo::normal. Good luck!"
      );

      m_inputBuffers.emplace_back(
        *(devices[0]), 
        sizeof(unsigned long) * m_bufferSize, 
        xrt::bo::flags::normal, 
        m_kernels[kernelName]->group_id(argumentIndex)
      );
    }
    else {
      m_inputBuffers.emplace_back(
        *(devices[0]), 
        sizeof(unsigned long) * m_bufferSize, 
        mem_flags.value(), 
        m_kernels[kernelName]->group_id(argumentIndex)
      );
    }

    if (!m_runs.contains(kernelName)) {
      m_runs[kernelName] = std::make_unique<xrt::run>(*m_kernels[kernelName]);
    }

    ATH_CHECK(m_runs[kernelName].get() != nullptr);
    m_runs[kernelName]->set_arg(argumentIndex, m_inputBuffers.back());
  }

  for (const auto& [kernelName, storeGateKey, argumentIndex] : m_vSizeInterfaces) {
    ATH_MSG_DEBUG(
      "Setting up " << 
      kernelName << 
      " to get input size from " << 
      storeGateKey << 
      " for argument " << 
      argumentIndex
    );

    const std::vector<std::shared_ptr<xrt::device>> devices = 
      m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    ATH_CHECK(devices.size() != 0);

    if (!m_kernels.contains(kernelName)) {
      m_kernels[kernelName] = std::make_unique<xrt::kernel>(
        *(devices[0]),
        (devices[0])->get_xclbin_uuid(),
        kernelName,
        xrt::kernel::cu_access_mode::exclusive
      );
    }

    ATH_CHECK(m_kernels[kernelName].get() != nullptr);

    if (!m_runs.contains(kernelName)) {
      m_runs[kernelName] = std::make_unique<xrt::run>(*m_kernels[kernelName]);
    }

    ATH_CHECK(m_runs[kernelName].get() != nullptr);
  }

  for (const auto& [kernelName, storeGateKey, argumentIndex] : m_outputInterfaces) {
    ATH_MSG_DEBUG(
      "Setting up " << 
      kernelName << 
      " to write " << 
      storeGateKey << 
      " from argument " << 
      argumentIndex
    );

    const std::vector<std::shared_ptr<xrt::device>> devices = 
      m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    ATH_CHECK(devices.size() != 0);

    if (!m_kernels.contains(kernelName)) {
      m_kernels[kernelName] = std::make_unique<xrt::kernel>(
        *(devices[0]),
        devices[0]->get_xclbin_uuid(),
        kernelName,
        xrt::kernel::cu_access_mode::exclusive
      );
    }

    const std::optional<xrt::bo::flags> mem_flags = 
      determine_mem_flags(m_kernels[kernelName], argumentIndex);

    if (!mem_flags.has_value()) {
      ATH_MSG_WARNING(
          "Unable to determine mem_flags for argument with index " << 
          argumentIndex <<
          " in kernel named " <<
          kernelName << 
          ". Defaulting to xrt::bo::normal. Good luck!"
      );

      m_outputBuffers.emplace_back(
        *(devices[0]), 
        sizeof(unsigned long) * m_bufferSize, 
        xrt::bo::flags::normal, 
        m_kernels[kernelName]->group_id(argumentIndex)
      );
    }
    else {
      m_outputBuffers.emplace_back(
        *(devices[0]), 
        sizeof(unsigned long) * m_bufferSize, 
        mem_flags.value(), 
        m_kernels[kernelName]->group_id(argumentIndex)
      );
    }

    if (!m_runs.contains(kernelName)) {
      m_runs[kernelName] = std::make_unique<xrt::run>(*m_kernels[kernelName]);
    }

    ATH_CHECK(m_runs[kernelName].get() != nullptr);
    m_runs[kernelName]->set_arg(argumentIndex, m_outputBuffers.back());
  }

  for (const auto& [kernelName, argumentIndex, sourceKernelName, sourceArgumentIndex] : m_sharedInterfaces) {
    ATH_MSG_DEBUG(
      "Setting up shared buffer between " << 
      kernelName << 
      " argument " << 
      argumentIndex << 
      " and " << 
      sourceKernelName <<
      " argument " <<
      sourceArgumentIndex
    );

    const std::vector<std::shared_ptr<xrt::device>> devices = 
      m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

    ATH_CHECK(devices.size() != 0);

    if (!m_kernels.contains(kernelName)) {
      m_kernels[kernelName] = std::make_unique<xrt::kernel>(
        *(devices[0]),
        devices[0]->get_xclbin_uuid(),
        kernelName,
        xrt::kernel::cu_access_mode::exclusive
      );
    }

    if (!m_runs.contains(kernelName)) {
      m_runs[kernelName] = std::make_unique<xrt::run>(*m_kernels[kernelName]);
    }

    ATH_CHECK(m_runs[kernelName].get() != nullptr);

    std::size_t index = 0;
    for (const auto& [outputKernelName, outputStoreGateKey, outputArgumentIndex] : m_outputInterfaces) {
      if (
        outputKernelName == sourceKernelName &&
        outputArgumentIndex == sourceArgumentIndex
      ) {
        m_runs[kernelName]->set_arg(argumentIndex, m_outputBuffers[index]);

        break;
      }

      index++;
    }
  }

  for (const auto& kernelNames : m_kernelOrder) {
    for (const auto& kernelName : kernelNames) {
      const std::vector<std::shared_ptr<xrt::device>> devices = 
        m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(kernelName);

      ATH_CHECK(devices.size() != 0);

      if (!m_kernels.contains(kernelName)) {
        ATH_MSG_DEBUG("Creating kernel: " << kernelName);

        m_kernels[kernelName] = std::make_unique<xrt::kernel>(
          *(devices[0]),
          devices[0]->get_xclbin_uuid(),
          kernelName,
          xrt::kernel::cu_access_mode::exclusive
        );

        if (!m_runs.contains(kernelName)) {
          m_runs[kernelName] = std::make_unique<xrt::run>(*m_kernels[kernelName]);
        }

        ATH_CHECK(m_runs[kernelName].get() != nullptr);
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingXrtAlgorithm::execute(const EventContext& ctx) const
{
  ATH_CHECK(m_inputDataStreamKeys.size() == m_inputBuffers.size());
  std::size_t inputHandleIndex = 0;
  for (
    const SG::ReadHandleKey<std::vector<unsigned long>>& inputDataStreamKey : 
    m_inputDataStreamKeys
  ) {
    SG::ReadHandle<std::vector<unsigned long>> inputDataStream(inputDataStreamKey, ctx);
    ATH_MSG_DEBUG("Writing: " << inputDataStream.name());
    unsigned long* inputMap = m_inputBuffers.at(inputHandleIndex).map<unsigned long*>();

    ATH_CHECK(inputDataStream->size() <= m_bufferSize);

    ATH_MSG_DEBUG("Copy " + inputDataStream.name() + " from storegate to host side map");
    {
      Athena::Chrono chrono(
        "Copy " + inputDataStream.name() + " from storegate to host side map",
        m_chronoSvc.get()
      );

      for (std::size_t index = 0; index < inputDataStream->size(); index++) {
        inputMap[index] = inputDataStream->at(index);
      }
    }
    
    ATH_MSG_DEBUG("Copy " + inputDataStream.name() + " from host side map to device");
    {
      Athena::Chrono chrono(
        "Copy " + inputDataStream.name() + " from host side map to device",
        m_chronoSvc.get()
      );

      m_inputBuffers.at(inputHandleIndex).sync(XCL_BO_SYNC_BO_TO_DEVICE);
    }

    inputHandleIndex++;
  }

  ATH_CHECK(m_vSizeDataStreamKeys.size() == m_vSizeInterfaces.size());
  std::size_t vSizeHandleIndex = 0;
  for (
    const SG::ReadHandleKey<std::vector<unsigned long>>& vSizeDataStreamKey : 
    m_vSizeDataStreamKeys
  ) {
    SG::ReadHandle<std::vector<unsigned long>> vSizeDataStream(vSizeDataStreamKey, ctx);
    const auto& [kernelName, storeGateKey, argumentIndex] = m_vSizeInterfaces[vSizeHandleIndex];
    ATH_MSG_DEBUG("Setting VSize: " << kernelName << ", " << vSizeDataStream.name() << ", " << vSizeDataStream->size());

    m_runs.at(kernelName)->set_arg(argumentIndex, vSizeDataStream->size());
    vSizeHandleIndex++;
  }

  ATH_MSG_DEBUG("Run kernels");
  {
    Athena::Chrono chrono("Run kernels", m_chronoSvc.get());

    for (const auto& kernelNames : m_kernelOrder) {
      for (const auto& kernelName : kernelNames) {
        ATH_MSG_DEBUG("Running: " << kernelName);
        m_runs.at(kernelName)->start();
      }

      for (const auto& kernelName : kernelNames) {
        ATH_MSG_DEBUG("Waiting: " << kernelName);
        m_runs.at(kernelName)->wait();
      }
    }
  }

  std::size_t outputHandleIndex = 0;
  for (
    const SG::WriteHandleKey<std::vector<unsigned long>>& outputDataStreamKey : 
    m_outputDataStreamKeys
  ) {
    SG::WriteHandle<std::vector<unsigned long>> outputDataStream(outputDataStreamKey, ctx);
    ATH_CHECK(outputDataStream.record(std::make_unique<std::vector<unsigned long>>(m_bufferSize)));

    ATH_MSG_DEBUG("Copy " + outputDataStream.name() + " from device to host side map");
    {
      Athena::Chrono chrono(
        "Copy " + outputDataStream.name() + " from device to host side map",
        m_chronoSvc.get()
      );

      m_outputBuffers.at(outputHandleIndex).sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    }

    const unsigned long* outputMap = m_outputBuffers.at(outputHandleIndex).map<unsigned long*>();
    ATH_MSG_DEBUG("Copy " + outputDataStream.name() + " from host side map to storegate");
    {
      Athena::Chrono chrono(
        "Copy " + outputDataStream.name() + " from host side map to storegate",
        m_chronoSvc.get()
      );

      for (std::size_t index = 0; index < outputDataStream->size(); index++) {
        outputDataStream->at(index) = outputMap[index];
      }
    }

    outputHandleIndex++;
  }

  return StatusCode::SUCCESS;
}

std::optional<xrt::bo::flags> EFTrackingXrtAlgorithm::determine_mem_flags(
  const std::unique_ptr<xrt::kernel>& kernel,
  const std::size_t index
) const {
  for (
    const xrt::xclbin::kernel& kernelMetaData : 
    kernel->get_xclbin().get_kernels()
  ) {
    if (kernel->get_name() != kernelMetaData.get_name()) {
      continue;
    }

    for (const xrt::xclbin::arg& arg : kernelMetaData.get_args()) {
      if (arg.get_index() != index) {
        continue;
      }

      if (arg.get_mems().size() == 0) {
        ATH_MSG_WARNING(
          "No mems associated with argument " << 
          index <<
          " of " <<
          kernel->get_name() <<
          ". Expect more warnings."
        );

        return std::nullopt;
      }
      
      if (arg.get_mems()[0].get_tag().find("HOST") != std::string::npos) {
        return xrt::bo::flags::host_only;
      }

      return xrt::bo::flags::normal;
    }
  }

  return std::nullopt;
}

