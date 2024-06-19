//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHXRTINTERFACES_IDEVICEMGMTSVC_H
#define ATHXRTINTERFACES_IDEVICEMGMTSVC_H

// Gaudi include(s).
#include "GaudiKernel/IService.h"
#include "GaudiKernel/StatusCode.h"

// XRT includes
#include <experimental/xrt_system.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

// AMD XRT OpenCL implementation requires these macros to be defined
// before including the OpenCL headers to enable OpenCL 1.2 API.
#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

// AMD OpenCL implementation includes.
#include <CL/cl2.hpp>
#include <CL/cl2xrt.hpp>

namespace AthXRT {

/// Interface for @c AthXRT::DeviceMgmtSvc service
///
/// The implementation of this service is supposed to program
/// XRT accelerator and return device and xclbin uuid handles
/// required by user code to create kernels, buffer objects and
/// runs.
///
/// @author Quentin Berthet <quentin.berthet@cern.ch>
///
class IDeviceMgmtSvc : public virtual IService {

 public:
  /// Declare the interface ID
  DeclareInterfaceID(AthXRT::IDeviceMgmtSvc, 1, 0);

  /// @brief Struct holding OpenCL handles for a kernel.
  struct OpenCLHandle {
    std::shared_ptr<cl::Context> context{nullptr};
    std::shared_ptr<cl::Program> program{nullptr};
  };

  /// @brief Get a list of OpenCL handles providing the specified kernel.
  virtual const std::vector<IDeviceMgmtSvc::OpenCLHandle>
  get_opencl_handles_by_kernel_name(const std::string &name) const = 0;

  /// @brief Get a list of XRT devices providing the specified kernel.
  virtual const std::vector<std::shared_ptr<xrt::device>>
  get_xrt_devices_by_kernel_name(const std::string &name) const = 0;

};  // class IDeviceMgmtSvc

}  // namespace AthXRT

#endif  // ATHXRTINTERFACES_IDEVICEMGMTSVC_H
