/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_XRT_ALGORITHM
#define EFTRACKING_XRT_ALGORITHM

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"

// Gaudi::Property<std::map<std::string, std::vector<std::map<std::string, std::string>>>> 
#include "Gaudi/Parsers/Factory.h" 
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

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

 public:
  EFTrackingXrtAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

