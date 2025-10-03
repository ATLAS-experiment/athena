//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "DeviceMgmtSvc.h"

// System include(s).
#include <filesystem>
#include <fstream>
#include <CL/cl_ext_xilinx.h>

namespace AthXRT {

/// @brief Inspect the available devices and fill the SystemInfo structure.
/// Find all available XRT compatible accelerator devices and group them by
/// type.
/// @param si The SystemInfo structure to fill
StatusCode DeviceMgmtSvc::inspect_devices(SystemInfo &si) {

  std::vector<cl::Platform> platforms;

  ATH_CHECK(cl::Platform::get(&platforms) == CL_SUCCESS);
  si.device_count = 0;
  for (const cl::Platform &platform : platforms) {

    // Filter platforms by name. Currently AMD FPGA platform is
    // still referenced as "Xilinx".
    std::string platform_name;
    ATH_CHECK(platform.getInfo(CL_PLATFORM_NAME, &platform_name) == CL_SUCCESS);
    if (platform_name != "Xilinx") {
      ATH_MSG_WARNING("Skipping unsuported platform " << platform_name);
      continue;
    }

    // Get devices list for the platform and count total devices.
    std::vector<cl::Device> devices;
    ATH_CHECK(platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices) ==
              CL_SUCCESS);
    ATH_MSG_DEBUG("Found XRT/OpenCL platform '" << platform_name << "' with "
                                                << devices.size()
                                                << " devices");
    si.device_count += devices.size();

    // Group devices by type, using the name property.
    // All similar devices should have the same name,
    // I.E: "xilinx_u250_gen3x16_xdma_shell_4_1"
    for (const cl::Device &device : devices) {
      const std::string device_name = get_device_name(device);

      // Check if we already have a device with the same name.
      bool found = false;
      for (std::vector<cl::Device> &list : si.device_types) {
        std::string list_device_name;
        ATH_CHECK(list[0].getInfo(CL_DEVICE_NAME, &list_device_name) ==
                  CL_SUCCESS);
        if (device_name == list_device_name) {
          found = true;
          list.push_back(device);
          break;
        }
      }
      if (!found) {
        std::vector<cl::Device> new_list = {device};
        si.device_types.push_back(new_list);
      }
    }
  }

  // We expect to have at least one device to program.
  if (si.device_count < 1) {
    // This should not be this catastrophic and we could fallback to
    // software implementation, but for now we will consider that an
    // accelerator have to be present if this service is configured.
    ATH_MSG_ERROR("No XRT device found");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("Found a total of "
                 << si.device_count << " AMD FPGA device(s) ("
                 << si.device_types.size() << " device type(s))");
  }

  return StatusCode::SUCCESS;

}  // DeviceMgmtSvc::inspect_devices()

