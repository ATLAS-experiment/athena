# XRT Example Package

## Introduction

This package includes several examples demonstrating how to use the XRT library and AthXRT services to load and utilize AMD/Xilinx FPGA acceleration kernels directly from Athena through OpenCL and/or XRT native APIs. For detailed information on building and configuring the service, please refer to the AthXRT service README.

## Building the FPGA acceleration kernels

This package provides two examples of FPGA acceleration kernel implementations:

* Vector Addition
* Vector Multiplication

Instruction on how to build the binary configuratino file from these examples can be found in the README found in the ./hls folder. These binary configuration files are required to use the AthExXRT package.

## Run AthExXRT examples

Running the AthExXRT example requires the following conditions:

* An AMD/Xilinx FPGA accelerator device (the current kernels have been tested on: VCK5000, U250, and U50 boards).
* Successful build of the FPGA configuration file produced in the previous step for the matching FPGA board.
* An Athena build environment with the XRT library installed (tested with XRT 2022.2, 2023.1, and 2023.2).

**Note** that XRT-equipped AlmaLinux 9 Docker images can be found [here](https://gitlab.cern.ch/qberthet/atlas-xrt-devel-env).

After a successful Athena build, the examples from the `./python/` folder can be launched with the following command:

```
python -m AthExXRT.KernelExampleConfig FPGAMgmt.HLSDir="<directory/to/xclbinfiles/>"
```
where `FPGA.Mgmgt.HLSDir` points to the directory containing the `krnl_Combined.xclbin` binary file.

If all goes well, the output will contain the following lines:

```
...
AthXRT::DeviceMgmtSvc                                        0    INFO Found a total of 2 AMD FPGA device(s) (1 device type(s))
AthXRT::DeviceMgmtSvc                                        0    INFO Loaded /workdir/athena_xrt/Control/AthenaExamples/AthExXRT/test/../hls/krnl_Combined.xclbin on 2 xilinx_vck5000_gen4x8_qdma_base_2 device(s): 0000:c1:00.1 0000:81:00.1
...

AthenaHiveEventLoopMgr                                 0     0    INFO   ===>>>  start processing event #1, run #1 on slot 0,  0 events processed so far  <<<===
AthExXRT::VectorAddOCLExampleAlg                       0     0    INFO OpenCL vector addition test PASSED!
AthExXRT::VectorMultOCLExampleAlg                      0     0    INFO OpenCL vector multiplication test PASSED!
AthExXRT::VectorMultXRTExampleAlg                      0     0    INFO XRT vector multiplication test PASSED!
AthExXRT::VectorAddXRTExampleAlg                       0     0    INFO XRT vector addition test PASSED!
AthenaHiveEventLoopMgr                                 0     0    INFO   ===>>>  done processing event #1, run #1 on slot 0,  1 events processed so far  <<<===
AthenaHiveEventLoopMgr                                 1     0    INFO   ===>>>  start processing event #2, run #1 on slot 0,  1 events processed so far  <<<===
AthExXRT::VectorAddOCLExampleAlg                       1     0    INFO OpenCL vector addition test PASSED!
AthExXRT::VectorAddXRTExampleAlg                       1     0    INFO XRT vector addition test PASSED!
AthExXRT::VectorMultXRTExampleAlg                      1     0    INFO XRT vector multiplication test PASSED!
AthExXRT::VectorMultOCLExampleAlg                      1     0    INFO OpenCL vector multiplication test PASSED!
AthenaHiveEventLoopMgr                                 1     0    INFO   ===>>>  done processing event #2, run #1 on slot 0,  2 events processed so far  <<<===
...
```

