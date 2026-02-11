# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Project file for building a selected set of packages against an
# installed ATLAS release/nightly.

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
