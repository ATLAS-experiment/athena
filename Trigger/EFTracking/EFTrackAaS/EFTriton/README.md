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

## 2. Build the Pipeline Wrappers (`EFTritonAlgsPipelines`)

The pipeline wrappers feed GPU algorithms into the backend. They live under `EFTritonAlgsPipelines/`, and are header-only: they resolve `traccc`, `detray`, `vecmem`, `covfie` and CUDA from the Athena environment sourced in step 1.

This step is **optional** for building the backend. Because the wrappers are header-only,
`EFTritonRunner` reads them straight out of this source tree; it does not need them
installed. Build here when you want the `TracccG200Standalone` test driver below.

```bash
cd EFTritonAlgsPipelines

# Unset compilers if running inside container with conflicting environment overrides
unset CC CXX

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install"

cmake --build build --target install -j"$(nproc)"
```

This installs the headers under `install/include/EFTritonPipelines/` and the CMake package
under `install/lib/cmake/EFTritonPipelines/`.

To test the G200 standalone executable independently with a sample event:

```bash
./build/TracccG200Standalone ../EFTritonTester/event000000000-cells.csv 0
```

---

## 3. Build the Triton Backend (`EFTritonRunner`)

The backend source code (`src/traccc_g200.cc`) wraps the GPU pipeline and links against the Triton Server SDK plus the `traccc`, `covfie`, `vecmem`, `detray` and CUDA libraries resolved from the Athena environment. The pipeline headers are picked up from `../../EFTritonAlgsPipelines`; override with `-DEFTRITON_PIPELINES_DIR=<dir>` if your checkout differs.

Navigate to `EFTritonRunner/G200` to compile and install the model and library:

```bash
cd EFTritonRunner/G200

# Clear stale build cache
rm -rf build install

# Configure using environment variables set up by Athena.
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install" \
  -DTRITON_ROOT=/opt/triton-sdk/r25.11 \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_CXX_EXTENSIONS=OFF

# Compile and Install
cmake --build build --target install -j"$(nproc)"
```

Compilation produces a self-contained model repository under `$PWD/install/model_repository`:

```
install/
├── setup_env.sh                 # replays the build-time LD_LIBRARY_PATH
└── model_repository/
    └── traccc_g200/             # model name, as passed to the client
        ├── config.pbtxt
        └── 1/                   # version directory; Triton needs at least one
            └── libtriton_traccc.so
```

Two naming rules are load-bearing here:

- The library basename must match `backend: "traccc"` in `config.pbtxt`, i.e.
  `libtriton_<backend>.so`. The *model* name (`traccc_g200`) is independent of the
  *backend* name (`traccc`).
- Triton searches for the backend library in the model version directory first, then
  the model directory, then the global backend directory. Installing it into `1/`
  uses the first of those, so no `--backend-directory` flag is required.

---

## 4. Start Triton Server

Once compiled, launch the Triton server and pass the path to the newly installed model repository:

```bash
apptainer run --nv   --bind "${PWD}:/work" --bind /cvmfs:/cvmfs  --bind /eos/project/a/atlas-eftracking/GPU/ITk_data/ATLAS-P2-RUN4-03-00-01:/geoDir   /scratch/large/cahinder/tritontracccimage.sif

source install/setup_env.sh

tritonserver \
  --model-repository="$PWD/install/model_repository" \
  --log-verbose=1
```

> **Note:** To load or unload models dynamically, add
> `--model-control-mode=explicit --load-model=traccc_g200`. `--load-model` is only
> accepted in explicit mode; passing it on its own makes the server exit.


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
