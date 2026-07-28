# Installing a Triton r23.04 Development SDK on AlmaLinux 9

This guide installs the Triton **development libraries, headers, and CMake packages** needed to compile a custom Triton backend on an AlmaLinux 9 machine.

It is intended for the following deployment model:

- Triton `common`, `core`, and `backend` are built and installed once under `/opt`.
- The Triton source repositories are **not** committed into Athena.
- A standalone backend build, or a future Athena package, consumes the installed Triton SDK with `find_package()`.
- Athena supplies traccc, ACTS, vecmem, detray, CUDA, and their matching LCG dependencies.

The procedure is written for:

- AlmaLinux 9
- Triton release `r23.04`
- GCC 14
- CUDA 13
- CMake 4.x

The workarounds for newer compilers and CUDA versions are explained in the troubleshooting section.

---

## 1. Resulting installation layout

The installation will use:

```text
/opt/triton-sdk/r23.04/
├── include/
│   ├── rapidjson/
│   └── triton/
├── lib/ or lib64/
│   ├── cmake/
│   │   ├── RapidJSON/
│   │   ├── TritonBackend/
│   │   ├── TritonCommon/
│   │   └── TritonCore/
│   ├── libtritonbackendutils.a
│   └── stubs/
│       └── libtritonserver.so
└── src/
    ├── backend/
    ├── common/
    └── core/
```

The exact library directory may be `lib` or `lib64`, depending on how CMake and the platform install the packages.

---

## 2. Prerequisites

Setup athena with 
```bash
asetup Athena,main,latest
```

If the machine accesses GitHub through a proxy, export the proxy before cloning or configuring:

```bash
export HTTPS_PROXY=http://proxy.example.org:3128
export HTTP_PROXY="$HTTPS_PROXY"
export https_proxy="$HTTPS_PROXY"
export http_proxy="$HTTP_PROXY"
```

Replace the example proxy with the correct local value.

---

## 3. Define installation variables

Use variables throughout the installation so the release or prefix can be changed easily:

```bash
export TRITON_RELEASE=r23.04
export TRITON_PREFIX=/opt/triton-sdk/${TRITON_RELEASE}
export TRITON_SRC_ROOT=${TRITON_PREFIX}/src
export TRITON_BUILD_ROOT=/tmp/triton-${TRITON_RELEASE}-sdk-build
export RAPIDJSON_SRC=/tmp/rapidjson
export RAPIDJSON_BUILD=/tmp/rapidjson-build
```

Create the directories:

```bash
sudo mkdir -p "${TRITON_SRC_ROOT}"
sudo chown -R "$(id -un):$(id -gn)" /opt/triton-sdk
```

For a shared machine, ownership and permissions should instead be managed according to the site's software-installation policy.

---

## 4. Clone matching Triton source repositories

All Triton repositories must use the same release tag.

```bash
cd "${TRITON_SRC_ROOT}"

git clone --branch "${TRITON_RELEASE}" --depth 1 \
  https://github.com/triton-inference-server/common.git

git clone --branch "${TRITON_RELEASE}" --depth 1 \
  https://github.com/triton-inference-server/core.git

git clone --branch "${TRITON_RELEASE}" --depth 1 \
  https://github.com/triton-inference-server/backend.git
```

Verify the checked-out revisions:

```bash
for repo in common core backend; do
  echo "=== ${repo} ==="
  git -C "${TRITON_SRC_ROOT}/${repo}" describe --tags --always --dirty
  git -C "${TRITON_SRC_ROOT}/${repo}" status --short --branch
  echo
done
```

Each repository should report `r23.04`, with no local changes.

Do not mix `r23.04` with `main` or a different Triton release.

---

## 5. Install a GCC-14-compatible RapidJSON copy

The AlmaLinux 9 `rapidjson-devel` package contains RapidJSON 1.1.0. Its `GenericStringRef` assignment operator fails with newer compilers because it assigns to a const data member.

Install a private RapidJSON copy under the Triton SDK prefix rather than modifying `/usr/include`.

