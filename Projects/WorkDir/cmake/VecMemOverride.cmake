# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Project file for building a custom version of VecMem against an
# installed ATLAS release/nightly.
#

# Local source directory to take VecMem from.
set(ATLAS_VECMEM_SOURCE_DIR "" CACHE PATH
   "Optional local VecMem source directory")

if(IS_DIRECTORY "${ATLAS_VECMEM_SOURCE_DIR}")

   # Tell the user what's happening.
   message(STATUS "Using VecMem from: ${ATLAS_VECMEM_SOURCE_DIR}")

   # Use FetchContent for the build.
   include(FetchContent)

   # We need to set the library output directories to match the
   # expected location of the Athena build. Since the Athena CMake code
   # has not derived this yet at this point, we need to resort to the
   # `Project_PLATFORM` environment variable which is set by asetup /
   # the nightly's `setup.sh`.
   set(VECMEM_BUILD_PLATFORM "$ENV{${ATLAS_PROJECT}_PLATFORM}" CACHE STRING
      "Platform name used by the VecMem build")

   # Make sure that VecMem build products would end up in the correct
   # directories.
   set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "")
   set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "")
   set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "")
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/${VECMEM_BUILD_PLATFORM}/${CMAKE_INSTALL_LIBDIR}"
      CACHE PATH "")
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${PROJECT_BINARY_DIR}/${VECMEM_BUILD_PLATFORM}/${CMAKE_INSTALL_BINDIR}"
      CACHE PATH "")

  # This causes `FetchContent` to use the specified source directory
  # instead of obtaining the source code remotely
  set(FETCHCONTENT_SOURCE_DIR_VECMEM "${ATLAS_VECMEM_SOURCE_DIR}" CACHE PATH "")

  # Configuration flag(s) for the VecMem build.
  set(VECMEM_BUILD_TESTING OFF CACHE BOOL "Disable VecMem tests")

  # This instructs CMake to intercept any calls to `find_package(vecmem)`.
  # The first call will trigger the `FetchContent` initialization and make
  # all VecMem targets available for building.
  FetchContent_Declare(vecmem SYSTEM OVERRIDE_FIND_PACKAGE)

endif()
