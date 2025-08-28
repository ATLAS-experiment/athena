# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Set up the project.
cmake_minimum_required( VERSION 3.25 )
file( READ ${CMAKE_SOURCE_DIR}/version.txt _version )
string( STRIP ${_version} _version )
project( Athena VERSION ${_version} LANGUAGES C CXX Fortran )
unset( _version )

# Turn on CUDA as an available compiler, if it's available.
include( CheckLanguage )
check_language( CUDA )
if( CMAKE_CUDA_COMPILER )
   enable_language( CUDA )
endif()
# Use C++20 with CUDA.
set( CMAKE_CUDA_STANDARD 20 CACHE STRING "CUDA C++ standard to use" )
# Force the compilation of CUDA code for only compute capability 5.2 for now.
# Mainly to get rid of warnings from 11.0 about not supporting older compute
# capabilities anymore.
set( CMAKE_CUDA_ARCHITECTURES "all-major" CACHE STRING
   "CUDA architectures to build code for" )

set( CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Wno-deprecated-gpu-targets" )

# Turn on HIP as a compiler, if it's available.
check_language( HIP )
if( CMAKE_HIP_COMPILER )
   enable_language( HIP )
endif()
# Use C++20 with HIP.
set( CMAKE_HIP_STANDARD 20 CACHE STRING "HIP C++ standard to use" )


# Set the versions of the TDAQ externals to pick up for the build:
if( NOT LCG_NIGHTLY )
   if( NOT "$ENV{LCG_NIGHTLY}" STREQUAL "" )
      set( LCG_NIGHTLY $ENV{LCG_NIGHTLY} CACHE STRING
         "LCG nightly build flavor" )
      message( STATUS "Using LCG_NIGHTLY: ${LCG_NIGHTLY}" )
   endif()
endif()

# Set the versions of the TDAQ externals to pick up for the build:
if( LCG_NIGHTLY )
    # TDAQ_RELEASE_BASE should be set to a NIGHTLY TDAQ build!
    set( TDAQ-COMMON_VERSION "99-00-00" CACHE STRING
       "The version of tdaq-common to use for the build" )
    set( TDAQ_VERSION "99-00-00" CACHE STRING
       "The version of tdaq to use for the build" )
else()
    set( TDAQ-COMMON_VERSION "12-02-00" CACHE STRING
       "The version of tdaq-common to use for the build" )
    set( TDAQ_VERSION "12-02-00" CACHE STRING
       "The version of tdaq to use for the build" )
endif()

set( TDAQ-COMMON_ATROOT
   "$ENV{TDAQ_RELEASE_BASE}/tdaq-common/tdaq-common-${TDAQ-COMMON_VERSION}"
   CACHE PATH "The directory to pick up tdaq-common from" )
set( TDAQ_PROJECT_NAME "tdaq" CACHE STRING "The name of the tdaq project" )
set( TDAQ_ATROOT
   "$ENV{TDAQ_RELEASE_BASE}/${TDAQ_PROJECT_NAME}/${TDAQ_PROJECT_NAME}-${TDAQ_VERSION}"
   CACHE PATH "The directory to pick up tdaq from" )
mark_as_advanced( TDAQ-COMMON_ATROOT TDAQ_PROJECT_NAME
   TDAQ_ATROOT )

# HepMC3
set( HEPMC3_USE TRUE CACHE BOOL "Build with HepMC3" )

# Configure flake8:
set( ATLAS_FLAKE8 flake8_atlas --isolated --select ATL,B,F,E101,E7,E9,W6
                               --ignore B006,B007,B019,E701,E702,E704,E741
                               --enable-extensions ATL902
   CACHE STRING "Default flake8 command" )
set( ATLAS_PYTHON_CHECKER ${ATLAS_FLAKE8}
   CACHE STRING "Python checker command to run during Python module compilation" )

# Apply (compiler) settings from CxxUtils:
set( CxxUtilsSettings_DIR
   "${CMAKE_SOURCE_DIR}/../../Control/CxxUtils/cmake" )
find_package( CxxUtilsSettings )

# Enable CppCheck:
option( ATLAS_USE_CPPCHECK "Use CppCheck in the build" ON )

# Decide whether to enable LTO for AtlasGeant4 and its OBJECT libs
include( CheckIPOSupported )
check_ipo_supported( RESULT ATLAS_IPO_CHECK_RESULT )
if( NOT ATLAS_IPO_CHECK_RESULT )
   message( STATUS "Link-time-optimization is not supported in this build" )
endif()
option( ATLAS_GEANT4_USE_LTO "Use link-time-optimization in building AtlasGeant4"
   ${ATLAS_IPO_CHECK_RESULT} )

# Find the ATLAS CMake code:
find_package( AtlasCMake QUIET )

# Find the base project(s):
find_package( AthenaExternals REQUIRED )
find_package( Gaudi )
find_package( tdaq-common )
find_package( tdaq )

# Set up CppCheck.
if( ATLAS_USE_CPPCHECK )
   find_package( Cppcheck 2.17 )

   # Find ATLAS-specific wrapper:
   find_program( CPPCHECK_ATLAS cppcheck_atlas
      PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../../Control/CxxUtils/scripts" )
   mark_as_advanced( CPPCHECK_ATLAS )

   if( CPPCHECK_FOUND AND CPPCHECK_ATLAS )
      # Prepend cppcheck command to already defined command line options:
      list( PREPEND CMAKE_CPPCHECK_DEFAULT ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/atlas_build_run.sh
         "${CMAKE_CURRENT_SOURCE_DIR}/../../Control/CxxUtils/scripts/cppcheck_atlas" )

      # Ignore externals:
      foreach( ignored_path "${LCG_RELEASE_BASE}"
            "${TDAQ-COMMON_INCLUDE_DIR}" "${TDAQ_INCLUDE_DIR}"
            "${${CMAKE_PROJECT_NAME}Externals_INCLUDE_DIR}" )
         if( ignored_path )
            list( APPEND CMAKE_CPPCHECK_DEFAULT "--suppress=*:${ignored_path}/*" )
         endif()
      endforeach()

      # Set the cache variable(s) that CMake uses to decide how to use cppcheck:
      set( CMAKE_C_CPPCHECK "${CMAKE_CPPCHECK_DEFAULT}"
         CACHE STRING "CppCheck command to use for the C source files" )
      set( CMAKE_CXX_CPPCHECK "${CMAKE_CPPCHECK_DEFAULT}"
         CACHE STRING "CppCheck command to use for the C++ source files" )
      # Let the user know what happened:
      if( CMAKE_C_CPPCHECK OR CMAKE_CXX_CPPCHECK )
         list( JOIN CMAKE_CPPCHECK_DEFAULT " " _cmd )
         message( STATUS "Checking C/C++ files with: ${_cmd}" )
      endif()
   else()
      message( WARNING "Cppcheck is enabled via ATLAS_USE_CPPCHECK but cannot be found.")
   endif()
endif()

# Find some auxiliary packages:
find_package( Frontier_Client )
find_package( Doxygen )
find_package( PNG )
find_package( VDT )
find_package( TIFF )
find_package( auth_get_sso_cookie )
find_package( heaptrack )
find_package( libffi )
find_package( gl2ps )
find_package( autopep8 )

# Set up the correct runtime environment for OpenBLAS.
set( OpenBlasEnvironment_DIR "${CMAKE_SOURCE_DIR}/cmake"
   CACHE PATH "Directory holding OpenBlasEnvironmentConfig.cmake" )
mark_as_advanced( OpenBlasEnvironment_DIR )
find_package( OpenBlasEnvironment )

# Set up where to find the AthenaPoolUtilitiesTest CMake code.
set( AthenaPoolUtilitiesTest_DIR
   "${CMAKE_SOURCE_DIR}/../../Database/AthenaPOOL/AthenaPoolUtilities/cmake"
   CACHE PATH "Directory holding the AthenaPoolUtilititesTest module" )

# Set up where to find the xAODUtilities CMake code.
set( xAODUtilities_DIR
   "${CMAKE_SOURCE_DIR}/../../Event/xAOD/xAODCore/cmake"
   CACHE PATH "Directory holding the xAODUtilities module" )

# Make the local CMake files visible to AtlasCMake.
list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake )

# Set up CTest:
atlas_ctest_setup()

# Set up the "ATLAS project".
atlas_project( USE AthenaExternals ${AthenaExternals_VERSION}
   PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../../ )

# Generate the environment setup for the externals, to be used during the build:
lcg_generate_env( SH_FILE ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh )

# Generate replacement rules for the installed paths:
set( _replacements )
if( NOT "$ENV{ATLAS_BUILD_DIR}" STREQUAL "" )
   list( APPEND _replacements "$ENV{ATLAS_BUILD_DIR}/install"
                              "\${Athena_DIR}/../../../.." )
endif()

# Now generate and install the installed setup files:
lcg_generate_env(
   SH_FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/env_setup_install.sh
   REPLACE ${_replacements} )
install( FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/env_setup_install.sh
   DESTINATION . RENAME env_setup.sh )

# Configure and install the pre- and post-configuration files:
string( REPLACE "$ENV{TDAQ_RELEASE_BASE}" "\$ENV{TDAQ_RELEASE_BASE}"
   TDAQ-COMMON_ATROOT "${TDAQ-COMMON_ATROOT}" )
string( REPLACE "${TDAQ-COMMON_VERSION}" "\${TDAQ-COMMON_VERSION}"
   TDAQ-COMMON_ATROOT "${TDAQ-COMMON_ATROOT}" )

# Temporarily add tdaq dependency to Athena build:
string( REPLACE "$ENV{TDAQ_RELEASE_BASE}" "\$ENV{TDAQ_RELEASE_BASE}"
   TDAQ_ATROOT "${TDAQ_ATROOT}" )
string( REPLACE "${TDAQ_VERSION}" "\${TDAQ_VERSION}"
   TDAQ_ATROOT "${TDAQ_ATROOT}" )

configure_file( ${CMAKE_SOURCE_DIR}/cmake/PreConfig.cmake.in
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PreConfig.cmake @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/cmake/PostConfig.cmake.in
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PostConfig.cmake @ONLY )
install( FILES
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PreConfig.cmake
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PostConfig.cmake
   DESTINATION ${CMAKE_INSTALL_CMAKEDIR} )

# Install graphviz output if available:
install( FILES ${CMAKE_BINARY_DIR}/packages.dot
   DESTINATION . OPTIONAL )

# Setup IDE integration:
set( ATLAS_ENABLE_IDE_HELPERS OFF CACHE BOOL "Enable IDE helpers" )
if( ATLAS_ENABLE_IDE_HELPERS )
   set( ATLAS_IDEHELPERSCRIPTS_SETUP
      "${CMAKE_SOURCE_DIR}/../../.vscode/IDEHelperScripts/Setup.cmake"
      CACHE FILEPATH "Setup file for the IDE / VS Code helpers" )
   include( "${ATLAS_IDEHELPERSCRIPTS_SETUP}" )
endif()

# Package up the release using CPack:
atlas_cpack_setup()