### 5.1 Clone RapidJSON

```bash
rm -rf "${RAPIDJSON_SRC}" "${RAPIDJSON_BUILD}"

git clone https://github.com/Tencent/rapidjson.git "${RAPIDJSON_SRC}"
```

Record the exact commit used:

```bash
git -C "${RAPIDJSON_SRC}" rev-parse HEAD | \
  tee "${TRITON_PREFIX}/RAPIDJSON_COMMIT"
```

For a production deployment, replace the moving default branch with a site-approved pinned commit after validating it once.

### 5.2 Verify that the incompatible line is absent

```bash
if grep -q 'length = rhs.length' \
  "${RAPIDJSON_SRC}/include/rapidjson/document.h"; then
  echo "ERROR: the selected RapidJSON revision contains the incompatible assignment operator"
  exit 1
fi
```

### 5.3 Configure and install RapidJSON

CMake 4.x no longer enables compatibility modes older than CMake 3.5. RapidJSON's older CMake project therefore needs `CMAKE_POLICY_VERSION_MINIMUM`.

```bash
cmake \
  -S "${RAPIDJSON_SRC}" \
  -B "${RAPIDJSON_BUILD}" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${TRITON_PREFIX}" \
  -DRAPIDJSON_BUILD_DOC=OFF \
  -DRAPIDJSON_BUILD_EXAMPLES=OFF \
  -DRAPIDJSON_BUILD_TESTS=OFF
```

Build and install:

```bash
cmake --build "${RAPIDJSON_BUILD}"
sudo cmake --install "${RAPIDJSON_BUILD}"
```

Restore ownership if the install command created root-owned files in a user-managed prefix:

```bash
sudo chown -R "$(id -un):$(id -gn)" "${TRITON_PREFIX}"
```

### 5.4 Verify the installed RapidJSON files

```bash
find "${TRITON_PREFIX}" \
  \( -name RapidJSONConfig.cmake \
  -o -path '*/include/rapidjson/document.h' \) \
  -print
```

Verify the installed header is not the incompatible copy:

```bash
if grep -q 'length = rhs.length' \
  "${TRITON_PREFIX}/include/rapidjson/document.h"; then
  echo "ERROR: incompatible RapidJSON header installed under ${TRITON_PREFIX}"
  exit 1
fi
```

Find the installed CMake package directory:

```bash
export RapidJSON_DIR="$(dirname "$(find "${TRITON_PREFIX}" \
  -name RapidJSONConfig.cmake -print -quit)")"

echo "RapidJSON_DIR=${RapidJSON_DIR}"
test -f "${RapidJSON_DIR}/RapidJSONConfig.cmake"
```

Stop here if `RapidJSON_DIR` is empty.

---

## 6. Configure the Triton development SDK

Start from a clean build directory:

```bash
rm -rf "${TRITON_BUILD_ROOT}"
```

Configure the Triton `backend` project. It brings in the matching `common` and `core` projects. The `FETCHCONTENT_SOURCE_DIR_*` options force it to use the local clones instead of downloading duplicate copies.

```bash
cmake \
  -S "${TRITON_SRC_ROOT}/backend" \
  -B "${TRITON_BUILD_ROOT}" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${TRITON_PREFIX}" \
  -DCMAKE_INSTALL_LIBDIR=lib64 \
  -DCMAKE_CXX_FLAGS="-I${TRITON_PREFIX}/include" \
  -DTRITON_COMMON_REPO_TAG="${TRITON_RELEASE}" \
  -DTRITON_CORE_REPO_TAG="${TRITON_RELEASE}" \
  -DTRITON_THIRD_PARTY_REPO_TAG="${TRITON_RELEASE}" \
  -DFETCHCONTENT_SOURCE_DIR_REPO_COMMON="${TRITON_SRC_ROOT}/common" \
  -DFETCHCONTENT_SOURCE_DIR_REPO_CORE="${TRITON_SRC_ROOT}/core" \
  -DTRITON_ENABLE_GPU=OFF \
  -DTRITON_ENABLE_STATS=ON \
  -DBUILD_TESTING=OFF \
  -DTRITON_ENABLE_TESTS=OFF \
  -DRapidJSON_DIR="${RapidJSON_DIR}" \
  -DRAPIDJSON_INCLUDE_DIRS="${TRITON_PREFIX}/include" \
  -DRapidJSON_INCLUDE_DIRS="${TRITON_PREFIX}/include"
```