/// @brief Inspect the provided XCLBIN files and fill the SystemInfo structure.
/// Gather information using XRT native API about the XCLBIN files and the
/// kernel(s) they contain and perform some basic sanity checks.
/// @param si The SystemInfo structure to fill
StatusCode DeviceMgmtSvc::inspect_xclbins(SystemInfo &si) {

  // We expect at least one XCLBIN file to be specified.
  if (m_xclbin_path_list.empty()) {
    ATH_MSG_ERROR("No XCLBIN list specified");
    return StatusCode::FAILURE;
  }

  // If there is more XCLBIN files to program than device(s), this
  // is probably an error (the opposite is ok).
  if (m_xclbin_path_list.size() > si.device_count) {
    ATH_MSG_ERROR(
        "More XCLBIN file(s) specified than "
        "devices type available ("
        << si.device_count << "): ");
    for (const std::string &xclbin_path : m_xclbin_path_list) {
      ATH_MSG_ERROR(xclbin_path);
    }
    return StatusCode::FAILURE;
  }

  // Inspect XCLBIN files.
  for (const std::string &xclbin_path : m_xclbin_path_list) {

    if (!std::filesystem::exists(xclbin_path)) {
      ATH_MSG_ERROR("XCLBIN file does not exist: " << xclbin_path);
      return StatusCode::FAILURE;
    }

    // Create a temporary XRT API XCLBIN object to use introspection
    // to gather some information. With this approach, we are loading
    // them twice from disk which is not optimal.
    DeviceMgmtSvc::XclbinInfo xclbin_info;
    try {
      xrt::xclbin xrt_xclbin(xclbin_path);
      xclbin_info.path = xclbin_path;
      xclbin_info.xsa_name = xrt_xclbin.get_xsa_name();
      xclbin_info.fpga_device_name = xrt_xclbin.get_fpga_device_name();
      xclbin_info.uuid = xrt_xclbin.get_uuid().to_string();

      for (const xrt::xclbin::kernel &kernel : xrt_xclbin.get_kernels()) {
        const std::string& kernelName = kernel.get_name();

        // Ensure that the kernel have a least one compute unit.
        // Having a kernel without compute unit is not a common use case,
        // but it is possible if a .xo with a kernel is linked in the
        // .xclbin, but the number of said kernel is set to 0.
        if (!kernel.get_cus().empty()) {
          xclbin_info.kernel_names.push_back(kernelName);
        }

        for (const xrt::xclbin::ip &computeUnit : kernel.get_cus()) {
          const std::string& computeUnitName = computeUnit.get_name();
          const std::string computeUnitIsolatedName = 
            computeUnitName.substr(kernelName.size() + 1);

          const std::string computeUnitUsableName = kernelName + ":{" + computeUnitIsolatedName + "}";

          xclbin_info.cu_names.push_back(std::move(computeUnitUsableName));
        }
      }
    } catch (const std::exception &e) {
      ATH_MSG_ERROR(e.what());
      ATH_MSG_ERROR("Could not create xrt::xclbin from " << xclbin_path);
      return StatusCode::FAILURE;
    }
    m_xclbin_infos.push_back(xclbin_info);
  }

  // Extract some more information from the XCLBIN collection:
  // The number of different XCLBIN files and the number of different
  // FPGA device names targeted by the XCLBIN files.
  std::set<std::string> uuids;
  std::set<std::string> fpga_device_names;
  for (const XclbinInfo &info : m_xclbin_infos) {
    uuids.insert(info.uuid);
    fpga_device_names.insert(info.fpga_device_name);
  }
  si.different_xclbin_count = uuids.size();
  si.different_xclbin_fpga_device_name = fpga_device_names.size();

  return StatusCode::SUCCESS;

}  // DeviceMgmtSvc::inspect_xclbin()

/// @brief Get the name of a cl::device.
/// @param device The device to get the name from.
/// @return The name of the device as a string, or "error" in case of failure.
std::string DeviceMgmtSvc::get_device_name(const cl::Device &device) const {

  cl_int err = CL_SUCCESS;
  std::string device_name;
  err = device.getInfo(CL_DEVICE_NAME, &device_name);
  if (err != CL_SUCCESS) {
    ATH_MSG_ERROR("Failed to get device name");
    return std::string("error");
  }
  return device_name;
}

/// @brief Get the BDF (bus:device:function) string of a cl::device.
/// @param device The device to get the BDF from.
/// @return The BDF of the device as a string, or "error" in case of failure.
std::string DeviceMgmtSvc::get_device_bdf(const cl::Device &device) const {

  cl_int err = CL_SUCCESS;
  std::string device_bdf;
  err = device.getInfo(CL_DEVICE_PCIE_BDF, &device_bdf);
  if (err != CL_SUCCESS) {
    ATH_MSG_ERROR("Failed to get device BDF");
    return std::string("error");
  }
  return device_bdf;
}

/// @brief Get substring up to the nth occurrence of a token.
/// @param str The string to process.
/// @param token The token to search for.
/// @param n The number of occurence to search for.
/// @return The substring up to and including the nth occurrence of the token.
///         If the token is not found n times, the whole string is returned.
static std::string getPrefixUpToNthOccurrence(const std::string &str,
                                              char token, int n) {

  std::size_t pos = 0;
  int count = 0;

  // Iterate through the string to find the nth occurrence of the token
  while (count < n && pos != std::string::npos) {
    pos = str.find(token, pos + 1);
    count++;
  }

  // If the token isn't found n times, return the whole string
  if (pos == std::string::npos) {
    return str;
  }

  // Return the substring up to and including the nth occurrence
  return str.substr(0, pos + 1);
}

