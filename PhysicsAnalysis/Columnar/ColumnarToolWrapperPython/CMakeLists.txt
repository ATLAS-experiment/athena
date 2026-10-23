# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack
# @author Matthew Feickert
# @author Giordon Stark
#
# Notes:
#   This package (ColumnarToolWrapperPython) is only built if
#   ColumnarModeArray is one of the columnar modes of the project, and
#   it is then built for that mode only.

atlas_subdir( ColumnarToolWrapperPython )

# FIX ME: These settings should come from the Project configuration
if (COLUMNAR_ANALYSIS)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeArray;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
elseif (XAOD_STANDALONE)
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD;ColumnarModeArray")
  SET (COLUMNAR_MODE_SUFFIX_LIST ";_array")
else()
  SET (COLUMNAR_MODE_NAME_LIST "ColumnarModeXAOD")
  SET (COLUMNAR_MODE_SUFFIX_LIST "")
endif()

# For now this package only supports ColumnarModeArray, so I look up
# the suffix for that mode and skip the package if it isn't available.
# If at some point we need to support other modes as well (e.g. a
# future RDF mode), this could either become a loop over the modes or
# be split into separate packages per mode. That is left for later.
list(FIND COLUMNAR_MODE_NAME_LIST ColumnarModeArray idx)
if(idx LESS 0)
  message ("Skipping ColumnarToolWrapperPython, ColumnarModeArray is not available")
  return()
endif()
# COLUMNAR_MODE_SUFFIX_LIST may be a single-element list consisting of
# an empty string. This protects against that.
list(LENGTH COLUMNAR_MODE_SUFFIX_LIST suffix_len)
if(idx LESS suffix_len)
  list(GET COLUMNAR_MODE_SUFFIX_LIST ${idx} CMODE_SUFFIX)
else()
  set(CMODE_SUFFIX "")
endif()

# Add the shared library. This doesn't depend on the columnar mode
# itself, so don't add ColumnarModeLib here.
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
    LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperPythonLib AsgTestingLib ColumnarExampleToolsLib${CMODE_SUFFIX}
    POST_EXEC_SCRIPT noerror.sh )

  #atlas_add_test( ColumnarToolWrapperPython_import
  #  SCRIPT ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_import.py
  #  POST_EXEC_SCRIPT noerror.sh )

  #atlas_add_test( ColumnarToolWrapperPython_atlascp
  #  SCRIPT ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_atlascp.py
  #  POST_EXEC_SCRIPT noerror.sh )

  #atlas_add_test( ColumnarToolWrapperPython_example_tools
  #  SCRIPT ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_example_tools.py
  #  POST_EXEC_SCRIPT noerror.sh )

  #atlas_add_test( ColumnarToolWrapperPython_wrapper
  #  SCRIPT ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_wrapper.py
  #  POST_EXEC_SCRIPT noerror.sh )

  #atlas_add_test( ColumnarToolWrapperPython_bindings
  #  SCRIPT ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_bindings.py
  #  POST_EXEC_SCRIPT noerror.sh )
endif ()

# nanobind needs Development.Module. Development.Embed is additionally
# needed when using NB_SHARED (see below).
find_package(Python 3.8 COMPONENTS Interpreter Development.Module Development.Embed REQUIRED)

# Detect the installed nanobind package and import it into CMake
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)
find_package(nanobind CONFIG REQUIRED)

# nanobind link type: NB_STATIC (default) or NB_SHARED.
# Variable to allows switching between using shared or static libnanobind
# for users in an ATLAS Analysis Release environment with ATLAS CMake
# c.f. https://github.com/wjakob/nanobind/blob/e13448781a608efd3b1cb97a92e756a426400c60/docs/api_cmake.rst#high-level-interface
if(NOT DEFINED NANOBIND_LIBRARY_LINK_TYPE)
  set(
    NANOBIND_LIBRARY_LINK_TYPE NB_STATIC
    CACHE STRING
    "nanobind link type: NB_STATIC (default) or NB_SHARED."
  )
endif()

# nanobind_add_module creates a CMake MODULE target that produces a .so
# in CMake's build tree. ATLAS CMake expects Python modules in its own
# build-area Python directory, so we must bridge the two (see below).
nanobind_add_module(
  python_tool_handle
  STABLE_ABI
  LTO
  NB_SUPPRESS_WARNINGS
  "${NANOBIND_LIBRARY_LINK_TYPE}"
  src/NanobindBindings.cxx
)

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
#
# Copy the built .so into the ATLAS build-area Python directory
# so it's importable during build-time testing.
add_custom_command(TARGET python_tool_handle POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_PYTHON_OUTPUT_DIRECTORY}/${ATLAS_PACKAGE}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:python_tool_handle>
    "${CMAKE_PYTHON_OUTPUT_DIRECTORY}/${ATLAS_PACKAGE}/"
)

# Install the .so into the install-area Python directory.
install(FILES $<TARGET_FILE:python_tool_handle>
   DESTINATION "${CMAKE_INSTALL_PYTHONDIR}/${ATLAS_PACKAGE}/"
   OPTIONAL)

# This copies all .py and .so files from the package's python/ directory
# into the ATLAS build and install Python areas and runs the Python
# bytecode compilation step.
atlas_install_python_modules( python/*.py )

# Link headers and libraries.
target_include_directories(
  python_tool_handle
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/ColumnarToolWrapperPython"
)
target_link_libraries(python_tool_handle PRIVATE ColumnarToolWrapperPythonLib ColumnarCoreLib ColumnarModeLib${CMODE_SUFFIX} CxxUtils Python::Python)

# ATLAS CMake needs to be told about the Python libraries at link time.
# This is NOT required for normal CMake.

# Required if and only if using NB_SHARED
if (NANOBIND_LIBRARY_LINK_TYPE STREQUAL "NB_SHARED")
  # Imported target Python::Python defined if component Development.Embed is found
  target_link_libraries(nanobind PRIVATE Python::Python)
endif()

target_link_libraries(python_tool_handle PRIVATE Python::Python)

# Ensure nanobind module builds before the package success message.
# ATLAS CMake needs to be told about extra targets to make the success
# message for the package build appear last.
add_dependencies( Package_ColumnarToolWrapperPython python_tool_handle )
