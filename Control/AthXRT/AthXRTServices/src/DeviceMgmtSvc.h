//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHXRTSERVICES_DEVICEMGMTSVC_H
#define ATHXRTSERVICES_DEVICEMGMTSVC_H

// AthXRT include(s).
#include "AthXRTInterfaces/IDeviceMgmtSvc.h"

// Framework include(s).
#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Parsers/Factory.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

// System include(s).
#include <string>

namespace AthXRT {

/// @brief Service used for programming XRT compatible accelerators.
///
/// This service aims to be a central point for managing XRT compatible
/// accelerators. It will enumerate the available devices, program them
/// with the specified XCLBIN files and provide a way for client algorithms
/// to request lists of devices by kernel name, either using native XRT
/// API or OpenCL API.
///
/// @author Quentin Berthet <quentin.berthet@cern.ch>
///
class DeviceMgmtSvc : public extends<AthService, IDeviceMgmtSvc> {

 public:
  /// Inherit the base class's constructor(s)
  using extends::extends;

  /// @name Interface inherited from @c IService
  /// @{

  /// Initialise the service
  virtual StatusCode initialize() override;

  /// Finalise the service
  virtual StatusCode finalize() override;

  /// @}

  /// @name Interface inherited from @c AthXRT::IDeviceMgmtSvc
  /// @{

  /// @brief Get a list of OpenCL handles providing the specified kernel.
  /// @param name The name of the kernel to search for.
  /// @return A vector of OpenCL handles structs providing the specified kernel.
  virtual const std::vector<IDeviceMgmtSvc::OpenCLHandle>
  get_opencl_handles_by_kernel_name(const std::string &name) const override;

  ///@brief Get a list of XRT devices providing the specified kernel.
  /// @param name The name of the kernel to search for.
  /// @return A vector of XRT device handles providing the specified kernel.
  virtual const std::vector<std::shared_ptr<xrt::device>>
  get_xrt_devices_by_kernel_name(const std::string &name) const override;

  /// @}

 private:
  /// @name Service properties
  /// @{

  /// The list of xclbin files to use.
  /// This is a list of paths to XCLBIN files to load on
  /// accelerator devices.
  Gaudi::Property<std::vector<std::string>> m_xclbin_path_list{
      this,
      "XclbinPathsList",
      {},
      "The list of XCLBIN files to program on FPGAs"};

  /// @}

  // Structure used to gather information about the devices and
  // XCLBIN files available on the system during inspect_devices() and
  // inspect_xclbins() functions. Later used by pair_devices_and_xclbins(),
  // to make decision on which XCLBIN file to program on which device.
  // Not created as a class member, as it is only used in the
  // initialize() function.
  struct SystemInfo {
    // Used to accumulate the devices (inner vector) by device
    // types (outter vector) during initialization, as final
    // grouping (by contexts) is not known yet.
    std::vector<std::vector<cl::Device>> device_types;

    std::size_t device_count;
    std::size_t different_xclbin_count;
    std::size_t different_xclbin_fpga_device_name;
  };

  // Helpers to get device information.
  std::string get_device_name(const cl::Device &device) const;
  std::string get_device_bdf(const cl::Device &device) const;

  // Main functions, called in sequence during initialize().
  // Split into multiple methods for code readability, but have no other
  // reason to be called separately.
  StatusCode inspect_devices(SystemInfo &si);
  StatusCode inspect_xclbins(SystemInfo &si);
  StatusCode pair_devices_and_xclbins(const SystemInfo &si);
  StatusCode program_devices();

  /// Struct to hold information about an XCLBIN file, as
  /// well as the kernels it contains.
  struct XclbinInfo {
    std::string path;
    std::string xsa_name;
    std::string fpga_device_name;
    std::string uuid;
    std::vector<std::string> kernel_names;
    std::vector<std::string> cu_names;
  };

  /// Helper function to check if an XCLBIN file is compatible with a device.
  bool is_xclbin_compatible_with_device(
      const DeviceMgmtSvc::XclbinInfo &xclbin_info,
      const cl::Device &device) const;

  /// List of XCLBIN files info configured for the service.
  std::vector<XclbinInfo> m_xclbin_infos;

  /// Struct to hold information about a context, as well as the devices,
  /// the program and XCLBIN file associated with the context.
  struct AthClContext {
    std::shared_ptr<cl::Context> context;
    std::vector<cl::Device> devices;
    XclbinInfo xclbin_info;
    std::shared_ptr<cl::Program> program;
  };

  /// List of contexts configured for the service.
  std::vector<AthClContext> m_ath_cl_contexts;

};  // class DeviceMgmtSvc

}  // namespace AthXRT

#endif  // ATHXRTSERVICES_DEVICEMGMTSVC_H