/// @brief Check if an XCLBIN is compatible with a device.
/// This is done by comparing the device name and the XSA name used in the
/// XCLBIN file up to the second occurrence of the underscore character. This
/// check is not a guarantee that the XCLBIN will work on the device, but
/// mismatching XSA names are a strong indicator of incompatibility.
/// @param xclbin_info The XCLBIN to check compatibility with.
/// @param device The device to check compatibility with.
/// @return True if the XCLBIN is compatible with the device, false otherwise.
bool DeviceMgmtSvc::is_xclbin_compatible_with_device(
    const DeviceMgmtSvc::XclbinInfo &xclbin_info,
    const cl::Device &device) const {

  const std::string device_prefix =
      getPrefixUpToNthOccurrence(get_device_name(device), '_', 2);
  const std::string xsa_prefix =
      getPrefixUpToNthOccurrence(xclbin_info.xsa_name, '_', 2);
  if (device_prefix == xsa_prefix) {
    return true;
  } else {
    return false;
  }
}

/// @brief Pair devices and XCLBINs and create contexts.
/// This function will pair devices and XCLBINsd epending on the number of
/// devices and XCLBINs, will attempt to program all provided XCLBINs according
/// to the following rules:
/// - If we have only one FPGA type and one XCLBIN: Program all devices with the
///   same XCLBIN and create one context.
/// - If we have only one FPGA type and multiple identical XCLBIN: Program the
///   same number of devices that we have XCLBIN files, but put them in only one
///   context as the XCLBIN will be identical for all programmed devices.
/// - If we have multiple type and multiple different XCLBIN, but all
///   targeting the same device: Program all devices with a different XCLBIN and
///   create one context per device/XCLBIN.
/// - If we have multiple type and multiple different XCLBIN, and the XCLBIN
///   files target multiple device types: Program each device with a matching
///   XCLBIN and create one context per device/XCLBIN.
/// Some devices might be left un-programmed if no matching XCLBIN is found.
/// @param si The SystemInfo structure containing information about the devices
///           and XCLBINs available on the system.
/// @return StatusCode::SUCCESS if the pairing was successful, or
///         StatusCode::FAILURE otherwise.
StatusCode DeviceMgmtSvc::pair_devices_and_xclbins(const SystemInfo &si) {

  // Do we have only one FPGA type?
  if (si.device_types.size() == 1) {

    if (m_xclbin_infos.size() == 1) {

      // We have one or multiple device(s) of the same type and only have one
      // XCLBIN: Program all device(s) with the same XCLBIN and create one
      // context.
      ATH_MSG_DEBUG("Case 1: One or multiple identical device(s), one xclbin");
      DeviceMgmtSvc::AthClContext ath_cl_context;
      for (const cl::Device &device : si.device_types[0]) {
        ath_cl_context.devices.push_back(device);
      }
      ath_cl_context.xclbin_info = m_xclbin_infos[0];
      m_ath_cl_contexts.push_back(ath_cl_context);

    } else {

      if (si.different_xclbin_fpga_device_name > 1) {

        // This is an error: we only have one FPGA type but
        // XCLBIN files targeting multiple fpga.
        ATH_MSG_ERROR(
            "Specified XCLBINs target multiple device types, but only one "
            "device type is present");
        return StatusCode::FAILURE;
      }

      if (si.different_xclbin_count == 1) {
        // We have multiple device of the same type and multiple identical
        // xclbin: Program the same number of devices that we have XCLBIN files,
        // but put them in only one context as the XCLBIN will be identical for
        // all programmed devices. Some devices might be left un-programmed.
        ATH_MSG_DEBUG(
            "Case 2: Multiple identical devices, multiple identical xclbins");
        DeviceMgmtSvc::AthClContext ath_cl_context;
        for (std::size_t i = 0; i < m_xclbin_infos.size(); ++i) {
          ath_cl_context.devices.push_back(si.device_types[0][i]);
        }
        ath_cl_context.xclbin_info = m_xclbin_infos[0];
        m_ath_cl_contexts.push_back(ath_cl_context);

      } else {

        // We have multiple device of the same type and multiple different
        // XCLBIN, but all targeting the same device: Program all devices with a
        // differnt XCLBIN and create one context per device/XCLBIN. Some
        // devices might be left un-programmed.
        ATH_MSG_DEBUG(
            "Case 3: Multiple identical devices, multiple different XCLBIN "
            "files, but targeting the same device type");
        for (std::size_t i = 0; i < m_xclbin_infos.size(); ++i) {
          DeviceMgmtSvc::AthClContext ath_cl_context;
          ath_cl_context.xclbin_info = m_xclbin_infos[i];
          ath_cl_context.devices.push_back(si.device_types[0][i]);
          m_ath_cl_contexts.push_back(ath_cl_context);
        }
      }
    }
  } else {

    // More tricky (and probably an edge case): we have different
    // devices types. We will try to pair each device them with a
    // XCLBIN files based on the device name and XCLBIN XSA name,
    // and load them in separate contexts.
    ATH_MSG_DEBUG("Case 4: Multiple different devices, multiple xclbins");
    std::vector<XclbinInfo> unaffected_xclbin_infos(m_xclbin_infos);
    for (const std::vector<cl::Device> &device_type : si.device_types) {
      for (const cl::Device &device : device_type) {
        DeviceMgmtSvc::AthClContext ath_cl_context;
        ath_cl_context.devices.push_back(device);

        // Try to find a matching XCLBIN for this device.
        std::vector<XclbinInfo>::iterator iter;
        bool found = false;
        for (iter = unaffected_xclbin_infos.begin();
             iter != unaffected_xclbin_infos.end();) {
          if (is_xclbin_compatible_with_device(*iter, device)) {
            ath_cl_context.xclbin_info = *iter;
            iter = unaffected_xclbin_infos.erase(iter);
            found = true;
            break;
          } else {
            ++iter;
          }
        }

        // Only keep this combination if we found a matching XCLBIN.
        if (found) {
          m_ath_cl_contexts.push_back(ath_cl_context);
        } else {
          // If we did not find a matching XCLBIN, we will not program the
          // device. This is not an error, but we will report it.
          ATH_MSG_WARNING("No compatible XCLBIN found for device "
                          << get_device_name(device) << " ("
                          << get_device_bdf(device) << ")");
        }
      }
    }

    for (const XclbinInfo &xclbin_info : unaffected_xclbin_infos) {
      // Report XCLBIN files that were not affected to a device.
      // (This could happen for XCLBIN not compatible with any device.)
      ATH_MSG_WARNING(
          "No compatible device found for XCLBIN: " << xclbin_info.path);
    }
  }

  return StatusCode::SUCCESS;

}  // DeviceMgmtSvc::pair_devices_and_xclbins()

