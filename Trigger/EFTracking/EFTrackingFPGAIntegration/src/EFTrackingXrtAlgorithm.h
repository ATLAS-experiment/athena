/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_XRT_ALGORITHM
#define EFTRACKING_XRT_ALGORITHM

#include <memory>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthXRTInterfaces/IDeviceMgmtSvc.h"
#include "Gaudi/Property.h"

// Gaudi::Property<std::map<std::string, std::vector<std::map<std::string, std::string>>>> 
#include "Gaudi/Parsers/Factory.h" 
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

/**
 *  @class EFTrackingXrtAlgorithm
 *         Generic Athena algorithm for running xclbins (FPGA firmware). The 
 *         idea is to associate a set store gate
 *         handles with memory mapped kernel interfaces (kernels and interfaces 
 *         defined in the xclbin). 
 *
 *         The objects behind these store gate handles are then written directly 
 *         to the FPGAs global memory. The kernels are then run. Finally outputs 
 *         from the FPGAs global memory are written back into storegate.
 */
class EFTrackingXrtAlgorithm : public AthReentrantAlgorithm
{
  /**
   * @brief Keys to access encoded 64bit words following the EFTracking specification.
   */
  std::vector<SG::ReadHandleKey<std::vector<unsigned long>>> m_inputDataStreamKeys{};
  std::vector<SG::WriteHandleKey<std::vector<unsigned long>>> m_outputDataStreamKeys{};

  ServiceHandle<AthXRT::IDeviceMgmtSvc> m_DeviceMgmtSvc{
    this, 
    "DeviceMgmtSvc", 
    "AthXRT::DeviceMgmtSvc",
    "The XRT device manager service to use"
  };
  
  Gaudi::Property<std::string> m_xclbinPath{
    this,
    "xclbinPath", 
    "", 
    "Path to Xilinx Compute Language Binary (firmware)."
  };

  Gaudi::Property<
    std::map<std::string, std::vector<std::map<std::string, std::string>>>
  > m_kernelDefinitions{
    this,
    "kernelDefinitions", 
    {}, 
    "List of named kernels."
  };

  Gaudi::Property<std::size_t> m_bufferSize {
    this,
    "bufferSize",
    8192,
    "Capacity of xrt buffers in terms of 64bit words."
  };
  // Device pointer
  std::shared_ptr<xrt::device> m_device{};

  // Kernel objects
  std::vector<std::unique_ptr<xrt::kernel>> m_kernels{};

  // Kernel run objects
  std::vector<std::unique_ptr<xrt::run>> m_runs{};

  // Buffer objects
  mutable std::vector<xrt::bo> m_inputBuffers ATLAS_THREAD_SAFE {};
  mutable std::vector<xrt::bo> m_outputBuffers ATLAS_THREAD_SAFE {};

  std::shared_ptr<xrt::device> getDevice();
  bool deviceHasKernels(
    const std::shared_ptr<xrt::device>& device
  ) const;

  bool deviceHasKernel(
    const std::shared_ptr<xrt::device>& device,
    const std::vector<std::shared_ptr<xrt::device>>& devices
  ) const;

 public:
  EFTrackingXrtAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

