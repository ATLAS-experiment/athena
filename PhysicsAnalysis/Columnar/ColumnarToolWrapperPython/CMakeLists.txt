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
    NANOBIND_LIBRARY_LINK_TYPE NB_STATIC
    CACHE STRING
    "The nanobind high level interface parameter NB_STATIC or NB_SHARED. Default is NB_STATIC."
  )
endif()

# nanobind_add_module creates a CMake MODULE target and produces a file
# named:
#
#   python_tool_handle.cpython-<abi>-<arch>.so
#
# This filename is what Python will load when doing:
#
#   import python_tool_handle
#
# However, ATLAS does not install Python extensions from CMake targets.
# ATLAS only installs files that exist under the package's python/
# directory using atlas_install_python_modules().
nanobind_add_module(
  python_tool_handle
  STABLE_ABI
  LTO
  NB_SUPPRESS_WARNINGS
  "${NANOBIND_LIBRARY_LINK_TYPE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/NanobindBindings.cxx"
)

# Bridge nanobind's target-based build with ATLAS's file-based Python
# installation model.
#
# ATLAS installs Python modules by scanning the package's python/
# directory in the *source tree*. It does not look at the CMake build tree
# and it does not know about CMake targets.
#
# Therefore, after the nanobind module is built, we explicitly copy the
# generated .so file into:
#
#   <INSTALL>/python/<this package>/
#
# since ATLAS CMake's install scripts only install from source, but not from
# build.
install(FILES $<TARGET_FILE:python_tool_handle>
   DESTINATION "${CMAKE_INSTALL_PYTHONDIR}/${ATLAS_PACKAGE}/"
   OPTIONAL)

# This copies all .py and .so files from the package's python/ directory
# into the ATLAS build and install Python areas and runs the Python
# bytecode compilation step.
atlas_install_python_modules( python/*.py )

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