/// @brief Program the devices with the XCLBIN files and create contexts.
/// This function will program the devices with the XCLBIN files and create
/// contexts based on the information in m_ath_cl_contexts. If an incompatible
/// XCLBIN is found for a device, the initialization will fail.
/// @return StatusCode::SUCCESS if the programming was successful, or
///         StatusCode::FAILURE otherwise.
StatusCode DeviceMgmtSvc::program_devices() {

  cl_int err = CL_SUCCESS;

  for (AthClContext &ath_cl_context : m_ath_cl_contexts) {

    // Create an OpenCL context for the device(s).
    ath_cl_context.context = std::make_shared<cl::Context>(
        ath_cl_context.devices, nullptr, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
      ATH_MSG_ERROR("Failed to create cl::Context");
      return StatusCode::FAILURE;
    }

    // Load XCLBIN file from disk.
    std::ifstream file;
    try {
      file.open(ath_cl_context.xclbin_info.path.c_str(), std::ios::binary);
    } catch (...) {
      ATH_MSG_ERROR("Could not open " << ath_cl_context.xclbin_info.path
                                      << " for reading");
      return StatusCode::FAILURE;
    }
    std::vector<char> xclbin_buffer((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());

    // Wrap XCLBIN data and size in a vector of cl::Program::Binaries.
    // If we program multiple devices, we need to provide the same
    // binary for each device.
    cl::Program::Binaries binary;
    for (std::size_t i = 0; i < ath_cl_context.devices.size(); ++i) {
      binary.push_back({xclbin_buffer.data(), xclbin_buffer.size()});
    }

    // Create a program from the XCLBIN binary.
    // Effectively loading the XCLBIN on the device(s).
    ath_cl_context.program = std::make_shared<cl::Program>(
        *ath_cl_context.context, ath_cl_context.devices, binary, nullptr, &err);
    if (err != CL_SUCCESS) {
      ATH_MSG_ERROR("Failed to create cl::Program");
      return StatusCode::FAILURE;
    }

    // Report what have been done.
    std::string bdfs = "";
    for (const cl::Device &device : ath_cl_context.devices) {
      bdfs += get_device_bdf(device);
      bdfs += " ";
    }
    ATH_MSG_INFO("Loaded " << ath_cl_context.xclbin_info.path << " on "
                           << ath_cl_context.devices.size() << " "
                           << get_device_name(ath_cl_context.devices[0])
                           << " device(s): " << bdfs);
  }

  return StatusCode::SUCCESS;

}  // DeviceMgmtSvc::program_devices

/// @brief Initialize the service.
/// This function will inspect the available devices and XCLBIN files, pair
/// them and create contexts based on the information gathered. It will then
/// program the devices with the XCLBIN files and create contexts.
/// @return StatusCode::SUCCESS if the initialization was successful, or
///         StatusCode::FAILURE otherwise.
StatusCode DeviceMgmtSvc::initialize() {

  SystemInfo sys_info;

  // Inspect available device(s) and fill sys_info.
  ATH_CHECK(inspect_devices(sys_info));

  // Inspect provided XCLBINs to gather information about kernel(s)
  // into sys_info and a list of XclbinInfo.
  ATH_CHECK(inspect_xclbins(sys_info));

  // Now we can make a decision about the pairing of device(s)
  // and xclbin(s), and the number of required context(s) by
  // filling m_ath_cl_contexts.
  ATH_CHECK(pair_devices_and_xclbins(sys_info));

  // Program the devices with the XCLBINs and create contexts,
  // and programs based on m_ath_cl_contexts.
  ATH_CHECK(program_devices());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode DeviceMgmtSvc::finalize() {

  // Finalise the base class.
  ATH_CHECK(Service::finalize());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

const std::vector<std::shared_ptr<xrt::device>>
DeviceMgmtSvc::get_xrt_devices_by_kernel_name(const std::string &name) const {

  std::vector<std::shared_ptr<xrt::device>> devices;

  // Iterate over all contexts and check if the kernel name is in the list.
  // If so, add the device(s) to the list of devices to return.
  for (const AthClContext &ath_cl_context : m_ath_cl_contexts) {
    if (std::find(ath_cl_context.xclbin_info.kernel_names.begin(),
                  ath_cl_context.xclbin_info.kernel_names.end(),
                  name) != ath_cl_context.xclbin_info.kernel_names.end()) {
      for (const cl::Device &device : ath_cl_context.devices) {
        devices.push_back(std::make_shared<xrt::device>(
            xrt::opencl::get_xrt_device(device())));
      }
    }
    else if (std::find(ath_cl_context.xclbin_info.cu_names.begin(),
                       ath_cl_context.xclbin_info.cu_names.end(),
                       name) != ath_cl_context.xclbin_info.cu_names.end()) {
      for (const cl::Device &device : ath_cl_context.devices) {
        devices.push_back(std::make_shared<xrt::device>(
            xrt::opencl::get_xrt_device(device())));
      }
    }
  }

  return devices;
}

const std::vector<IDeviceMgmtSvc::OpenCLHandle>
DeviceMgmtSvc::get_opencl_handles_by_kernel_name(
    const std::string &name) const {

  std::vector<IDeviceMgmtSvc::OpenCLHandle> handles;

  // Iterate over all contexts and check if the kernel name is in the list.
  // If so, add the context and program to the list of handles to return.
  for (const AthClContext &ath_cl_context : m_ath_cl_contexts) {
    if (std::find(ath_cl_context.xclbin_info.kernel_names.begin(),
                  ath_cl_context.xclbin_info.kernel_names.end(),
                  name) != ath_cl_context.xclbin_info.kernel_names.end()) {
      IDeviceMgmtSvc::OpenCLHandle handle = {ath_cl_context.context,
                                             ath_cl_context.program};
      handles.push_back(handle);
    }
  }

  return handles;
}

}  // namespace AthXRT
