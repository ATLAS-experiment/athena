# Copyright 2021, NVIDIA CORPORATION & AFFILIATES.
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

project(TracccBackend VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# RPATH Setup: Embed dependency paths (CUDA/Athena) into output binary
#
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(DEFINED ENV{CUDA_TOOLKIT_ROOT})
  list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN" "$ENV{CUDA_TOOLKIT_ROOT}/lib64")
endif()

#
# User-configurable locations
#
set(
  TRITON_ROOT
  "/opt/triton-sdk/r23.04"
  CACHE PATH
  "Installed Triton SDK prefix"
)

#
# Dynamic Dependencies Setup
#

# 1. Start with Triton search path
list(PREPEND CMAKE_PREFIX_PATH "${TRITON_ROOT}")

# 2. Inherit CMAKE_PREFIX_PATH set by 'asetup' (contains Athena & AthenaExternals)
if(DEFINED ENV{CMAKE_PREFIX_PATH})
  list(APPEND CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
endif()

# 3. Align CUDA Toolkit path with active 'asetup' environment
if(DEFINED ENV{CUDA_TOOLKIT_ROOT})
  set(CUDAToolkit_ROOT "$ENV{CUDA_TOOLKIT_ROOT}")
elseif(DEFINED ENV{CUDA_HOME})
  set(CUDAToolkit_ROOT "$ENV{CUDA_HOME}")
endif()

# 4. Dynamically extract the exact LCG release version from the environment
set(DETECTED_LCG_RELEASE "")

# Check high-level environment variables set by asetup
foreach(ENV_VAR ROOT_INCLUDE_PATH LD_LIBRARY_PATH)
  if(DEFINED ENV{${ENV_VAR}} AND NOT DETECTED_LCG_RELEASE)
    string(REGEX MATCH "(LCG_[A-Za-z0-9_]+)" MATCHED_LCG "$ENV{${ENV_VAR}}")
    if(MATCHED_LCG)
      set(DETECTED_LCG_RELEASE "${MATCHED_LCG}")
    endif()
  endif()
endforeach()

# Map the extracted release to central SFT view path
if(DETECTED_LCG_RELEASE AND DEFINED ENV{ALRB_SFT_LCG} AND DEFINED ENV{LCG_PLATFORM})
  set(EXACT_VIEW_PATH "$ENV{ALRB_SFT_LCG}/views/${DETECTED_LCG_RELEASE}/$ENV{LCG_PLATFORM}")
  
  if(EXISTS "${EXACT_VIEW_PATH}")
    list(APPEND CMAKE_PREFIX_PATH "${EXACT_VIEW_PATH}")
  else()
    string(REGEX REPLACE "([0-9]+)$" "*" LCG_SERIES_PATTERN "${DETECTED_LCG_RELEASE}")
    file(GLOB LCG_VIEW_FALLBACKS "$ENV{ALRB_SFT_LCG}/views/${LCG_SERIES_PATTERN}/$ENV{LCG_PLATFORM}")
    if(LCG_VIEW_FALLBACKS)
      list(GET LCG_VIEW_FALLBACKS 0 FOUND_VIEW)
      list(APPEND CMAKE_PREFIX_PATH "${FOUND_VIEW}")
    endif()
  endif()
endif()

find_package(TritonCommon CONFIG REQUIRED)
find_package(TritonCore CONFIG REQUIRED)
find_package(TritonBackend CONFIG REQUIRED)

find_package(covfie CONFIG REQUIRED)
find_package(traccc CONFIG REQUIRED)
find_package(CUDAToolkit REQUIRED)

#
# GPU pipeline wrappers
#
set(
  EFTRITON_PIPELINES_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../EFTritonAlgsPipelines"
  CACHE PATH
  "Directory containing the EFTritonPipelines/ header tree"
)

if(NOT EXISTS "${EFTRITON_PIPELINES_DIR}/EFTritonPipelines/TracccG200Standalone.hpp")
  message(
    FATAL_ERROR
    "Pipeline headers not found under '${EFTRITON_PIPELINES_DIR}'. "
    "Point -DEFTRITON_PIPELINES_DIR at the EFTritonAlgsPipelines directory."
  )
endif()

#
# Backend library
#
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/libtriton_traccc.ldscript"
  "${CMAKE_CURRENT_BINARY_DIR}/libtriton_traccc.ldscript"
  COPYONLY
)

add_library(
  triton-traccc-backend
  SHARED
  src/traccc_g200.cc
)

add_library(
  TracccBackend::triton-traccc-backend
  ALIAS
  triton-traccc-backend
)

target_compile_features(
  triton-traccc-backend
  PRIVATE
  cxx_std_20
)

target_include_directories(
  triton-traccc-backend
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${EFTRITON_PIPELINES_DIR}"
)

target_compile_definitions(
  triton-traccc-backend
  PRIVATE
  ALGEBRA_PLUGINS_INCLUDE_ARRAY
)

target_compile_options(
  triton-traccc-backend
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
      -Wall
      -Wextra
      -Wno-unused-parameter
      -Wno-type-limits
      -Werror
    >
)

target_link_libraries(
  triton-traccc-backend
  PRIVATE
    # Installed Triton SDK
    TritonCore::triton-core-serverapi
    TritonCore::triton-core-backendapi
    TritonCore::triton-core-serverstub
    TritonBackend::triton-backend-utils

    # Athena/LCG traccc stack
    vecmem::core
    vecmem::cuda
    traccc::io
    traccc::core
    traccc::device_common
    traccc::cuda
    traccc::cuda_utils
    detray::io
    detray::core
)

set_target_properties(
  triton-traccc-backend
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME triton_traccc
)

if(UNIX AND NOT APPLE)
  target_link_options(
    triton-traccc-backend
    PRIVATE
    "LINKER:--version-script=${CMAKE_CURRENT_BINARY_DIR}/libtriton_traccc.ldscript"
  )
endif()

#
# Installation
#
set(
  TRITON_MODEL_VERSION_DIR
  "model_repository/traccc_g200/1"
)

install(
  TARGETS triton-traccc-backend
  EXPORT TracccBackendTargets
  LIBRARY DESTINATION "${TRITON_MODEL_VERSION_DIR}"
  RUNTIME DESTINATION "${TRITON_MODEL_VERSION_DIR}"
)

install(
  FILES cfg/config.pbtxt
  DESTINATION model_repository/traccc_g200
)

#
# Runtime environment helper
#
set(ATHENA_LD_LIBRARY_PATH "$ENV{LD_LIBRARY_PATH}")

if(NOT ATHENA_LD_LIBRARY_PATH)
  message(
    WARNING
    "LD_LIBRARY_PATH is empty; the generated setup_env.sh will not be usable. "
    "Configure from a shell where 'asetup' has been sourced."
  )
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/setup_env.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup_env.sh"
  @ONLY
)

install(
  PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/setup_env.sh"
  DESTINATION .
)

set(
  INSTALL_CONFIGDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/TracccBackend"
)

install(
  EXPORT TracccBackendTargets
  FILE TracccBackendTargets.cmake
  NAMESPACE TracccBackend::
  DESTINATION "${INSTALL_CONFIGDIR}"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TracccBackendConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/TracccBackendConfig.cmake"
  INSTALL_DESTINATION "${INSTALL_CONFIGDIR}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/TracccBackendConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TracccBackendConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TracccBackendConfigVersion.cmake"
  DESTINATION "${INSTALL_CONFIGDIR}"
)
