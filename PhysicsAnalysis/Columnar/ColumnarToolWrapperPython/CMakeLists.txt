# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack
# @author Matthew Feickert
# @author Giordon Stark
#
# Notes:
#   This package (ColumnarToolWrapperPython) should compile in any project, but
#   the runtime code depends on being in a columnar-based project (e.g.
#   ColumnarAnalysis).

atlas_subdir( ColumnarToolWrapperPython )

# Add the shared library:
atlas_add_library (ColumnarToolWrapperPythonLib
ColumnarToolWrapperPython/*.h
  INTERFACE
  PUBLIC_HEADERS ColumnarToolWrapperPython
  LINK_LIBRARIES ColumnarToolWrapperLib PATInterfaces)

find_package( GTest )

if (XAOD_STANDALONE)
  atlas_add_test( gt_PythonToolHandle
    SOURCES test/gt_PythonToolHandle.cxx
    INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperPythonLib AsgTestingLib ColumnarExampleToolsLib
    POST_EXEC_SCRIPT nopost.sh )
endif ()

# Though nanobind only needs Development.Module it seems ATLAS CMake requires
# the full Development (Development.Module and Development.Embed) to be used.
find_package(Python 3.8 COMPONENTS Interpreter Development.Module Development.Embed REQUIRED)

# Detect the installed nanobind package and import it into CMake
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
find_package(nanobind CONFIG REQUIRED)

# Variable to allows switching between using shared or static libnanobind
# for users in an ATLAS Analysis Release environment with ATLAS CMake
# c.f. https://github.com/wjakob/nanobind/blob/e13448781a608efd3b1cb97a92e756a426400c60/docs/api_cmake.rst#high-level-interface
if(NOT DEFINED NANOBIND_LIBRARY_LINK_TYPE)
  set(
    NANOBIND_LIBRARY_LINK_TYPE NB_SHARED
    CACHE STRING
    "The nanobind high level interface parameter NB_STATIC or NB_SHARED. Default is NB_SHARED."
  )
endif()

nanobind_add_module(
  python_tool_handle
  STABLE_ABI
  LTO
  NB_SUPPRESS_WARNINGS
  "${NANOBIND_LIBRARY_LINK_TYPE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/NanobindBindings.cxx"
)

# Link external headers and libraries
target_include_directories(
  python_tool_handle
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/ColumnarToolWrapperPython"
)
target_link_libraries(python_tool_handle PRIVATE ColumnarToolWrapperPythonLib ColumnarCoreLib Python::Python)

# ATLAS CMake needs to be told about the Python libraries at link time.
# This is NOT required for normal CMake.

# Required if and only if using NB_SHARED
if (NANOBIND_LIBRARY_LINK_TYPE STREQUAL "NB_SHARED")
  # Imported target Python::Python defined if component Development.Embed is found
  target_link_libraries(nanobind PRIVATE Python::Python)
endif()

target_link_libraries(python_tool_handle PRIVATE Python::Python)

# ATLAS CMake needs to be told about extra targets to make the success
# message for the package build appear last.
add_dependencies( Package_ColumnarToolWrapperPython python_tool_handle )
