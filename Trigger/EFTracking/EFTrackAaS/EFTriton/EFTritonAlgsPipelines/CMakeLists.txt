cmake_minimum_required(VERSION 3.22)

project(EFTritonPipelines VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#
# Dynamic Dependencies Setup
#

# 1. Inherit CMAKE_PREFIX_PATH set by 'asetup' (contains Athena & AthenaExternals)
if(DEFINED ENV{CMAKE_PREFIX_PATH})
  list(APPEND CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
endif()

# 2. Dynamically extract the exact LCG release version from the environment
set(DETECTED_LCG_RELEASE "")

# Check high-level environment variables set by asetup
foreach(ENV_VAR ROOT_INCLUDE_PATH LD_LIBRARY_PATH)
  if(DEFINED ENV{${ENV_VAR}} AND NOT DETECTED_LCG_RELEASE)
    # Extract strings matching /LCG_<version>/ or /sw/lcg/releases/LCG_<version>/
    string(REGEX MATCH "(LCG_[A-Za-z0-9_]+)" MATCHED_LCG "$ENV{${ENV_VAR}}")
    if(MATCHED_LCG)
      set(DETECTED_LCG_RELEASE "${MATCHED_LCG}")
    endif()
  endif()
endforeach()

# Map the extracted release (e.g. LCG_109a_ATLAS_11) to the central SFT view path
if(DETECTED_LCG_RELEASE AND DEFINED ENV{ALRB_SFT_LCG} AND DEFINED ENV{LCG_PLATFORM})
  # First try exact release match in SFT views (e.g. /cvmfs/sft.cern.ch/lcg/views/LCG_109a_ATLAS_11/x86_64-el9-gcc14-opt)
  set(EXACT_VIEW_PATH "$ENV{ALRB_SFT_LCG}/views/${DETECTED_LCG_RELEASE}/$ENV{LCG_PLATFORM}")

  if(EXISTS "${EXACT_VIEW_PATH}")
    list(APPEND CMAKE_PREFIX_PATH "${EXACT_VIEW_PATH}")
  else()
    # Fallback: Glob for matching release series prefix (e.g., LCG_109a_ATLAS_*) if exact version directory isn't mirrored
    string(REGEX REPLACE "([0-9]+)$" "*" LCG_SERIES_PATTERN "${DETECTED_LCG_RELEASE}")
    file(GLOB LCG_VIEW_FALLBACKS "$ENV{ALRB_SFT_LCG}/views/${LCG_SERIES_PATTERN}/$ENV{LCG_PLATFORM}")
    if(LCG_VIEW_FALLBACKS)
      list(GET LCG_VIEW_FALLBACKS 0 FOUND_VIEW)
      list(APPEND CMAKE_PREFIX_PATH "${FOUND_VIEW}")
    endif()
  endif()
endif()

find_package(covfie CONFIG REQUIRED)
find_package(traccc CONFIG REQUIRED)
find_package(CUDAToolkit REQUIRED)

#
# Pipeline headers
#
# Header-only: the GPU pipeline wrappers under EFTritonPipelines/ are consumed both by
# the standalone test driver below and by the Triton backend in EFTritonRunner/.
#
add_library(EFTritonPipelines INTERFACE)

# In-tree consumers use the same name as installed ones. ALIAS targets are not exported,
# so EXPORT_NAME below is what makes install(EXPORT) emit EFTritonPipelines::pipelines
# rather than EFTritonPipelines::EFTritonPipelines.
add_library(
  EFTritonPipelines::pipelines
  ALIAS
  EFTritonPipelines
)

set_target_properties(
  EFTritonPipelines
  PROPERTIES
    EXPORT_NAME pipelines
)

target_include_directories(
  EFTritonPipelines
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_features(
  EFTritonPipelines
  INTERFACE
  cxx_std_20
)

target_compile_definitions(
  EFTritonPipelines
  INTERFACE
  ALGEBRA_PLUGINS_INCLUDE_ARRAY
)

target_link_libraries(
  EFTritonPipelines
  INTERFACE
    # Athena/LCG traccc dependencies
    vecmem::core vecmem::cuda traccc::io
    traccc::core traccc::device_common traccc::cuda traccc::cuda_utils
    detray::io detray::core
    # <cuda_runtime.h> is included directly by the pipeline headers
    CUDA::cudart
)

#
# Standalone test driver
#
add_executable(
  TracccG200Standalone
  src/TracccG200Standalone.cpp
)

target_compile_options(
  TracccG200Standalone
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
      -Wall
      -Wextra
      -Wno-unused-parameter
      -Wno-type-limits
    >
)

target_link_libraries(
  TracccG200Standalone
  PRIVATE
    EFTritonPipelines
)

#
# Installation
#
install(
  TARGETS EFTritonPipelines
  EXPORT EFTritonPipelinesTargets
)

install(
  TARGETS TracccG200Standalone
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(
  DIRECTORY EFTritonPipelines
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING PATTERN "*.hpp"
)

set(
  INSTALL_CONFIGDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/EFTritonPipelines"
)

install(
  EXPORT EFTritonPipelinesTargets
  FILE EFTritonPipelinesTargets.cmake
  NAMESPACE EFTritonPipelines::
  DESTINATION "${INSTALL_CONFIGDIR}"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EFTritonPipelinesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/EFTritonPipelinesConfig.cmake"
  INSTALL_DESTINATION "${INSTALL_CONFIGDIR}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/EFTritonPipelinesConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/EFTritonPipelinesConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EFTritonPipelinesConfigVersion.cmake"
  DESTINATION "${INSTALL_CONFIGDIR}"
)
