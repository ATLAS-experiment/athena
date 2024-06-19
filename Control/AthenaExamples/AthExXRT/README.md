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

After a successful Athena build, the simplest example from `./test/` folder can be launched with the following command:

```
python3 VectorAdditionOCLExample_test.py
```

If all goes well, the output will contain the following lines:


```
...
AthXRT::DeviceMgmtSvc                                        0    INFO Found a total of 1 AMD FPGA device(s) (1 device type(s))
AthXRT::DeviceMgmtSvc                                        0    INFO Loaded /workdir/athena_xrt/Control/AthenaExamples/AthExXRT/test/../hls/krnl_VectorAdd.xclbin on 1 xilinx_vck5000_gen4x8_qdma_base_2 device(s): 0000:c1:00.1
...
AthenaHiveEventLoopMgr                                 0     0    INFO   ===>>>  start processing event #1, run #1 on slot 0,  0 events processed so far  <<<===
AthExXRT::VectorAddOCLExampleAlg                       0     0    INFO OpenCL vector addition test PASSED!
AthenaHiveEventLoopMgr                                 0     0    INFO   ===>>>  done processing event #1, run #1 on slot 0,  1 events processed so far  <<<===
AthenaHiveEventLoopMgr                                 1     0    INFO   ===>>>  start processing event #2, run #1 on slot 0,  1 events processed so far  <<<===
AthExXRT::VectorAddOCLExampleAlg                       1     0    INFO OpenCL vector addition test PASSED!
AthenaHiveEventLoopMgr                                 1     0    INFO   ===>>>  done processing event #2, run #1 on slot 0,  2 events processed so far  <<<===
...
```
The two lines stating `AthExXRT::VectorAddOCLExampleAlg ... INFO OpenCL vector addition test PASSED!` indicate that a vector addition has been executed on the FPGA and that the result has been verified against the software reference.

A more comprehensive example, which exercises multiple kernels (vector addition and vector multiplication) and both OpenCL and XRT APIs, is also provided. Run the following on a system with two FPGA boards:

```
python3 MultiAPIMultiKernelExample_test.py
```

Should output the following:

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

The `--evtMax` parameter can be used to increase the number of event the example will run for (default is 2), and the `--thread` parameter can be used to increase the number of threads used to process the events (default is 1).

The `-x` parameter can be used to specify a list of XCLBIN files to load on the device(s). When not specifified, the python script will first attmpt to use the provided default path (if the script is called from another python script), or attempt to use the relative path to default XCLBIN file location in the `hls` folder.

As an example, the following command demonstrates how to run the previous example with a XCLBIN containing only the vector addition kernel loaded onto one device and another XCLBIN containing only the vector multiplication kernel loaded onto the other device. This is run with 10 threads on 2000 events:

```
python3 MultiAPIMultiKernelExample_test.py --thread 10 --evtMax 2000 -x ../hls/krnl_VectorAdd.xclbin ../hls/krnl_VectorMult.xclbin
```