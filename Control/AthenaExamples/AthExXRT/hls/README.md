# HLS Kernel Projects

This folder contains the necessary materials to build the XCLBIN configuration files used in conjunction with the AthExXRT service. By default, it will produce three XCLBIN files:

* `krnl_VectorAdd.xclbin`: Contains two instances of the `krnl_VectorAdd` kernel.
* `krnl_VectorMult.xclbin`: Contains two instances of the `krnl_VectorMult` kernel.
* `krnl_Combined.xclbin`: Contains two instances of the `krnl_VectorAdd` kernel and two instances of the `krnl_VectorMult kernel`.

To build the XCLBIN files, the Vitis and XRT environments need to be set up in the shell. The XCLBIN files can then be produced using:

```make all```

**Note** The default platform used is `xilinx_u250_gen3x16_xdma_4_1_202210_1`. This can be overridden with the `PLATFORM` environnement variable. As an example, the following produces the build for the `VCK5000` board: 

```PLATFORM=xilinx_vck5000_gen4x8_qdma_2_202220_1 make all```

Build artifacts are created under `./build_[PLATFORM]_[TARGET]`, and link to the resulting XCLBIN files are created in the top folder. It is possible to build the kernel for multiple platforms, but the links will only target the last configuration built.

**Note** These examples prioritize simplicity over speed or functionality.