### Why `TRITON_ENABLE_GPU=OFF`?

This SDK is being built to provide the Triton backend API, utility library, headers, CMake packages, and `libtritonserver` stub.

With Triton r23.04 and CUDA 13, enabling Triton's internal GPU build can request the obsolete `compute_53` target, which CUDA 13 rejects. Disabling Triton's internal GPU support avoids that old architecture setting.

This does **not** prevent the custom backend from using CUDA through traccc, vecmem, or its own CUDA code. Those dependencies are linked separately when the backend is compiled in the Athena environment.

### Why is `CMAKE_CXX_FLAGS` used here?

RapidJSON is header-only. On the tested AlmaLinux 9 setup, Triton still selected `/usr/include/rapidjson` even when `RapidJSON_DIR` referred to the private installation. Adding the SDK include directory explicitly ensures the compatible header is searched first.

This flag is applied only while constructing the standalone Triton SDK. It should not be copied into the Athena-side backend project.

---

## 7. Validate the configuration before building

Check the selected RapidJSON package:

```bash
grep -E 'RapidJSON_DIR|RAPIDJSON_INCLUDE' \
  "${TRITON_BUILD_ROOT}/CMakeCache.txt" || true
```

Run a single-job verbose build initially:

```bash
cmake --build "${TRITON_BUILD_ROOT}" \
  --verbose \
  -j1 2>&1 | tee /tmp/triton-sdk-build.log
```

If this succeeds, the SDK is built. If it fails, inspect the actual compile command:

```bash
grep -B2 -A2 'backend_common.cc' /tmp/triton-sdk-build.log
```

The command should contain:

```text
-I/opt/triton-sdk/r23.04/include
```

An error mentioning `/usr/include/rapidjson/document.h` means the system RapidJSON header is still being selected.

When the single-job build succeeds, future rebuilds can use all available cores:

```bash
cmake --build "${TRITON_BUILD_ROOT}" -j"$(nproc)"
```

Do not run the install step after a failed build.

---

## 8. Install the Triton SDK

Only after the build completes successfully:

```bash
sudo cmake --install "${TRITON_BUILD_ROOT}"
```

For a user-managed installation prefix:

```bash
sudo chown -R "$(id -un):$(id -gn)" "${TRITON_PREFIX}"
```

---

## 9. Verify the completed SDK

Find the installed CMake packages and libraries:

```bash
find "${TRITON_PREFIX}" \
  \( -name 'TritonCoreConfig.cmake' \
  -o -name 'TritonBackendConfig.cmake' \
  -o -name 'TritonCommonConfig.cmake' \
  -o -name 'libtritonbackendutils*' \
  -o -name 'libtritonserver*' \) \
  -print
```

Check installed headers:

```bash
find "${TRITON_PREFIX}/include/triton" \
  -maxdepth 4 \
  -type f \
  -print | sort | head -50
```

At minimum, confirm these files exist:

```bash
test -f "${TRITON_PREFIX}/include/triton/core/tritonbackend.h"
test -f "${TRITON_PREFIX}/include/triton/core/tritonserver.h"
test -f "${TRITON_PREFIX}/include/triton/backend/backend_common.h"
```

Locate the package directories:

```bash
find "${TRITON_PREFIX}" -type f \
  \( -name TritonCommonConfig.cmake \
  -o -name TritonCoreConfig.cmake \
  -o -name TritonBackendConfig.cmake \) \
  -print
```

A successful installation should provide all three packages.

---

## 10. Record an installation manifest

Create a small manifest so the installation can be reproduced later:

