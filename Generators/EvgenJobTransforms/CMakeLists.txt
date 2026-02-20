# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( EvgenJobTransforms )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
atlas_install_runtime( share/file/*.txt )

# test sample definitions
atlas_install_joboptions( share/EvgenTest POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Set up the runtime environment for EvgenJobTransforms.
configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/HepMCEnvironmentConfig.cmake.in
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HepMCEnvironmentConfig.cmake
   @ONLY )
set( HepMCEnvironment_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}
   CACHE PATH "Location of HepMCEnvironmentConfig.cmake" )

find_package( HepMCEnvironment )
