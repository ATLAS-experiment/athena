//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXXRT_VECTORMULTXRTEXAMPLEALG_H
#define ATHEXXRT_VECTORMULTXRTEXAMPLEALG_H

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
/// a classic hello world FPGA HLS kernel (Vector multiplication) in Athena
/// framework using the AthXRT device manager service prototype.
///
/// @author Quentin Berthet <quentin.berthet@cern.ch>
///
class VectorMultXRTExampleAlg : public AthReentrantAlgorithm, public AthXRT::StateHandler {

 public:
  // Inherit the base class's constructor(s).
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override {
      return StateHandler::initialize();
  }

  virtual StatusCode initialize_global() override;
  virtual StatusCode initialize_worker() override;

  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  /// The XRT device manager to use
  ServiceHandle<AthXRT::IDeviceMgmtSvc> m_DeviceMgmtSvc{
      this, "DeviceMgmtSvc", "AthXRT::DeviceMgmtSvc",
      "The XRT device manager service to use"};

  // Kernel name string
  static constexpr char s_krnl_name[] = "krnl_VectorMult";

  // list of found accelerator devices
  std::vector<std::shared_ptr<xrt::device>> m_devices;

  // Kernel arguments indexes
  // Must match the kernel arguments order.
  static constexpr int s_krnl_param_in1 = 0;
  static constexpr int s_krnl_param_in2 = 1;
  static constexpr int s_krnl_param_out = 2;
  static constexpr int s_krnl_param_size = 3;

  // Number of uint32_t element in the vectors
  static constexpr int s_element_count = 4096;

  /// Slot-specific state.
  struct SlotData {
    /// Device pointer
    std::shared_ptr<xrt::device> m_device = nullptr;

    /// Kernel object
    std::unique_ptr<xrt::kernel> m_kernel = nullptr;

    /// Kernel run object
    std::unique_ptr<xrt::run> m_run = nullptr;

    /// Buffer objects
    std::unique_ptr<xrt::bo> m_bo_in1 = nullptr;
    std::unique_ptr<xrt::bo> m_bo_in2 = nullptr;
    std::unique_ptr<xrt::bo> m_bo_out = nullptr;
  };

  /// List of slot-specific data.
  SG::SlotSpecificObj<SlotData> m_slots;

};  // class VectorMultXRTExampleAlg

}  // namespace AthExXRT

#endif  // ATHEXXRT_VECTORMULTXRTEXAMPLEALG_H