```bash
{
  echo "TRITON_RELEASE=${TRITON_RELEASE}"
  echo "TRITON_PREFIX=${TRITON_PREFIX}"
  echo "INSTALL_DATE=$(date --iso-8601=seconds)"
  echo "HOST=$(hostname -f 2>/dev/null || hostname)"
  echo "OS=$(grep PRETTY_NAME /etc/os-release | cut -d= -f2-)"
  echo "GCC=$(gcc --version | head -1)"
  echo "CMAKE=$(cmake --version | head -1)"
  echo "CUDA=$(nvcc --version 2>/dev/null | tail -1 || echo unavailable)"
  echo "RAPIDJSON_COMMIT=$(git -C "${RAPIDJSON_SRC}" rev-parse HEAD)"
  for repo in common core backend; do
    echo "TRITON_${repo^^}_COMMIT=$(git -C "${TRITON_SRC_ROOT}/${repo}" rev-parse HEAD)"
  done
} | tee "${TRITON_PREFIX}/INSTALL_MANIFEST.txt"
```

Review it:

```bash
cat "${TRITON_PREFIX}/INSTALL_MANIFEST.txt"
```

Commit hashes are more precise than release labels and make future debugging easier.

---

## 11. Optional environment setup script

Create a setup script for consumers of the SDK:

```bash
cat > "${TRITON_PREFIX}/setup.sh" <<EOF_SETUP
export TRITON_ROOT="${TRITON_PREFIX}"
export CMAKE_PREFIX_PATH="${TRITON_PREFIX}:\${CMAKE_PREFIX_PATH:-}"
export LD_LIBRARY_PATH="${TRITON_PREFIX}/lib64:${TRITON_PREFIX}/lib:\${LD_LIBRARY_PATH:-}"
EOF_SETUP
```

Use it with:

```bash
source /opt/triton-sdk/r23.04/setup.sh
```

The Athena backend can also pass `TRITON_ROOT` explicitly to CMake instead of sourcing this script.

---

## 12. Consuming the SDK from a standalone backend build

In the custom backend's `CMakeLists.txt`, use an installed SDK rather than `add_subdirectory()` on Triton source trees:

```cmake
set(
  TRITON_ROOT
  "/opt/triton-sdk/r23.04"
  CACHE PATH
  "Installed Triton SDK prefix"
)

list(PREPEND CMAKE_PREFIX_PATH "${TRITON_ROOT}")

find_package(TritonCommon CONFIG REQUIRED)
find_package(TritonCore CONFIG REQUIRED)
find_package(TritonBackend CONFIG REQUIRED)
```

Link the imported targets:

```cmake
target_link_libraries(
  triton-traccc-backend
  PRIVATE
    TritonCore::triton-core-serverapi
    TritonCore::triton-core-backendapi
    TritonCore::triton-core-serverstub
    TritonBackend::triton-backend-utils
)
```


---

## 13. Building the custom backend in an Athena environment

Start a clean shell and initialize Athena:

```bash
asetup Athena,main,latest
```

Then configure the backend:

```bash
cmake \
  -S . \
  -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install" \
  -DTRITON_ROOT=/opt/triton-sdk/r23.04
```

Build and install:

```bash
cmake --build build -j"$(nproc)"
cmake --install build
```

The backend project should obtain traccc, ACTS, vecmem, detray, CUDA, and their transitive dependencies from the configured Athena environment. It should obtain only the Triton API and backend utility components from `TRITON_ROOT`.

When this becomes a proper Athena package, the same installed Triton SDK can be exposed as an Athena external rather than vendoring the Triton source repositories.

---

## 14. Troubleshooting

### 14.1 CMake appears to be stuck during configuration

Check for active child processes:

```bash
ps -ef --forest | grep -E 'cmake|git|curl|wget' | grep -v grep
```

Inspect the build directory while CMake runs:

```bash
watch -n 2 "du -sh ${TRITON_BUILD_ROOT}; find ${TRITON_BUILD_ROOT}/_deps -maxdepth 2 -type d 2>/dev/null"
```

Test GitHub access:

