# AthXRT Services

# Introduction

This package provides an Athena service designed for programming AMD/Xilinx FPGA accelerators through the [XRT](https://github.com/Xilinx/XRT) library.

Before deploying an FPGA for any practical application, it must be "programmed." This process involves loading a binary file onto the FPGA. This binary file, referred to as an "xclbin" file in AMD/Xilinx terminology, specifies the configuration of the FPGA hardware resources to create one or more acceleration "kernels." Loading this configuration file incurs a significant time overhead and is typically not performed dynamically.

The AthXRT service serves as a unified entry point for Athena user algorithms utilizing these accelerators. One or more configuration files can be specified via a configuration script through CA. During the Athena launch, the service will evaluate all available accelerators and program them using the rules defined below. Once programmed, algorithms can request a specific kernel name from the service and receive handles to the accelerators providing these kernels.

# General use

A system with an AMD/Xilinx FPGA accelerator and the XRT library installed is required for this library to be functional.

The AthXRT service is instantiated by configuring it within an Athena algorithm set to run. During initialization, the service leverages the underlying XRT library to enumerate compatible AMD/Xilinx FPGA accelerators. A list of configuration files to be loaded onto the accelerators is specified by the user algorithm via the `XclbinPathsList` run flags. A set of rules (defined below) governs how the accelerators and configuration files are matched. Once the configuration files are loaded onto the accelerators, the service initialization is complete.

User algorithms can then request handles to the programmed device(s) by specifying a kernel name. There are two ways to obtain these handles:

**OpenCL API**: For algorithms using the OpenCL API, the `get_opencl_handles_by_kernel_name(const std::string &name)` method returns a vector of `OpenCLHandle` structures containing the OpenCL programs and contexts in which the requested kernel name can be instantiated.

**XRT Native API**: For algorithms using the XRT Native API, the `get_xrt_devices_by_kernel_name(const std::string &name)` method returns a vector of `xrt::device` instances on which the requested kernel name can be instantiated.

The service does not enforce any specific method for using the provided OpenCL contexts or XRT devices: algorithms are free to create and manage kernels, buffers, and queues as they see fit. While it is safe to use both APIs from different algorithms to access the same device and/or kernel, this might not be the most efficient approach.

This service does not cover the building and distribution of FPGA configuration files.

# Device/XCLBIN matching rules

The following rules are applied in sequence to determine how to pair the accelerators and XCLBIN files:

* If only one accelerator is present, the service expects at most one XCLBIN file to be provided by the user, which will be loaded onto the device (most common use case).

* If multiple accelerators of the same model are present and only one XCLBIN file is provided, all the accelerators will be loaded with the same XCLBIN file.

* If multiple accelerators of the same model are present and multiple XCLBIN files are provided, each XCLBIN file is loaded onto one accelerator. If the same XCLBIN file is provided multiple times, it will be loaded onto multiple devices. If there are fewer XCLBIN files than accelerators, some accelerators will remain unprogrammed.

* If multiple accelerators of different models are present (the least probable case, but it has been encountered in some test setups), the service will attempt to detect which model each XCLBIN file is built for and pair the accelerator and XCLBIN file accordingly.

# Build & run environement

The XRT library environment must be set up prior to compiling this package. CMake tests for and uses the `XILINX_XRT` variable defined during the XRT environment setup. If this variable is not defined, the package build will be skipped. As XRT is currently not distributed by Atlas software, users can either manually install a native Athena/XRT environment or use Docker images on an Athena-incompatible system equipped with an accelerator.

# Docker

Docker images with the Athena build environment and XRT versions 2022.2, 2023.1 and 2023.1 (based on AlmaLinux 9) are available from this repository:

* https://gitlab.cern.ch/qberthet/atlas-xrt-devel-env

Branches are used to manage the various XRT versions (since the XRT library needs to match the host XOCL/ZOCL kernel driver). Other XRT versions can be added if needed.

These Docker images can be used to build Athena with the AthXRT package or to run Athena on an FPGA-equipped host running an Athena-incompatible OS.
