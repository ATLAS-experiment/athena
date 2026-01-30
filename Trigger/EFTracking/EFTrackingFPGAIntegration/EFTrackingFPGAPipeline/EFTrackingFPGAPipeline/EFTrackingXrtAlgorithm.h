/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_XRT_ALGORITHM
#define EFTRACKING_XRT_ALGORITHM

#include <memory>
#include <map>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthXRTInterfaces/IDeviceMgmtSvc.h"
#include "Gaudi/Property.h"
#include "Gaudi/Parsers/Factory.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

/**
 *  @class EFTrackingXrtAlgorithm
 *         Generic Athena algorithm for running xclbin kernels, creating a 
 *         mapping between store gate keys and kernel interfaces
 *
 *         Three interface types are supported, inputs, vSizes and outputs.
 *         Inputs and outputs are for memory mapped interfaces.
 *         VSizes tell the kernel how long an input is (based on the `size()` of 
 *         the associated `std::vector` (retrieved from store gate).
 */
class EFTrackingXrtAlgorithm : public AthReentrantAlgorithm
{
  /**
   * @brief Keys to access encoded 64bit words following the EFTracking specification.
   */
  SG::ReadHandleKeyArray<std::vector<unsigned long>> m_inputDataStreamKeys{this, "inputDataStreamKeys", {}};
  SG::ReadHandleKeyArray<std::vector<unsigned long>> m_vSizeDataStreamKeys{this, "vSizeDataStreamKeys", {}};
  SG::WriteHandleKeyArray<std::vector<unsigned long>> m_outputDataStreamKeys{this, "outputDataStreamKeys", {}};

  ServiceHandle<AthXRT::IDeviceMgmtSvc> m_DeviceMgmtSvc{
    this, 
    "DeviceMgmtSvc", 
    "AthXRT::DeviceMgmtSvc",
    "The XRT device manager service to use"
  };

  ServiceHandle<IChronoSvc> m_chronoSvc{
    this,
    "ChronoStatSvc",
    "ChronoStatSvc",
    "Stop watch"
  };

  Gaudi::Property<std::vector<std::tuple<std::string, std::string, int>>> m_inputInterfaces {
    this,
    "inputInterfaces",
    {},
    ""
  };

  Gaudi::Property<std::vector<std::tuple<std::string, std::string, int>>> m_vSizeInterfaces {
    this,
    "vSizeInterfaces",
    {},
    ""
  };

  Gaudi::Property<std::vector<std::tuple<std::string, std::string, int>>> m_outputInterfaces {
    this,
    "outputInterfaces",
    {},
    ""
  };

  Gaudi::Property<std::vector<std::tuple<std::string, int, std::string, int>>> m_sharedInterfaces {
    this,
    "sharedInterfaces",
    {},
    ""
  };

  Gaudi::Property<std::vector<std::vector<std::string>>> m_kernelOrder {
    this,
    "kernelOrder",
    {},
    ""
  };

  Gaudi::Property<std::size_t> m_bufferSize {
    this,
    "bufferSize",
    8192,
    "Capacity of xrt buffers in terms of 64bit words."
  };

  std::map<std::string, std::unique_ptr<xrt::kernel>> m_kernels{};
  std::map<std::string, std::unique_ptr<xrt::run>> m_runs{};

  // Buffer objects
  mutable std::vector<xrt::bo> m_inputBuffers ATLAS_THREAD_SAFE {};
  mutable std::vector<xrt::bo> m_outputBuffers ATLAS_THREAD_SAFE {};
  
  std::optional<xrt::bo::flags> determine_mem_flags(
    const std::unique_ptr<xrt::kernel>& kernel,
    const std::size_t index
  ) const;

 public:
  EFTrackingXrtAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