```bash
git ls-remote \
  https://github.com/triton-inference-server/third_party.git \
  "${TRITON_RELEASE}"
```

If that command hangs, configure the site's HTTP/HTTPS proxy.

### 14.2 Duplicate CMake targets

Errors such as:

```text
add_library cannot create target "triton-core-serverapi"
because another target with the same name already exists
```

mean that `common` or `core` were added twice.

When building the SDK, configure only the Triton `backend` repository and provide local `common` and `core` paths through:

```text
FETCHCONTENT_SOURCE_DIR_REPO_COMMON
FETCHCONTENT_SOURCE_DIR_REPO_CORE
```

Do not separately call `add_subdirectory()` for `common` and `core`.

### 14.3 RapidJSON package not found

Confirm the package config exists:

```bash
find "${TRITON_PREFIX}" -name RapidJSONConfig.cmake -print
```

Then set:

```bash
export RapidJSON_DIR=/path/containing/RapidJSONConfig.cmake
```

The variable must identify the directory, not the config file itself.

### 14.4 `assignment of read-only member ... length`

If the error references:

```text
/usr/include/rapidjson/document.h
```

then the system RapidJSON 1.1.0 header is being selected.

Confirm the private header is valid:

```bash
grep -n 'length = rhs.length' \
  "${TRITON_PREFIX}/include/rapidjson/document.h"
```

This command should produce no output.

Then configure Triton with:

```text
-DCMAKE_CXX_FLAGS=-I/opt/triton-sdk/r23.04/include
```

Use a verbose single-job build to confirm the include order.

### 14.5 `compute_53` is rejected by CUDA 13

An error such as:

```text
nvcc fatal: Value 'compute_53' is not defined for option 'gpu-code'
```

comes from an old CUDA architecture setting in the r23.04 build.

For this development SDK, use:

```text
-DTRITON_ENABLE_GPU=OFF
```

The custom backend can still use CUDA through traccc and vecmem.

### 14.6 CMake 4 rejects an old minimum CMake version

Use:

```text
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

when configuring older third-party projects such as RapidJSON or Triton r23.04.

### 14.7 GoogleTest is still built

Some older Triton subprojects may configure tests despite generic test switches. This is usually harmless.

The important rule is: do not run `cmake --install` if the build itself failed. An install failure involving a missing GoogleTest archive is normally a secondary consequence of the failed build.

### 14.8 Athena finds traccc but not Boost, Eigen, or nlohmann_json

That is a separate Athena/LCG environment issue, not a Triton SDK issue.

For a standalone build, initialize the intended environment first:

```bash
asetup Athena,main,latest
```

Avoid manually mixing arbitrary system Boost, Eigen, or JSON packages with an Athena-built traccc/ACTS installation. Their package versions must match the Athena externals stack.

---

## 15. Updating or reinstalling

To rebuild the same release:

```bash
rm -rf "${TRITON_BUILD_ROOT}"
```

Then rerun the configuration, build, and install commands.

To install a different release, use a different prefix:

```bash
export TRITON_RELEASE=rXX.YY
export TRITON_PREFIX=/opt/triton-sdk/${TRITON_RELEASE}
```

Do not overwrite a working `r23.04` installation with another release. Side-by-side prefixes make rollbacks and compatibility testing much easier.

---

## 16. Minimal end-to-end checklist

```text
[ ] Install compiler, Git, CMake, and build tools
[ ] Clone Triton common/core/backend at the same tag
[ ] Install a compatible private RapidJSON under the SDK prefix
[ ] Verify the private RapidJSON header has no invalid const assignment
[ ] Configure only Triton backend, using local common/core source overrides
[ ] Disable Triton's internal GPU support for CUDA 13 compatibility
[ ] Force the private RapidJSON include path during the SDK build
[ ] Build successfully before running the install step
[ ] Verify TritonCommon, TritonCore, and TritonBackend CMake configs
[ ] Record exact commit hashes in an installation manifest
[ ] Consume the SDK through find_package(), not add_subdirectory()
[ ] Initialize Athena before compiling the traccc custom backend
```
