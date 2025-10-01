# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PathResolver )

# External dependencies:
find_package( ROOT COMPONENTS RIO Core )

# Component(s) in the package:
atlas_add_library( PathResolver
   Root/*.cxx
   PUBLIC_HEADERS PathResolver
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES AsgTools
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_dictionary( PathResolverDict
   PathResolver/PathResolverDict.h PathResolver/selection.xml
   LINK_LIBRARIES PathResolver )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Set up a helper environment variable for the test(s):
set( _dpath "${CMAKE_CURRENT_SOURCE_DIR}/test" )
set( _dpath "${CMAKE_CURRENT_SOURCE_DIR}/test/B:${_dpath}" )
set( _dpath "${CMAKE_CURRENT_SOURCE_DIR}/test/A:${_dpath}" )

# Test(s) in the package:
atlas_add_executable( test-path-resolver
  test/PathResolver_test.cxx
  LINK_LIBRARIES CxxUtils PathResolver )
atlas_add_test( PathResolver_test
   SCRIPT test-path-resolver
   ENVIRONMENT "DATAPATH=${_dpath}:$ENV{DATAPATH}"
   LOG_IGNORE_PATTERN "MessageSvc not found" )
atlas_add_test( PathResolver_test_download
   SCRIPT "test-path-resolver -d"
   ENVIRONMENT "DATAPATH=${_dpath}:$ENV{DATAPATH}"
   POST_EXEC_SCRIPT nopost.sh )

# Skip the runtime environment setup in release rebuilding mode:
if( ATLAS_RELEASE_MODE )
   return()
endif()

# Set up the general runtime environment:
set( PathResolverEnvironment_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   CACHE PATH "Location of PathResolverEnvironment.cmake" )
find_package( PathResolverEnvironment )
