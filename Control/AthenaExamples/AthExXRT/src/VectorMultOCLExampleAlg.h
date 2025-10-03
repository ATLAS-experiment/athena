//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXXRT_VECTORMULTOCLEXAMPLEALG_H
#define ATHEXXRT_VECTORMULTOCLEXAMPLEALG_H

// STL include(s).
#include <memory>

// AthXRT include(s).
#include "AthXRTInterfaces/IDeviceMgmtSvc.h"
#include "AthXRTInterfaces/StateHandler.h"

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "GaudiKernel/ServiceHandle.h"

namespace AthExXRT {

/// Example algorithm exercising the @c AthXRT core services
///
/// This example code is intended to demonstrate the integration of
/// a classic hello world FPGA HLS kernel (Vector mltiplication) in Athena
/// framework using the AthXRT device manager service prototype.
///
/// @author Quentin Berthet <quentin.berthet@cern.ch>
///
class VectorMultOCLExampleAlg : public ::AthReentrantAlgorithm, public AthXRT::StateHandler {

 public:
	 using ::AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override {
      return StateHandler::initialize();
  }

  virtual StatusCode initialize_global() override;
  virtual StatusCode initialize_worker() override;

  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

  virtual StatusCode stop() override {
      return StateHandler::stop();
  }
  virtual StatusCode stop_worker() override;

 private:
  /// The XRT device manager to use
  ServiceHandle<AthXRT::IDeviceMgmtSvc> m_DeviceMgmtSvc{
      this, "DeviceMgmtSvc", "AthXRT::DeviceMgmtSvc",
      "The XRT device manager service to use"};

  // Kernel name string
  static constexpr char s_krnl_name[] = "krnl_VectorMult";

  // Kernel arguments indexes
  // Must match the kernel arguments order.
  static constexpr int s_krnl_param_in1 = 0;
  static constexpr int s_krnl_param_in2 = 1;
  static constexpr int s_krnl_param_out = 2;
  static constexpr int s_krnl_param_size = 3;

  // Number of uint32_t element in the vectors
  static constexpr int s_element_count = 4096;

  std::vector<AthXRT::IDeviceMgmtSvc::OpenCLHandle> m_handles;

  /// Slot-specific state.
  struct SlotData {
    /// OpenCL context pointer
    std::shared_ptr<cl::Context> m_context = nullptr;

    /// OpenCL program pointer
    std::shared_ptr<cl::Program> m_program = nullptr;

    /// Kernel object
    std::unique_ptr<cl::Kernel> m_kernel = nullptr;

    /// Kernel run object
    std::unique_ptr<cl::CommandQueue> m_queue = nullptr;

    /// Buffer objects
    std::unique_ptr<cl::Buffer> m_dev_buf_in1 = nullptr;
    std::unique_ptr<cl::Buffer> m_dev_buf_in2 = nullptr;
    std::unique_ptr<cl::Buffer> m_dev_buf_out = nullptr;

    uint32_t* m_host_buf_in1 = nullptr;
    uint32_t* m_host_buf_in2 = nullptr;
    uint32_t* m_host_buf_out = nullptr;
  };

  /// List of slot-specific data.
  SG::SlotSpecificObj<SlotData> m_slots;

};  // class VectorMultOCLExampleAlg

}  // namespace AthExXRT

#endif  // ATHEXXRT_VECTORMULTOCLEXAMPLEALG_H
