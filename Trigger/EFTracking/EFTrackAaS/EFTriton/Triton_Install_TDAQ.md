# Building & Installing Triton r25.11 on AlmaLinux 9 (CUDA 13 / GCC 14 / CMake 4)

This document provides step-by-step instructions for:
1. **Part A:** Building and installing the Triton GPU Development SDK (`/opt/triton-sdk/r25.11`).
2. **Part B:** Building and running the complete bare-metal Triton Inference Server binary locally.

---

# PART A: Installing Triton SDK r25.11

## A1. Environment & Variable Setup

setupATLAS
asetup Athena,latest,miain

export TRITON_RELEASE=r25.11
export TRITON_PREFIX=/opt/triton-sdk/${TRITON_RELEASE}
export TRITON_SRC_ROOT=${TRITON_PREFIX}/src
export TRITON_BUILD_ROOT=/tmp/triton-${TRITON_RELEASE}-sdk-build
export RAPIDJSON_SRC=/tmp/rapidjson
export RAPIDJSON_BUILD=/tmp/rapidjson-build

sudo mkdir -p "${TRITON_SRC_ROOT}"
sudo chown -R "$(id -un):$(id -gn)" /opt/triton-sdk

export HTTP_PROXY=http://np04-web-proxy.cern.ch:3128
export HTTPS_PROXY=http://np04-web-proxy.cern.ch:3128
export NO_PROXY=".cern.ch"
export http_proxy=http://np04-web-proxy.cern.ch:3128
export https_proxy=http://np04-web-proxy.cern.ch:3128
export no_proxy=".cern.ch"


---

## A2. Clone Dependencies & Patch Source Code

### A2.1 Clone Triton Repositories

cd "${TRITON_SRC_ROOT}"
git clone --branch "${TRITON_RELEASE}" --depth 1 https://github.com/triton-inference-server/common.git
git clone --branch "${TRITON_RELEASE}" --depth 1 https://github.com/triton-inference-server/core.git
git clone --branch "${TRITON_RELEASE}" --depth 1 https://github.com/triton-inference-server/backend.git

---

## A3. Install GCC-14 Compatible RapidJSON

System RapidJSON (v1.1.0) fails under GCC 14 due to a const-assignment bug in GenericStringRef. Install upstream RapidJSON under the SDK prefix:

rm -rf "${RAPIDJSON_SRC}" "${RAPIDJSON_BUILD}"
git clone https://github.com/Tencent/rapidjson.git "${RAPIDJSON_SRC}"

--- a/include/rapidjson/document.h
+++ b/include/rapidjson/document.h
@@ -316,7 +316,7 @@ struct GenericStringRef {

     GenericStringRef(const GenericStringRef& rhs) : s(rhs.s), length(rhs.length) {}

-    GenericStringRef& operator=(const GenericStringRef& rhs) { s = rhs.s; length = rhs.length; }
+    GenericStringRef& operator=(const GenericStringRef& rhs) { s = rhs.s; const_cast<SizeType&>(length) = rhs.length; return *this; }

     //! implicit conversion to const Ch pointer
     operator const Ch*() const { return s; }

cmake \
  -S "${RAPIDJSON_SRC}" \
  -B "${RAPIDJSON_BUILD}" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${TRITON_PREFIX}" \
  -DRAPIDJSON_BUILD_DOC=OFF \
  -DRAPIDJSON_BUILD_EXAMPLES=OFF \
  -DRAPIDJSON_BUILD_TESTS=OFF

cmake --build "${RAPIDJSON_BUILD}"
sudo cmake --install "${RAPIDJSON_BUILD}"

export RapidJSON_DIR="${TRITON_PREFIX}/lib/cmake/RapidJSON"
---

## A4. Build and Install Triton SDK

rm -rf "${TRITON_BUILD_ROOT}"

cmake \
  -S "${TRITON_SRC_ROOT}/backend" \
  -B "${TRITON_BUILD_ROOT}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${TRITON_PREFIX}" \
  -DCMAKE_INSTALL_LIBDIR=lib64 \
  -DCMAKE_CXX_FLAGS="-I${TRITON_PREFIX}/include" \
  -DTRITON_COMMON_REPO_TAG="${TRITON_RELEASE}" \
  -DTRITON_CORE_REPO_TAG="${TRITON_RELEASE}" \
  -DTRITON_THIRD_PARTY_REPO_TAG="${TRITON_RELEASE}" \
  -DFETCHCONTENT_SOURCE_DIR_REPO_COMMON="${TRITON_SRC_ROOT}/common" \
  -DFETCHCONTENT_SOURCE_DIR_REPO_CORE="${TRITON_SRC_ROOT}/core" \
  -DTRITON_ENABLE_GPU=ON \
  -DTRITON_ENABLE_STATS=ON \
  -DBUILD_TESTING=OFF \
  -DTRITON_ENABLE_TESTS=OFF \
  -DRapidJSON_DIR="${RapidJSON_DIR}" \
  -DRAPIDJSON_INCLUDE_DIRS="${TRITON_PREFIX}/include"

cmake --build "${TRITON_BUILD_ROOT}" -j"$(nproc)"
sudo cmake --install "${TRITON_BUILD_ROOT}"

---

## A5. Setup Environment & Verification

### A5.1 Create Setup Script

cat << 'EOF' | sudo tee "${TRITON_PREFIX}/setup.sh"
export TRITON_ROOT="/opt/triton-sdk/r25.11"
export CMAKE_PREFIX_PATH="${TRITON_ROOT}:${CMAKE_PREFIX_PATH:-}"
export LD_LIBRARY_PATH="${TRITON_ROOT}/lib64:${TRITON_ROOT}/lib:${LD_LIBRARY_PATH:-}"
EOF

### A5.2 Validate SDK Package

test -f "${TRITON_PREFIX}/include/triton/core/tritonbackend.h" && \
test -f "${TRITON_PREFIX}/include/triton/core/tritonserver.h" && \
find "${TRITON_PREFIX}" -name "TritonBackendConfig.cmake" | grep -q . && \
echo "SUCCESS: Triton GPU SDK installed successfully at ${TRITON_PREFIX}"

---
---
# PART B: Building & Running Full Triton Server Locally (Bare-Metal)

## B1. Clone Server Orchestrator

export SERVER_WORK_DIR=/tmp/triton-server-r25.11
git clone --branch r25.11 https://github.com/triton-inference-server/server.git "${SERVER_WORK_DIR}"
cd "${SERVER_WORK_DIR}"

---

## B2. Set Global CMake Policy Fallback

export CMAKE_POLICY_VERSION_MINIMUM=3.5

---

## B3. Initial Build Run (Fetches Third-Party Source Dependencies)

python3 build.py \
  --no-container-build \
  --enable-gpu \
  --backend=python \
  --build-dir=$(pwd)/build_output 

---

## B6. Run Native Triton Server

export LD_LIBRARY_PATH=$(pwd)/build_output/opt/tritonserver/lib:${LD_LIBRARY_PATH}

./build_output/opt/tritonserver/bin/tritonserver --model-repository=/path/to/model_repository