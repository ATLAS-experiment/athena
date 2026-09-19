# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Project file for building a custom version of Acts against an
# installed ATLAS release/nightly.

set(ATLAS_ACTS_SOURCE_DIR "" CACHE PATH "Optional local Acts source directory")

# The section below will configure an inline build of ACTS with a
# user-configured source directory. The ACTS build artifacts will be
# located inside the build directory of the current project.
if(IS_DIRECTORY ${ATLAS_ACTS_SOURCE_DIR})
  message(STATUS "Using ACTS from: ${ATLAS_ACTS_SOURCE_DIR}")

  include(FetchContent)

  # ACTS build configutation.
  # This should be consistent with the configruation in the externals [1].
  #
  # [1]: https://gitlab.cern.ch/atlas/atlasexternals/-/blob/main/External/Acts/CMakeLists.txt
  set(ACTS_USE_SYSTEM_NLOHMANN_JSON ON CACHE BOOL "Use system json")
  set(ACTS_BUILD_PLUGIN_JSON ON CACHE BOOL "Build json plugin")
  set(ACTS_BUILD_PLUGIN_GEOMODEL ON CACHE BOOL "Build geomodel plugin")
  set(ACTS_BUILD_PLUGIN_ROOT ON CACHE BOOL "Build root plugin")
  set(ACTS_BUILD_FATRAS ON CACHE BOOL "Build ACTS FATRAS")
  set(ACTS_BUILD_PLUGIN_TRACCC ON CACHE BOOL "Build TRACCC plugin")
  set(ACTS_USE_SYSTEM_VECMEM ON CACHE BOOL "Use system vecmem")
  set(DETRAY_SET_LOGGING NONE CACHE STRING "Disable logging in Detray")
  set(TRACCC_SUPPORTED_DETECTORS "itk_detector" CACHE STRING "Supported detectors for TRACCC")
  if(CMAKE_CUDA_COMPILER)
    # Overall options.
    set(ACTS_ENABLE_CUDA ON CACHE BOOL "Enable CUDA support in Acts in general")
    # Make Acts's Findonnxruntime.cmake visible to the Acts build.
    list(PREPEND CMAKE_MODULE_PATH "${ATLAS_ACTS_SOURCE_DIR}/cmake")
    find_package(onnxruntime)
    # Turn on the build of the GNN plugin.
    set(ACTS_BUILD_PLUGIN_GNN ON CACHE BOOL "Build GNN plugin")
    set(ACTS_GNN_ENABLE_ONNX ON CACHE BOOL "Enable ONNX support in the GNN plugin")
    set(ACTS_GNN_ENABLE_TORCH OFF CACHE BOOL "Disable Torch support in the GNN plugin")
    set(ACTS_GNN_ENABLE_TENSORRT OFF CACHE BOOL "Disable TensorRT support in the GNN plugin")
    set(ACTS_GNN_ENABLE_MODULEMAP ON CACHE BOOL "Enable module map usage in the GNN plugin")
    # Turn on the build of the traccc plugin(s).
    set(DETRAY_BUILD_CUDA ON CACHE BOOL "Turn on CUDA support in Detray") # Workaround for EFTRACK-1010
  endif()
  if(CMAKE_HIP_COMPILER)
    # Detray/Traccc option(s).
    set(DETRAY_BUILD_HIP ON CACHE BOOL "Turn on HIP support in Detray")
    set(TRACCC_BUILD_HIP ON CACHE BOOL "Turn on HIP support in Traccc")
    set(TRACCC_SETUP_ROCTHRUST ON CACHE BOOL "Set up ROC Thrust for Traccc")
  endif()

  # Make sure that find_package(traccc) and find_package(detray) calls would
  # not actually look for traccc or detray. Since in this setup both will be
  # provided by this Acts build.
  file(COPY "${CMAKE_CURRENT_LIST_DIR}/traccc-config.cmake"
            "${CMAKE_CURRENT_LIST_DIR}/traccc-config-version.cmake"
            "${CMAKE_CURRENT_LIST_DIR}/detray-config.cmake"
            "${CMAKE_CURRENT_LIST_DIR}/detray-config-version.cmake"
       DESTINATION "${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}" )

  # We need to set the library output directories to match the
  # expected location of the Athena build. Since the Athena CMake code
  # has not derived this yet at this point, we need to resort to the
  # `Project_PLATFORM` environment variable which is set by asetup /
  # the nightly's `setup.sh`.
  set(ACTS_BUILD_PLATFORM "$ENV{${ATLAS_PROJECT}_PLATFORM}" CACHE STRING "Platform name used by the Acts build")

  set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "")
  set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "")
  set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/${ACTS_BUILD_PLATFORM}/${CMAKE_INSTALL_LIBDIR}" CACHE PATH "" )
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/${ACTS_BUILD_PLATFORM}/${CMAKE_INSTALL_BINDIR}" CACHE PATH ""
  )

  # This causes `FetchContent` to use the specified source directory
  # instead of obtaining the source code remotely
  set(FETCHCONTENT_SOURCE_DIR_ACTS "${ATLAS_ACTS_SOURCE_DIR}" CACHE PATH "")

  # This instructs CMake to intercept any calls to `find_package(Acts)`.
  # The first call will trigger the `FetchContent` initialization and make
  # all ACTS targets available for building.
  FetchContent_Declare(Acts
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
endif()
