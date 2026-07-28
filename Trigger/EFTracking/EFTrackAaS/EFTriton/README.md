# `traccc`-as-a-Service (`traccc-aaS`) with NVIDIA Triton

Welcome to the EF Tracking implementation of traccc as-a-Service with NVIDIA Triton Inference Server. This document will demonstrate how to build, test, and run the server, as well as how to use it with our Python client. An Athena client edition will be updated.

This repository runs traccc as-a-Service. This uses a custom backend, with a wrapper for GPU pipeline information, to launch the Triton server. This Triton server is launched with a model algorithm to transfer information between the client and the GPU. Currently, the model is built for the G200 pipeline, with more models to be included at a later date.

---

## 1. Prerequisites & Environment Setup

Select one of the two deployment workflows below depending on your machine environment.

### Option A: Interactive Cluster Node (`ef-tb-g01`)

Use this option when building directly on an ATLAS cluster node against the CVMFS Athena nightlies and an installed Triton SDK.

1. **Source the Athena Environment**:
   ```bash
   asetup Athena,main,latest
   ```

2. **Verify Toolchain & Triton SDK**:
   Ensure Triton SDK `r23.04` is available under `/opt/triton-sdk/r23.04`.
   ```bash
   test -f /opt/triton-sdk/r23.04/include/triton/core/tritonbackend.h && echo "Triton SDK OK"
   ```

### Option B: Container Environment (Apptainer / Docker)

Use this option if you are deploying inside an isolated container image.

1. **Obtain or Build the Image**:
   A pre-built container image is available on EOS:
   ```text
   /eos/project/a/atlas-eftracking/AaS/traccc-aas_v1p4_report.sif
   ```

   To build a custom `.sif` image locally using Docker:
   ```bash
   # Build Docker image
   sudo docker build -t traccc-aas:latest .

   # Convert to Apptainer image (use a custom temp dir if disk space is low)
   mkdir -p /tmp/apptainer-tmp
   sudo APPTAINER_TMPDIR=/tmp/apptainer-tmp \
        APPTAINER_CACHEDIR=/tmp/apptainer-tmp \
        apptainer build traccc-aas_v1p4_report.sif docker-daemon://traccc-aas:latest
   ```

2. **Run the Container**:
   Launch the container with GPU access (`--nv`) and bind your workspace and geometry data:
   ```bash
   apptainer run --nv \
     --bind "$(pwd):/work" \
     --bind /eos/project/a/atlas-eftracking/GPU/ITk_data/ATLAS-P2-RUN4-03-00-01:/geoDir \
     traccc-aas_v1p4_report.sif
   ```

---

## 2. Optional: Build and Test Standalone Wrappers

The standalone pipeline wrappers feed GPU algorithms into the backend. They live under `EFTritonAlgsPipelines/`.

To test the G200 standalone executable independently with a sample event:

```bash
cd EFTritonAlgsPipelines
mkdir -p build && cd build

# Unset compilers if running inside container with conflicting environment overrides
unset CC CXX

cmake ..
cmake --build . -j"$(nproc)"

./TracccG200Standalone ../../EFTritonTester/event000000000-cells.csv 0
```

---

## 3. Build the Triton Backend (`EFTritonRunner`)

The backend source code (`src/traccc_g200.cc`) wraps the GPU pipeline and links against `traccc`, `covfie`, `vecmem`, `detray`, and the Triton Server SDK.

Navigate to `EFTritonRunner/G200` to compile and install the model and library:

```bash
cd EFTritonRunner/G200

# Clear stale build cache
rm -rf build install

# Configure using environment variables set up by Athena
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install" \
  -DTRITON_ROOT=/opt/triton-sdk/r23.04

# Compile and Install
cmake --build build --target install -j"$(nproc)"
```

Compilation places the required `libtriton_traccc.so` library and the model configuration (`config.pbtxt`) into `$PWD/install/model_repository`.

---

## 4. Start Triton Server

Once compiled, launch the Triton server and pass the path to the newly installed model repository:

```bash
tritonserver \
  --model-repository="$PWD/install/model_repository" \
  --load-model=traccc-g200 \
  --log-verbose=1
```

> **Note:** Add `--model-control-mode=explicit` if you intend to dynamically load or unload multiple models.

---

## 5. Running the Client

### Multi-Node Setup (SSH Tunneling)
If the server is running on worker node `ef-tb-g01` and your client is on a different node, open a terminal on the client node and set up an SSH tunnel:

```bash
ssh -L 8001:localhost:8001 $USER@ef-tb-g01
```

### Run Client
Navigate to `EFTritonTester/` and run the Python client script:

```bash
cd EFTritonTester
python TracccTritonClient.py
```

---

## References & Additional Links

- **C++ Standalone Triton Client:** [traccc-aaS GitHub Client Guide](#)
- **Athena Tracking-as-a-Service Guide:** [CERN CodiMD Notes](#)
