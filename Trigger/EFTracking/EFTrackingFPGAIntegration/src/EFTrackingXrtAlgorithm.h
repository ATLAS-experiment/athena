/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_XRT_ALGORITHM
#define EFTRACKING_XRT_ALGORITHM

#include <memory>

#include <nlohmann/json.hpp>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthXRTInterfaces/IDeviceMgmtSvc.h"
#include "Gaudi/Property.h"

#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

namespace {
// Some kernels require the length of the input test vector. storeGateKey is 
// used to get the std::vector which we will call size() on to get the 
// test vector length.
struct VSize {
  const int runIndex;
  const int argumentIndex;
  SG::ReadHandleKey<std::vector<unsigned long>> storeGateKey;
};
}

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

  Gaudi::Property<std::string> m_kernelDefinitionsJsonString {
    this,
    "kernelDefinitionsJsonString",
    "{}",
    "String representation of the json kernel definitions."
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

  // VSize objects
  std::vector<VSize> m_vsizes{};

 public:
  EFTrackingXrtAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

